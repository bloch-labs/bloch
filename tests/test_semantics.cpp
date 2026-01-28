// Copyright 2026 Akshay Pal (https://bloch-labs.com)
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

#include "bloch/core/lexer/lexer.hpp"
#include "bloch/core/parser/parser.hpp"
#include "bloch/core/semantics/semantic_analyser.hpp"
#include "bloch/support/error/bloch_error.hpp"
#include "test_framework.hpp"

using namespace bloch::core;
using bloch::support::BlochError;

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

TEST(SemanticTest, NullAllowedForClassReferences) {
    const char* src =
        "class Node { public Node next; public constructor() -> Node { this.next = null; return "
        "this; } }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}

TEST(SemanticTest, NullRejectedForPrimitive) {
    auto program = parseProgram("int x = null;");
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, NullRejectedForArrays) {
    auto program = parseProgram("int[] xs = null;");
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, NullComparisonRequiresClassReference) {
    auto program = parseProgram("function main() -> void { int x = 0; if (x == null) { } }");
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, NullComparisonAllowedForClassReference) {
    const char* src =
        "class Foo { public constructor() -> Foo = default; } function main() -> void { Foo f = "
        "null; if (f == null) { } }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}

TEST(SemanticTest, NullNotEqualsAllowedForClassReference) {
    const char* src =
        "class Foo { public constructor() -> Foo = default; } function main() -> void { Foo f = "
        "new Foo(); if (f != null) { } }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}

TEST(SemanticTest, MethodOverloadingAllowedForDifferentParams) {
    const char* src =
        "class Foo { public constructor() -> Foo = default; public function bar(int a) -> void { } "
        "public function bar(float a) -> void { } }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}

TEST(SemanticTest, DuplicateMethodSignatureRejected) {
    const char* src =
        "class Foo { public constructor() -> Foo = default; public function bar(int a) -> void { } "
        "public function bar(int b) -> void { } }";
    auto program = parseProgram(src);
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

TEST(SemanticTest, ShotsAnnotationAllowedOnMain) {
    const char* src = "@shots(5) function main() -> void { }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}

TEST(SemanticTest, ShotsAnnotationRejectedOffMain) {
    const char* src = "@shots(5) function foo() -> void { }";
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

TEST(SemanticTest, CastToCharIsRejectedWithMessage) {
    const char* src = "function main() -> void { int x = 1; int y = (char) x; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    bool threw = false;
    try {
        analyser.analyse(*program);
    } catch (const BlochError& err) {
        threw = true;
        std::string msg = err.what();
        EXPECT_NE(msg.find("Cannot explicitally cast from int to char"), std::string::npos);
    }
    EXPECT_TRUE(threw);
}

TEST(SemanticTest, CastFromCharIsRejectedWithMessage) {
    const char* src = "function main() -> void { char c = 'a'; int y = (int) c; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    bool threw = false;
    try {
        analyser.analyse(*program);
    } catch (const BlochError& err) {
        threw = true;
        std::string msg = err.what();
        EXPECT_NE(msg.find("Cannot explicitally cast from char to int"), std::string::npos);
    }
    EXPECT_TRUE(threw);
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

TEST(SemanticTest, MeasureQubitArrayPasses) {
    const char* src = "function main() -> void { qubit[2] q; measure q; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
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

TEST(SemanticTest, DuplicateClassMethodFails) {
    const char* src = "class A { public function f() -> void { } public function f() -> void { } }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, PrivateFieldNotAccessibleOutsideClass) {
    const char* src =
        "class A { private int x = 1; public constructor() -> A { return this; } } "
        "function main() -> void { A a = new A(); int y = a.x; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, ProtectedFieldAccessibleInSubclass) {
    const char* src =
        "class Base { protected int x = 1; public constructor() -> Base { return this; } } "
        "class Derived extends Base { public constructor() -> Derived = default; public function "
        "get() -> int { return this.x; } } "
        "function main() -> void { Derived d = new Derived(); int y = d.get(); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}

TEST(SemanticTest, ProtectedFieldNotAccessibleOutsideHierarchy) {
    const char* src =
        "class Base { protected int x = 1; public constructor() -> Base { return this; } } "
        "class Other { public function get() -> int { Base b = new Base(); return b.x; } }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, PrivateFieldNotAccessibleInSubclass) {
    const char* src =
        "class Base { private int x = 1; public constructor() -> Base { return this; } } "
        "class Child extends Base { public function get() -> int { return this.x; } }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, AbstractMethodMustBeImplemented) {
    const char* src =
        "class A { public virtual function foo() -> void; } "
        "class B extends A { } "
        "function main() -> void { B b = new B(); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, AbstractMethodImplementedAllowsInstantiation) {
    const char* src =
        "class A { public constructor() -> A = default; public virtual function foo() -> void; } "
        "class B extends A { public constructor() -> B = default; public override function foo() "
        "-> void { return; } } "
        "function main() -> void { B b = new B(); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}

TEST(SemanticTest, OverrideWithoutBaseFails) {
    const char* src = "class A { public override function foo() -> void { return; } }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, SuperCallMustBeFirstInConstructor) {
    const char* src =
        "class A { public constructor() -> A { return this; } } "
        "class B extends A { public constructor() -> B { int x = 1; super(); return this; } }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, SuperCallFirstStatementAllowed) {
    const char* src =
        "class A { public constructor() -> A { return this; } } "
        "class B extends A { public constructor() -> B { super(); return this; } }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}

TEST(SemanticTest, FinalFieldAssignmentOutsideConstructorFails) {
    const char* src =
        "class A { public final int x = 1; public function set() -> void { this.x = 2; } }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, MemberCallArgumentTypeMismatchFails) {
    const char* src =
        "class A { public function foo(int x) -> void { } public constructor() -> A { return this; "
        "} } "
        "function main() -> void { A a = new A(); a.foo(1.2f); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, InstanceMethodCallOnTypeFails) {
    const char* src =
        "class A { public function foo() -> void { } public constructor() -> A { return this; } } "
        "function main() -> void { A.foo(); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, StaticClassCannotBeInstantiated) {
    const char* src =
        "static class Utils { public static function foo() -> void { return; } } "
        "function main() -> void { Utils u = new Utils(); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, OverrideStaticMethodFails) {
    const char* src =
        "class Base { public static function foo() -> void { return; } } "
        "class Child extends Base { public override function foo() -> void { return; } }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, SuperWithoutBaseCtorFails) {
    const char* src = "class A { public constructor() -> A { super(); return this; } }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, ThisDisallowedInStaticMethod) {
    const char* src =
        "class A { public static function foo() -> void { int x = this.bar; } "
        "private int bar = 1; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, FinalFieldAssignableInConstructor) {
    const char* src =
        "class A { public final int x; public constructor() -> A { this.x = 1; return this; } } "
        "function main() -> void { A a = new A(); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}

TEST(SemanticTest, DefaultConstructorParameterChecks) {
    const char* ok = "class A { int x; public constructor(int x) -> A = default; }";
    auto programOk = parseProgram(ok);
    SemanticAnalyser analyserOk;
    EXPECT_NO_THROW(analyserOk.analyse(*programOk));

    const char* badName = "class A { int a; public constructor(int b) -> A = default; }";
    auto programBadName = parseProgram(badName);
    SemanticAnalyser analyserBadName;
    EXPECT_THROW(analyserBadName.analyse(*programBadName), BlochError);

    const char* badType = "class A { int a; public constructor(float a) -> A = default; }";
    auto programBadType = parseProgram(badType);
    SemanticAnalyser analyserBadType;
    EXPECT_THROW(analyserBadType.analyse(*programBadType), BlochError);

    const char* badQubit = "class A { qubit q; public constructor(qubit q) -> A = default; }";
    auto programBadQubit = parseProgram(badQubit);
    SemanticAnalyser analyserBadQubit;
    EXPECT_THROW(analyserBadQubit.analyse(*programBadQubit), BlochError);
}

TEST(SemanticTest, DestroyNonClassFails) {
    const char* src = "int i = 1; destroy i;";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, CallingFieldAsFunctionFails) {
    const char* src =
        "class A { public int x = 1; public constructor() -> A { return this; } } "
        "function main() -> void { A a = new A(); a.x(); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, MissingConstructorFails) {
    const char* src = "class A { int x; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, PrivateConstructorNotAccessible) {
    const char* src =
        "class A { private constructor() -> A { return this; } } "
        "function main() -> void { A a = new A(); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, QuantumFunctionBitArrayReturnAllowed) {
    const char* src = "@quantum function foo() -> bit[] { bit[2] m = {0b, 1b}; return m; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}

TEST(SemanticTest, QuantumFunctionInvalidReturnFails) {
    const char* src = "@quantum function foo() -> int { return 1; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, GenericTypeArgumentCountMismatchFails) {
    const char* src =
        "class Box<T> { public constructor() -> Box<T> = default; } "
        "function main() -> void { Box<int, int> b; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, GenericBoundViolationFails) {
    const char* src =
        "class Base { public constructor() -> Base = default; } "
        "class Box<T extends Base> { public constructor() -> Box<T> = default; } "
        "function main() -> void { Box<int> b; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
}

TEST(SemanticTest, GenericBoundSatisfiedPasses) {
    const char* src =
        "class Base { public constructor() -> Base = default; } "
        "class Child extends Base { public constructor() -> Child = default; } "
        "class Box<T extends Base> { public T v; public constructor(T v) -> Box<T> { this.v = v; "
        "return this; } } "
        "function main() -> void { Box<Child> b = new Box<Child>(new Child()); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    EXPECT_NO_THROW(analyser.analyse(*program));
}
