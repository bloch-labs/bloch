#include "bloch/codegen/cpp_generator.hpp"
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

TEST(RuntimeTest, CppGenerationOmitsQuantumOps) {
    const char* src =
        "@quantum function flip() -> bit { qubit q; h(q); bit r = measure q; return r; } function "
        "main() -> void { bit b = flip(); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    eval.execute(*program);
    CppGenerator gen(eval.measurements());
    std::string cpp = gen.generate(*program);
    // should not contain quantum constructs
    EXPECT_EQ(cpp.find("h("), std::string::npos);
    EXPECT_EQ(cpp.find("measure"), std::string::npos);
    EXPECT_NE(cpp.find("bool b"), std::string::npos);
}