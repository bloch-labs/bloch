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

#include "bloch/error/bloch_error.hpp"
#include "bloch/lexer/lexer.hpp"
#include "bloch/parser/parser.hpp"
#include "bloch/semantics/semantic_analyser.hpp"
#include "test_framework.hpp"

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
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, ErrorHasLineColumn) {
    auto program = parseProgram("x = 5;");
    SemanticAnalyser analyser;
    bool threw = false;
    try {
        analyser.analyse(*program);
    } catch (const BlochError& err) {
        threw = true;
        EXPECT_GT(err.line, 0);
        EXPECT_GT(err.column, 0);
    }
    EXPECT_TRUE(threw);
}

TEST(SemanticTest, RedeclaredVariableFails) {
    auto program = parseProgram("int x; int x;");
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, InnerVariableNotVisibleOutside) {
    auto program = parseProgram("{ int y; } y = 1;");
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, OuterVariableVisibleInsideBlock) {
    auto program = parseProgram("int x; { x = 2; }");
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}

TEST(SemanticTest, RedeclareInInnerBlockFails) {
    auto program = parseProgram("int x; { int x; }");
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
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
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, QuantumReturnTypeBitAllowed) {
    const char* src = "@quantum function q() -> bit { return 0b; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}

TEST(SemanticTest, QuantumReturnTypeInvalidInt) {
    const char* src = "@quantum function q() -> int { return 0; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, QuantumReturnTypeInvalidString) {
    const char* src = "@quantum function q() -> string { return \"hello\"; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, QuantumReturnTypeInvalidChar) {
    const char* src = "@quantum function q() -> char { return \'c\'; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, VoidFunctionReturnValueFails) {
    const char* src = "function foo() -> void { return 1; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, NonVoidFunctionNeedsValue) {
    const char* src = "function foo() -> int { return; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, NonVoidFunctionMissingReturnFails) {
    const char* src = "function foo() -> int { int x = 1; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, FinalVariableAssignmentFails) {
    const char* src = "final int x = 1; x = 2;";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, FinalVariableDeclarationOk) {
    const char* src = "final int x = 1;";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}

TEST(SemanticTest, AssignFromFunctionCall) {
    const char* src = "function foo() -> bit { return 0b; } bit b = foo();";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}

TEST(SemanticTest, CallBeforeDeclaration) {
    const char* src = "bit b = foo(); function foo() -> bit { return 0b; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}

TEST(SemanticTest, CallUndefinedFunctionFails) {
    const char* src = "bit b = foo();";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, DuplicateFunctionDeclarationFails) {
    const char* src = "function foo() -> void { } function foo() -> void { }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, BuiltinGateCallIsValid) {
    const char* src = "qubit q; h(q);";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}

TEST(SemanticTest, BuiltinGateWrongArgCount) {
    const char* src = "qubit q; h();";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, AssignFromVoidFunctionFails) {
    const char* src = "function foo() -> void { } int x = foo();";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, AssignFromVoidBuiltinFails) {
    const char* src = "qubit q; qubit r = h(q);";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, BuiltinGateWrongArgType) {
    const char* src = "string s = \"hello\"; qubit q; h(s);";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, BuiltinGateLiteralArgTypeMismatchFails) {
    const char* src = "qubit q; rx(q, 1);";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, BuiltinGateLiteralArgTypeMatchPasses) {
    const char* src = "qubit q; rx(q, 1.0f);";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}

TEST(SemanticTest, FunctionArgumentTypeMismatchFails) {
    const char* src = "function foo(int a) -> void { } foo(1.2f);";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, FunctionArgumentVariableTypeMismatchFails) {
    const char* src = "function foo(float a) -> void { } int x; foo(x);";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, FunctionArgumentVariableTypeMatchPasses) {
    const char* src = "function foo(int a) -> void { } int x; foo(x);";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}

TEST(SemanticTest, FunctionArgumentTypeMatchPasses) {
    const char* src = "function foo(int a) -> void { } foo(3);";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}

TEST(SemanticTest, PostfixOperatorThrowsErrorWhenNotOnInt) {
    const char* src = "string s = \"hello\"; s++;";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, QubitArrayCannotBeInitialised) {
    const char* src = "qubit[] qs = { };";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, ArraySizeFromFinalIdentifierPasses) {
    const char* src = "final int nq = 6; @tracked qubit[nq] qs;";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
    auto* var = dynamic_cast<VariableDeclaration*>(program->statements[1].get());
    ASSERT_NE(var, nullptr);
    auto* arr = dynamic_cast<ArrayType*>(var->varType.get());
    ASSERT_NE(arr, nullptr);
    EXPECT_EQ(arr->size, 6);
}

TEST(SemanticTest, ArraySizeFromConstExpressionPasses) {
    const char* src = "final int a = 2; final int b = 3; int[a + b - 1] values;";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
    auto* var = dynamic_cast<VariableDeclaration*>(program->statements[2].get());
    ASSERT_NE(var, nullptr);
    auto* arr = dynamic_cast<ArrayType*>(var->varType.get());
    ASSERT_NE(arr, nullptr);
    EXPECT_EQ(arr->size, 4);
}

TEST(SemanticTest, ArraySizeRequiresFinalIntIdentifier) {
    const char* src = "int nq = 6; qubit[nq] qs;";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, AssignToFinalVariableFails) {
    const char* src = "final int x = 1; x = 2;";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, BitInitializerRequiresBitLiteral) {
    const char* src = "bit b = 0;";  // missing 'b' suffix
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, FloatInitializerRequiresFloatLiteral) {
    const char* src = "float f = 3;";  // missing 'f' suffix
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, ReturnTypeMismatchIntToFloatFails) {
    const char* src = "function f() -> float { return 1; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, ReturnTypeMismatchIntToBitFails) {
    const char* src = "function f() -> bit { return 1; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, ReturnStringConcatenationPasses) {
    const char* src = "function f() -> string { return \"a\" + 1; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}

TEST(SemanticTest, MeasureTargetMustBeQubitFails) {
    const char* src = "int x; measure x;";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, ResetTargetMustBeQubitFails) {
    const char* src = "int x; reset x;";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, MeasureExpressionTargetMustBeQubitFails) {
    const char* src = "int i; bit b = measure i;";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, FunctionArgumentExpressionTypeMismatchFails) {
    const char* src = "function foo(float a) -> void { } foo(1 + 2);";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, BuiltinGateExpressionArgTypeMismatchFails) {
    const char* src = "qubit q; rx(q, 1 + 2);";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, StringConcatInitializerPasses) {
    const char* src = "string s = \"hello\" + 5;";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}

TEST(SemanticTest, VoidParameterDisallowed) {
    const char* src = "function f(void x) -> void { }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}
