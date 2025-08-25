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