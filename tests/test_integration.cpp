#include "test_framework.hpp"
#include <array>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

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
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe)
        return result;
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        result += buffer.data();

    fs::path stem = blochFile.stem();
    fs::remove(blochFile);
    fs::remove(cwd / (stem.string() + ".cpp"));
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
    EXPECT_EQ("1\n1\n", output);
}

TEST(IntegrationTest, RunsClassicalProgram) {
    std::string src = R"(
function main() -> void {
    int a = 2 + 3;
    echo(a);
}
)";
    std::string output = runBloch(src, "classical_test.bloch");
    EXPECT_EQ("5\n5\n", output);
}