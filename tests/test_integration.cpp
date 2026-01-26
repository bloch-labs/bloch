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

#ifdef BLOCH_SKIP_INTEGRATION_TESTS
#include "test_framework.hpp"
TEST(IntegrationTest, SkippedOnWindows) { EXPECT_TRUE(true); }
#else
#include <array>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#if defined(_WIN32)
#include <windows.h>
#endif
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif
#if !defined(_WIN32) && !defined(__APPLE__)
#include <unistd.h>
#endif
#include "test_framework.hpp"

namespace {
    // Determine directory of the currently running test executable
    std::filesystem::path executableDir() {
#if defined(_WIN32)
        char buf[MAX_PATH];
        DWORD len = GetModuleFileNameA(nullptr, buf, MAX_PATH);
        if (len > 0) {
            std::filesystem::path p(buf);
            return p.parent_path();
        }
        return std::filesystem::current_path();
#elif defined(__APPLE__)
        uint32_t size = 0;
        _NSGetExecutablePath(nullptr, &size);
        std::string path;
        path.resize(size + 1);
        if (_NSGetExecutablePath(path.data(), &size) == 0) {
            std::filesystem::path p(path.c_str());
            return p.parent_path();
        }
        return std::filesystem::current_path();
#else
        char buf[4096];
        ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
        if (len > 0) {
            buf[len] = '\0';
            std::filesystem::path p(buf);
            return p.parent_path();
        }
        return std::filesystem::current_path();
#endif
    }

    std::filesystem::path findBlochBinary() {
        namespace fs = std::filesystem;
        fs::path base = executableDir();
#if defined(_WIN32)
        const char* exe = "bloch.exe";
#else
        const char* exe = "bloch";
#endif
        std::vector<fs::path> candidates = {
            base / exe,
            base.parent_path() / exe,
            base / ".." / exe,
            fs::current_path() / "bin" / exe,
            fs::current_path() / "bin" / "Release" / exe,
            fs::current_path().parent_path() / "bin" / exe,
            fs::current_path().parent_path() / "bin" / "Release" / exe,
        };
        for (auto& p : candidates) {
            std::error_code ec;
            if (fs::exists(p, ec))
                return fs::weakly_canonical(p, ec);
        }
        return exe;
    }

    std::string normalizeOutput(std::string result) {
        // Newlines
        {
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
        // ANSI escape sequences: \x1B[ ... letter
        {
            std::string tmp;
            tmp.reserve(result.size());
            for (size_t i = 0; i < result.size();) {
                unsigned char c = static_cast<unsigned char>(result[i]);
                if (c == 0x1B && i + 1 < result.size() && result[i + 1] == '[') {
                    // Skip until a letter (m, K, etc.) or end
                    i += 2;
                    while (i < result.size()) {
                        char ch = result[i++];
                        if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'))
                            break;
                    }
                } else {
                    tmp.push_back(static_cast<char>(c));
                    ++i;
                }
            }
            result.swap(tmp);
        }
        return result;
    }

    std::string runBloch(const std::string& source, const std::string& name,
                         const std::string& options = "") {
        namespace fs = std::filesystem;
        fs::path cwd = fs::current_path();
        fs::path blochFile = cwd / name;
        std::ofstream ofs(blochFile);
        ofs << source;
        ofs.close();

        fs::path blochBin = findBlochBinary();

        std::string cmd = std::string("\"") + blochBin.string() + "\"";
        if (!options.empty())
            cmd += " " + options;
        cmd += std::string(" \"") + fs::absolute(blochFile).string() + "\" 2>&1";
        std::string result;
        // Redirect stdout/stderr to a file and read it back (more portable than popen on Windows)
        fs::path stem = blochFile.stem();
        fs::path outPath = cwd / (stem.string() + ".out");
        std::string redirCmd = cmd + std::string(" > \"") + outPath.string() + "\" 2>&1";
        [[maybe_unused]] int rc = std::system(redirCmd.c_str());
        {
            std::ifstream in(outPath);
            if (in) {
                result.assign((std::istreambuf_iterator<char>(in)),
                              std::istreambuf_iterator<char>());
                in.close();
            }
        }

        result = normalizeOutput(std::move(result));

        fs::remove(blochFile);
        fs::remove(cwd / (stem.string() + ".qasm"));
        fs::remove(cwd / (stem.string() + ".out"));
        return result;
    }

    std::string runBlochCommand(const std::string& options) {
        namespace fs = std::filesystem;
        fs::path cwd = fs::current_path();
        fs::path blochBin = findBlochBinary();
        fs::path outPath = cwd / "bloch_cli.out";
        std::string cmd = std::string("\"") + blochBin.string() + "\"";
        if (!options.empty())
            cmd += " " + options;
        cmd += std::string(" > \"") + outPath.string() + "\" 2>&1";
        [[maybe_unused]] int rc = std::system(cmd.c_str());
        std::string result;
        {
            std::ifstream in(outPath);
            if (in) {
                result.assign((std::istreambuf_iterator<char>(in)),
                              std::istreambuf_iterator<char>());
            }
        }
        fs::remove(outPath);
        return normalizeOutput(std::move(result));
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

TEST(IntegrationTest, HelpListsAllCliOptions) {
    std::string output = runBlochCommand("--help");
    EXPECT_NE(output.find("--help"), std::string::npos);
    EXPECT_NE(output.find("--version"), std::string::npos);
    EXPECT_NE(output.find("--emit-qasm"), std::string::npos);
    EXPECT_NE(output.find("--shots"), std::string::npos);
    EXPECT_NE(output.find("--echo=auto|all|none"), std::string::npos);
    EXPECT_NE(output.find("--update"), std::string::npos);
}

#endif  // BLOCH_SKIP_INTEGRATION_TESTS

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

TEST(IntegrationTest, ShotsAnnotationSetsCount) {
    std::string src = R"(
@shots(3)
function main() -> void {
    @tracked qubit q;
    x(q);
}
)";
    std::string output = runBloch(src, "shots_annotation.bloch");
    EXPECT_NE(output.find("Shots: 3"), std::string::npos);
    EXPECT_EQ(output.find("The '--shots=N' flag will be deprecated"), std::string::npos);
}

TEST(IntegrationTest, ShotsFlagStillWorksAndWarns) {
    std::string src = R"(
function main() -> void {
    @tracked qubit q;
    x(q);
}
)";
    std::string output = runBloch(src, "shots_flag.bloch", "--shots=4");
    EXPECT_NE(output.find("Shots: 4"), std::string::npos);
    EXPECT_NE(output.find("The '--shots=N' flag will be deprecated"), std::string::npos);
}

TEST(IntegrationTest, ShotsAnnotationOverridesFlagAndWarns) {
    std::string src = R"(
@shots(2)
function main() -> void {
    @tracked qubit q;
    x(q);
}
)";
    std::string output = runBloch(src, "shots_annotation_override.bloch", "--shots=5");
    EXPECT_NE(output.find("Shots: 2"), std::string::npos);
    EXPECT_NE(output.find("differs from your @shots(N) annotation"), std::string::npos);
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
