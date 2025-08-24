#include <array>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include "test_framework.hpp"

namespace {
    std::string runBloch(const std::string& source, const std::string& name) {
        namespace fs = std::filesystem;
        fs::path cwd = fs::current_path();
        fs::path blochFile = cwd / name;
        std::ofstream ofs(blochFile);
        ofs << source;
        ofs.close();

        fs::path blochBin = cwd.parent_path() / "bin" / "bloch";
        std::string cmd = blochBin.string() + " " + name;
        std::array<char, 128> buffer;
        std::string result;
        struct PCloseDeleter {
            void operator()(FILE* f) const {
                if (f)
                    pclose(f);
            }
        };
        std::unique_ptr<FILE, PCloseDeleter> pipe(popen(cmd.c_str(), "r"));
        if (!pipe) return result;
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) result += buffer.data();

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
    bit b = 1;
    echo("Measured: " + b);
    echo(5 + 5);
}
)";
    std::string output = runBloch(src, "echo_concat_test.bloch");
    EXPECT_EQ("Measured: 1\n10\n", output);
}