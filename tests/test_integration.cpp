#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
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

        std::string cmd;
#ifdef BLOCH_BIN_PATH
        cmd = std::string(BLOCH_BIN_PATH);
#else
        fs::path blochBin = cwd.parent_path() / "bin" / "bloch";
        cmd = blochBin.string();
#endif
        if (!options.empty())
            cmd += " " + options;
        // Pass absolute path to avoid CTest working directory issues
        cmd += " " + blochFile.string() + " 2>&1";
        // Capture output via a temporary file to avoid pipe hangs
        fs::path stem = blochFile.stem();
        fs::path outFile = cwd / (stem.string() + ".out");
        std::string fullCmd = cmd + " > \"" + outFile.string() + "\" 2>&1";
        (void)std::system(fullCmd.c_str());
        std::ifstream ifs(outFile);
        std::string result((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

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
    @tracked int x = 1;
    echo(x);
}
)";
    std::string output = runBloch(src, "tracked_single.bloch", "--shots=1");
    EXPECT_NE(output.find("1\nShots: 1"), std::string::npos);
    EXPECT_NE(output.find("value | count | prob"), std::string::npos);
    std::string compact = output;
    compact.erase(std::remove(compact.begin(), compact.end(), ' '), compact.end());
    EXPECT_NE(compact.find("1|1|1.000"), std::string::npos);
}

TEST(IntegrationTest, TrackedMultiShotAggregates) {
    std::string src = R"(
function main() -> void {
    @tracked int x = 1;
    echo(x);
}
)";
    std::string output = runBloch(src, "tracked_multi.bloch", "--shots=3");
    EXPECT_NE(output.find("[INFO]: suppressing echo; to view them use --echo=all"),
              std::string::npos);
    EXPECT_NE(output.find("Shots: 3"), std::string::npos);
    EXPECT_NE(output.find("value | count | prob"), std::string::npos);
    std::string compact2 = output;
    compact2.erase(std::remove(compact2.begin(), compact2.end(), ' '), compact2.end());
    EXPECT_NE(compact2.find("1|3|1.000"), std::string::npos);
}

TEST(IntegrationTest, ArrayOperationsAndEcho) {
    std::string src = R"(
function main() -> void { 
    bit[] a = {0b, 1b, 1b, 0b};
    int[] b = {1,2,3};
    echo(b[0]);
    echo(b);
    b[0] = b[0] + 1;
    echo(b);
}
)";
    std::string output = runBloch(src, "array_ops.bloch");
    EXPECT_EQ("1\n{1, 2, 3}\n{2, 2, 3}\n", output);
}
