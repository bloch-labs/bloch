#include <sstream>
#include "bloch/lexer/lexer.hpp"
#include "bloch/parser/parser.hpp"
#include "bloch/runtime/runtime_evaluator.hpp"
#include "bloch/semantics/semantic_analyser.hpp"
#include "test_framework.hpp"

using namespace bloch;

static std::unique_ptr<Program> parseProgram(const char* src) {
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    return parser.parse();
}

TEST(RuntimeTest, GeneratesQasm) {
    const char* src =
        "@quantum function flip() -> bit { qubit q; h(q); bit r = measure q; return r; } function "
        "main() -> void { bit b = flip(); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    eval.execute(*program);
    std::string qasm = eval.getQasm();
    EXPECT_NE(qasm.find("OPENQASM 2.0"), std::string::npos);
    EXPECT_NE(qasm.find("h q[0]"), std::string::npos);
    EXPECT_NE(qasm.find("measure q[0]"), std::string::npos);
}

TEST(RuntimeTest, MultipleQubitDeclarationsAllocateDistinctQubits) {
    const char* src = "function main() -> void { qubit q0, q1; h(q0); h(q1); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    eval.execute(*program);
    std::string qasm = eval.getQasm();
    EXPECT_NE(qasm.find("h q[0]"), std::string::npos);
    EXPECT_NE(qasm.find("h q[1]"), std::string::npos);
}

TEST(RuntimeTest, MeasurementsPreservedInForLoops) {
    const char* src =
        "@quantum function flip() -> bit { qubit q; x(q); bit r = measure q; return r; } "
        "function main() -> void { for (int i = 0; i < 3; i = i + 1) { bit b = flip(); } }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    eval.execute(*program);
    const auto& meas = eval.measurements();
    ASSERT_EQ(meas.size(), 2u);
    for (const auto& kv : meas) {
        ASSERT_EQ(kv.second.size(), 3u);
    }
}

TEST(RuntimeTest, MeasurementsPreservedInWhileLoops) {
    const char* src =
        "@quantum function flip() -> bit { qubit q; x(q); bit r = measure q; return r; } "
        "function main() -> void { int i = 0; while (i < 3) { bit b = flip(); i = i + 1; } }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    eval.execute(*program);
    const auto& meas = eval.measurements();
    ASSERT_EQ(meas.size(), 2u);
    for (const auto& kv : meas) {
        ASSERT_EQ(kv.second.size(), 3u);
    }
}

TEST(RuntimeTest, EchoConcatenatesValues) {
    const char* src =
        "function main() -> void { bit b = 1b; echo(\"Measured: \" + b); echo(5 + 5); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    std::ostringstream output;
    auto* oldBuf = std::cout.rdbuf(output.rdbuf());
    eval.execute(*program);
    std::cout.rdbuf(oldBuf);
    EXPECT_EQ("Measured: 1\n10\n", output.str());
}

TEST(RuntimeTest, TernaryExecutesCorrectBranch) {
    const char* src =
        "function main() -> void { int x = 0; x ? echo(\"true\"); : echo(\"false\"); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    std::ostringstream output;
    auto* oldBuf = std::cout.rdbuf(output.rdbuf());
    eval.execute(*program);
    std::cout.rdbuf(oldBuf);
    EXPECT_EQ("false\n", output.str());
}

TEST(RuntimeTest, PostIncrementAndDecrement) {
    const char* src =
        "function main() -> void { int a = 5; echo(a++); echo(a); echo(a--); echo(a); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    std::ostringstream output;
    auto* oldBuf = std::cout.rdbuf(output.rdbuf());
    eval.execute(*program);
    std::cout.rdbuf(oldBuf);
    EXPECT_EQ("5\n6\n6\n5\n", output.str());
}

TEST(RuntimeTest, TracksVariableSingleShot) {
    const char* src = "function main() -> void { @tracked int x = 1; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    eval.execute(*program);
    const auto& counts = eval.trackedCounts();
    ASSERT_EQ(counts.at("x").at("1"), 1);
}

TEST(RuntimeTest, TracksVariableMultipleShots) {
    const char* src = "function main() -> void { @tracked int x = 1; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    std::unordered_map<std::string, std::unordered_map<std::string, int>> agg;
    for (int i = 0; i < 5; ++i) {
        RuntimeEvaluator eval;
        eval.execute(*program);
        for (const auto& vk : eval.trackedCounts())
            for (const auto& vv : vk.second) agg[vk.first][vv.first] += vv.second;
    }
    ASSERT_EQ(agg["x"]["1"], 5);
}

TEST(RuntimeTest, EchoModes) {
    const char* src = "function main() -> void { echo(1); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    eval.setEcho(false);
    std::ostringstream out;
    auto* old = std::cout.rdbuf(out.rdbuf());
    eval.execute(*program);
    std::cout.rdbuf(old);
    EXPECT_EQ("", out.str());
    RuntimeEvaluator eval2;
    eval2.setEcho(true);
    out.str("");
    old = std::cout.rdbuf(out.rdbuf());
    eval2.execute(*program);
    std::cout.rdbuf(old);
    EXPECT_EQ("1\n", out.str());
}

TEST(RuntimeTest, UninitializedTrackedVariable) {
    const char* src = "function main() -> void { @tracked int x; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    eval.execute(*program);
    const auto& counts = eval.trackedCounts();
    ASSERT_EQ(counts.at("x").at("__unassigned__"), 1);
}

TEST(RuntimeTest, ResetClearsQubit) {
    const char* src =
        "@quantum function main() -> bit { qubit q; x(q); reset q; bit r = measure q; return r; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    eval.execute(*program);
    const auto& meas = eval.measurements();
    ASSERT_EQ(meas.size(), 1u);
    EXPECT_EQ(meas.begin()->second[0], 0);
}

TEST(RuntimeTest, LogicalAndBitwiseOperations) {
    const char* src =
        "function main() -> void { echo(1b & 0b); echo(1b | 0b); echo(1b ^ 1b); echo(~0b); "
        "echo(!1b); echo(1b && 0b); echo(0b || 1b); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    std::ostringstream output;
    auto* oldBuf = std::cout.rdbuf(output.rdbuf());
    eval.execute(*program);
    std::cout.rdbuf(oldBuf);
    EXPECT_EQ("0\n1\n0\n1\n0\n0\n1\n", output.str());
}

TEST(RuntimeTest, BitArrayBitwiseOperations) {
    const char* src =
        "function main() -> void { bit[] a = {0b, 1b, 1b, 0b}; bit[] b = {1b, 0b, 1b, 0b}; "
        "echo(~a); echo(a & b); echo(a | b); echo(a ^ b); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    std::ostringstream output;
    auto* oldBuf = std::cout.rdbuf(output.rdbuf());
    eval.execute(*program);
    std::cout.rdbuf(oldBuf);
    EXPECT_EQ("{1b, 0b, 0b, 1b}\n{0b, 0b, 1b, 0b}\n{1b, 1b, 1b, 0b}\n{1b, 1b, 0b, 0b}\n",
              output.str());
}