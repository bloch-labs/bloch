#include <gtest/gtest.h>
#include "bloch/error/bloch_runtime_error.hpp"
#include "bloch/lexer/lexer.hpp"
#include "bloch/parser/parser.hpp"
#include "bloch/semantics/semantic_analyser.hpp"

using namespace bloch;

static std::unique_ptr<Program> parseProgram(const char* src) {
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    return parser.parse();
}

TEST(SemanticTest, VariableMustBeDeclared) {
    auto program = parseProgram("int x; x = 5;");
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}

TEST(SemanticTest, UseUndeclaredVariableFails) {
    auto program = parseProgram("x = 5;");
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochRuntimeError);
}

TEST(SemanticTest, ErrorHasLineColumn) {
    auto program = parseProgram("x = 5;");
    SemanticAnalyser analyser;
    try {
        analyser.analyse(*program);
        FAIL() << "Expected BlochRuntimeError";
    } catch (const BlochRuntimeError& err) {
        EXPECT_GT(err.line, 0);
        EXPECT_GT(err.column, 0);
    }
}

TEST(SemanticTest, RedeclaredVariableFails) {
    auto program = parseProgram("int x; int x;");
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochRuntimeError);
}

TEST(SemanticTest, InnerVariableNotVisibleOutside) {
    auto program = parseProgram("{ int y; } y = 1;");
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochRuntimeError);
}

TEST(SemanticTest, OuterVariableVisibleInsideBlock) {
    auto program = parseProgram("int x; { x = 2; }");
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}

TEST(SemanticTest, RedeclareInInnerBlockFails) {
    auto program = parseProgram("int x; { int x; }");
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochRuntimeError);
}

TEST(SemanticTest, FunctionScopeUsesParameters) {
    const char* src = "function foo(int a) -> void { a = 1; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}

TEST(SemanticTest, UseUndeclaredInsideFunctionFails) {
    const char* src = "function foo() -> void { x = 1; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochRuntimeError);
}

TEST(SemanticTest, QuantumReturnTypeBitAllowed) {
    const char* src = "@quantum function q() -> bit { return 0; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}

TEST(SemanticTest, QuantumReturnTypeInvalidInt) {
    const char* src = "@quantum function q() -> int { return 0; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochRuntimeError);
}

TEST(SemanticTest, QuantumReturnTypeInvalidString) {
    const char* src = "@quantum function q() -> string { return \"hello\"; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochRuntimeError);
}

TEST(SemanticTest, QuantumReturnTypeInvalidChar) {
    const char* src = "@quantum function q() -> char { return \'c\'; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochRuntimeError);
}

TEST(SemanticTest, VoidFunctionReturnValueFails) {
    const char* src = "function foo() -> void { return 1; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochRuntimeError);
}

TEST(SemanticTest, NonVoidFunctionNeedsValue) {
    const char* src = "function foo() -> int { return; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochRuntimeError);
}

TEST(SemanticTest, FinalVariableAssignmentFails) {
    const char* src = "final int x = 1; x = 2;";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochRuntimeError);
}

TEST(SemanticTest, FinalVariableDeclarationOk) {
    const char* src = "final int x = 1;";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}

TEST(SemanticTest, AssignFromFunctionCall) {
    const char* src = "function foo() -> bit { return 0; } bit b = foo();";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}

TEST(SemanticTest, CallBeforeDeclaration) {
    const char* src = "bit b = foo(); function foo() -> bit { return 0; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}

TEST(SemanticTest, CallUndefinedFunctionFails) {
    const char* src = "bit b = foo();";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochRuntimeError);
}

TEST(SemanticTest, DuplicateFunctionDeclarationFails) {
    const char* src = "function foo() -> void { } function foo() -> void { }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochRuntimeError);
}

TEST(SemanticTest, DuplicateMethodDeclarationFails) {
    const char* src =
        "class Foo { @methods: function bar() -> void { } function bar() -> void { } }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochRuntimeError);
}