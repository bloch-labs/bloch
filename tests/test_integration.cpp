// Copyright 2025 Akshay Pal (https://bloch-labs.com)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <array>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include "test_framework.hpp"

namespace {
    std::string runBloch(const std::string& source, const std::string& name,
                         const std::string& options = "") {
        namespace fs = std::filesystem;
        fs::path cwd = fs::current_path();
        fs::path blochFile = cwd / name;
        std::ofstream ofs(blochFile);
        ofs << source;
        ofs.close();

        auto findBinary = [&](const fs::path& start) -> fs::path {
            std::vector<fs::path> candidates;
#if defined(_WIN32)
            const char* exe = "bloch.exe";
            candidates.push_back(start / exe);
            candidates.push_back(start.parent_path() / "bin" / "Release" / exe);
            candidates.push_back(start.parent_path() / "bin" / exe);
            candidates.push_back(start / "bin" / "Release" / exe);
            candidates.push_back(start / "bin" / exe);
            candidates.push_back(start.parent_path() / "bin" / "Debug" / exe);
#else
            const char* exe = "bloch";
            candidates.push_back(start / exe);
            candidates.push_back(start.parent_path() / "bin" / exe);
            candidates.push_back(start / "bin" / exe);
#endif
            for (const auto& p : candidates) {
                if (fs::exists(p))
                    return fs::absolute(p);
            }
            return fs::absolute(start / exe);
        };

        fs::path blochBin = findBinary(cwd);

        std::string cmd = std::string("\"") + blochBin.string() + "\"";
        if (!options.empty()) cmd += " " + options;
        cmd += std::string(" \"") + fs::absolute(blochFile).string() + "\" 2>&1";
        std::array<char, 128> buffer;
        std::string result;
        struct PCloseDeleter {
            void operator()(FILE* f) const {
                if (f)
                {
#if defined(_WIN32)
                    _pclose(f);
#else
                    pclose(f);
#endif
                }
            }
        };
        std::unique_ptr<FILE, PCloseDeleter> pipe(
#if defined(_WIN32)
            _popen(cmd.c_str(), "r")
#else
            popen(cmd.c_str(), "r")
#endif
        );
        if (!pipe)
            return result;
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) result += buffer.data();

        // Normalize output for cross-platform comparisons:
        // - Convert CRLF/CR to LF
        // - Strip ANSI escape sequences (e.g., color codes)
        {
            // Newlines
            std::string tmp;
            tmp.reserve(result.size());
            for (size_t i = 0; i < result.size(); ++i) {
                char c = result[i];
                if (c == '\r') {
                    // Skip CR; if next is LF, LF will be handled naturally
                    continue;
                }
                tmp.push_back(c);
            }
            result.swap(tmp);
        }
        {
            // ANSI escape sequences: \x1B[ ... letter
            std::string tmp;
            tmp.reserve(result.size());
            for (size_t i = 0; i < result.size();) {
                unsigned char c = static_cast<unsigned char>(result[i]);
                if (c == 0x1B && i + 1 < result.size() && result[i + 1] == '[') {
                    // Skip until a letter (m, K, etc.) or end
                    i += 2;
                    while (i < result.size()) {
                        char ch = result[i++];
                        if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) break;
                    }
                } else {
                    tmp.push_back(static_cast<char>(c));
                    ++i;
                }
            }
            result.swap(tmp);
        }

        fs::path stem = blochFile.stem();
        fs::remove(blochFile);
        fs::remove(cwd / (stem.string() + ".qasm"));
        fs::remove(cwd / (stem.string() + ".out"));
        return result;
    }
}

TEST(IntegrationTest, RunsQuantumProgram) {
    std::string src = R"(
function main() -> void {
    qubit q;
    x(q);
    bit r = measure q;
    echo(r);
}
)";
    std::string output = runBloch(src, "quantum_test.bloch");
    EXPECT_EQ("1\n", output);
}

TEST(IntegrationTest, RunsClassicalProgram) {
    std::string src = R"(
function main() -> void {
    int a = 2 + 3;
    echo(a);
}
)";
    std::string output = runBloch(src, "classical_test.bloch");
    EXPECT_EQ("5\n", output);
}

TEST(IntegrationTest, CountsHeadsInLoop) {
    std::string src = R"(
@quantum
function flip() -> bit {
    qubit q;
    x(q);
    bit r = measure q;
    return r;
}

function main() -> void {
    int heads = 0;
    for (int i = 0; i < 10; i = i + 1) {
        bit b = flip();
        if (b == 1) {
            heads = heads + 1;
        }
    }
    echo(heads);
}
)";
    std::string output = runBloch(src, "coin_flip_test.bloch");
    EXPECT_EQ("10\n", output);
}

TEST(IntegrationTest, EchoesString) {
    std::string src = R"(
function main() -> void {
    string msg = "hello";
    echo(msg);
}
)";
    std::string output = runBloch(src, "string_echo_test.bloch");
    EXPECT_EQ("hello\n", output);
}

TEST(IntegrationTest, EchoConcatenatesValues) {
    std::string src = R"(
function main() -> void {
    bit b = 1b;
    echo("Measured: " + b);
    echo(5 + 5);
}
)";
    std::string output = runBloch(src, "echo_concat_test.bloch");
    EXPECT_EQ("Measured: 1\n10\n", output);
}

TEST(IntegrationTest, TrackedSingleShot) {
    std::string src = R"(
function main() -> void {
    @tracked qubit q;
    x(q);
}
)";
    std::string output = runBloch(src, "tracked_single.bloch", "--shots=1");
    EXPECT_NE(output.find("Shots: 1"), std::string::npos);
    EXPECT_NE(output.find("qubit q"), std::string::npos);
    EXPECT_NE(output.find("?"), std::string::npos);
}

TEST(IntegrationTest, TrackedMultiShotAggregates) {
    std::string src = R"(
function main() -> void {
    @tracked qubit q;
    x(q);
}
)";
    std::string output = runBloch(src, "tracked_multi.bloch", "--shots=3");
    EXPECT_NE(output.find("[INFO]: suppressing echo; to view them use --echo=all"),
              std::string::npos);
    EXPECT_NE(output.find("Shots: 3"), std::string::npos);
    EXPECT_NE(output.find("qubit q"), std::string::npos);
    EXPECT_NE(output.find("?"), std::string::npos);
}

TEST(IntegrationTest, ArrayOperationsAndEcho) {
    std::string src = R"(
function main() -> void { 
    bit[] a = {0b, 1b, 1b, 0b};
    int[] b = {1,2,3};
    echo(a);
    echo(b[0]);
    echo(b);
    b[0] = b[0] + 1;
    echo(b);
}
)";
    std::string output = runBloch(src, "array_ops.bloch");
    EXPECT_EQ("{0, 1, 1, 0}\n1\n{1, 2, 3}\n{2, 2, 3}\n", output);
}
