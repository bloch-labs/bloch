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

#include "bloch/core/lexer/lexer.hpp"
#include "bloch/core/parser/parser.hpp"
#include "bloch/support/error/bloch_error.hpp"
#include "test_framework.hpp"

using namespace bloch::core;
using bloch::support::BlochError;

TEST(ParserTest, ParseVariableDeclaration) {
    Lexer lexer("int x;");
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto program = parser.parse();

    ASSERT_EQ(program->statements.size(), 1u);

    auto* var = dynamic_cast<VariableDeclaration*>(program->statements[0].get());
    ASSERT_NE(var, nullptr);
    EXPECT_EQ(var->name, "x");

    auto* type = dynamic_cast<PrimitiveType*>(var->varType.get());
    ASSERT_NE(type, nullptr);
    EXPECT_EQ(type->name, "int");
}

TEST(ParserTest, ParseInitialisedVariableDeclaration) {
    Lexer lexer("int x = 10;");
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto program = parser.parse();

    ASSERT_EQ(program->statements.size(), 1u);

    auto* var = dynamic_cast<VariableDeclaration*>(program->statements[0].get());
    ASSERT_NE(var, nullptr);
    EXPECT_EQ(var->name, "x");

    auto* type = dynamic_cast<PrimitiveType*>(var->varType.get());
    ASSERT_NE(type, nullptr);
    EXPECT_EQ(type->name, "int");

    auto* lit = dynamic_cast<LiteralExpression*>(var->initializer.get());
    ASSERT_NE(lit, nullptr);
    EXPECT_EQ(lit->value, "10");
    EXPECT_EQ(lit->literalType, "int");
}

TEST(ParserTest, ParseFinalVariableDeclaration) {
    Lexer lexer("final int x;");
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto program = parser.parse();

    ASSERT_EQ(program->statements.size(), 1u);

    auto* var = dynamic_cast<VariableDeclaration*>(program->statements[0].get());
    ASSERT_NE(var, nullptr);
    EXPECT_TRUE(var->isFinal);
    EXPECT_EQ(var->name, "x");

    auto* type = dynamic_cast<PrimitiveType*>(var->varType.get());
    ASSERT_NE(type, nullptr);
    EXPECT_EQ(type->name, "int");
}

TEST(ParserTest, ParseInitialisedFinalVariableDeclaration) {
    Lexer lexer("final int x = 10;");
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto program = parser.parse();

    ASSERT_EQ(program->statements.size(), 1u);

    auto* var = dynamic_cast<VariableDeclaration*>(program->statements[0].get());
    ASSERT_NE(var, nullptr);
    EXPECT_TRUE(var->isFinal);
    EXPECT_EQ(var->name, "x");

    auto* type = dynamic_cast<PrimitiveType*>(var->varType.get());
    ASSERT_NE(type, nullptr);
    EXPECT_EQ(type->name, "int");

    auto* lit = dynamic_cast<LiteralExpression*>(var->initializer.get());
    ASSERT_NE(lit, nullptr);
    EXPECT_EQ(lit->value, "10");
}

TEST(ParserTest, ParseInitialisedFloatVariableDeclaration) {
    Lexer lexer("float f = 3f;");
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto program = parser.parse();

    ASSERT_EQ(program->statements.size(), 1u);

    auto* var = dynamic_cast<VariableDeclaration*>(program->statements[0].get());
    ASSERT_NE(var, nullptr);
    EXPECT_EQ(var->name, "f");

    auto* type = dynamic_cast<PrimitiveType*>(var->varType.get());
    ASSERT_NE(type, nullptr);
    EXPECT_EQ(type->name, "float");

    auto* lit = dynamic_cast<LiteralExpression*>(var->initializer.get());
    ASSERT_NE(lit, nullptr);
    EXPECT_EQ(lit->value, "3f");
    EXPECT_EQ(lit->literalType, "float");
}

TEST(ParserTest, ParseArrayWithIdentifierSize) {
    const char* src = "final int n = 2; qubit[n] qs;";
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto program = parser.parse();

    ASSERT_EQ(program->statements.size(), 2u);
    auto* var = dynamic_cast<VariableDeclaration*>(program->statements[1].get());
    ASSERT_NE(var, nullptr);
    auto* arrType = dynamic_cast<ArrayType*>(var->varType.get());
    ASSERT_NE(arrType, nullptr);
    EXPECT_EQ(arrType->size, -1);
    auto* sizeVar = dynamic_cast<VariableExpression*>(arrType->sizeExpression.get());
    ASSERT_NE(sizeVar, nullptr);
    EXPECT_EQ(sizeVar->name, "n");
}

TEST(ParserTest, StateAnnotationIsRejected) {
    const char* src = "@state(\"+\") qubit q;";
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    EXPECT_THROW((void)parser.parse(), BlochError);
}

TEST(ParserTest, QuantumAnnotationOutsideFunctionRejected) {
    const char* src = "@quantum qubit q;";
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    EXPECT_THROW((void)parser.parse(), BlochError);
}

TEST(ParserTest, ParseMultipleQubitDeclarations) {
    Lexer lexer("qubit q, r;");
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto program = parser.parse();

    ASSERT_EQ(program->statements.size(), 2u);
    auto* q = dynamic_cast<VariableDeclaration*>(program->statements[0].get());
    auto* r = dynamic_cast<VariableDeclaration*>(program->statements[1].get());
    ASSERT_NE(q, nullptr);
    ASSERT_NE(r, nullptr);
    EXPECT_EQ(q->name, "q");
    EXPECT_EQ(r->name, "r");
}

TEST(ParserTest, RejectMultipleNonQubitDeclarations) {
    Lexer lexer("int a, b;");
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    EXPECT_THROW((void)parser.parse(), BlochError);
}

TEST(ParserTest, ParseClassicalFunction) {
    const char* src = "function add(int a, int b) -> int { return a + b; }";
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto program = parser.parse();

    ASSERT_EQ(program->functions.size(), 1u);

    auto* func = program->functions[0].get();
    EXPECT_EQ(func->name, "add");
    ASSERT_EQ(func->params.size(), 2u);
    EXPECT_EQ(func->params[0]->name, "a");
    EXPECT_EQ(func->params[1]->name, "b");

    auto* retType = dynamic_cast<PrimitiveType*>(func->returnType.get());
    ASSERT_NE(retType, nullptr);
    EXPECT_EQ(retType->name, "int");
    ASSERT_NE(func->body, nullptr);
    ASSERT_EQ(func->body->statements.size(), 1u);

    auto* ret = dynamic_cast<ReturnStatement*>(func->body->statements[0].get());
    ASSERT_NE(ret, nullptr);

    auto* bin = dynamic_cast<BinaryExpression*>(ret->value.get());
    ASSERT_NE(bin, nullptr);
    EXPECT_EQ(bin->op, "+");
}

TEST(ParserTest, ParseQuantumFunction) {
    const char* src = "@quantum function q() -> void { }";
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto program = parser.parse();

    ASSERT_EQ(program->functions.size(), 1u);

    auto* func = program->functions[0].get();
    EXPECT_TRUE(func->hasQuantumAnnotation);
    ASSERT_EQ(func->annotations.size(), 1u);
    EXPECT_EQ(func->annotations[0]->name, "quantum");
}

TEST(ParserTest, ExpressionPrecedence) {
    const char* src = "int x = 1 + 2 * 3;";
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto program = parser.parse();

    ASSERT_EQ(program->statements.size(), 1u);
    auto* var = dynamic_cast<VariableDeclaration*>(program->statements[0].get());
    ASSERT_NE(var, nullptr);
    auto* binAdd = dynamic_cast<BinaryExpression*>(var->initializer.get());
    ASSERT_NE(binAdd, nullptr);
    EXPECT_EQ(binAdd->op, "+");
    auto* leftLit = dynamic_cast<LiteralExpression*>(binAdd->left.get());
    ASSERT_NE(leftLit, nullptr);
    EXPECT_EQ(leftLit->value, "1");
    EXPECT_EQ(leftLit->literalType, "int");
    auto* mul = dynamic_cast<BinaryExpression*>(binAdd->right.get());
    ASSERT_NE(mul, nullptr);
    EXPECT_EQ(mul->op, "*");
}

TEST(ParserTest, ParseIfElse) {
    const char* src = "if (1) { return 1; } else { return 0; }";
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto program = parser.parse();

    ASSERT_EQ(program->statements.size(), 1u);
    auto* ifStmt = dynamic_cast<IfStatement*>(program->statements[0].get());
    ASSERT_NE(ifStmt, nullptr);
    ASSERT_NE(ifStmt->thenBranch, nullptr);
    ASSERT_NE(ifStmt->elseBranch, nullptr);

    auto* thenBlock = dynamic_cast<BlockStatement*>(ifStmt->thenBranch.get());
    ASSERT_NE(thenBlock, nullptr);
    ASSERT_EQ(thenBlock->statements.size(), 1u);

    auto* elseBlock = dynamic_cast<BlockStatement*>(ifStmt->elseBranch.get());
    ASSERT_NE(elseBlock, nullptr);
    ASSERT_EQ(elseBlock->statements.size(), 1u);
}

TEST(ParserTest, ParseEqualityOperators) {
    const char* src = "int a = 1 == 2; int b = 3 != 4;";
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto program = parser.parse();

    ASSERT_EQ(program->statements.size(), 2u);

    auto* eqVar = dynamic_cast<VariableDeclaration*>(program->statements[0].get());
    ASSERT_NE(eqVar, nullptr);
    auto* eqBin = dynamic_cast<BinaryExpression*>(eqVar->initializer.get());
    ASSERT_NE(eqBin, nullptr);
    EXPECT_EQ(eqBin->op, "==");

    auto* neVar = dynamic_cast<VariableDeclaration*>(program->statements[1].get());
    ASSERT_NE(neVar, nullptr);
    auto* neBin = dynamic_cast<BinaryExpression*>(neVar->initializer.get());
    ASSERT_NE(neBin, nullptr);
    EXPECT_EQ(neBin->op, "!=");
}

TEST(ParserTest, ParseWhile) {
    const char* src = "while (1) { int x = 0; }";
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto program = parser.parse();

    ASSERT_EQ(program->statements.size(), 1u);
    auto* whileStmt = dynamic_cast<WhileStatement*>(program->statements[0].get());
    ASSERT_NE(whileStmt, nullptr);
    auto* cond = dynamic_cast<LiteralExpression*>(whileStmt->condition.get());
    ASSERT_NE(cond, nullptr);
    EXPECT_EQ(cond->value, "1");
}

TEST(ParserTest, ParseTernaryStatement) {
    const char* src = "1 ? echo(\"a\"); : echo(\"b\");";
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto program = parser.parse();

    ASSERT_EQ(program->statements.size(), 1u);
    auto* tern = dynamic_cast<TernaryStatement*>(program->statements[0].get());
    ASSERT_NE(tern, nullptr);
    auto* cond = dynamic_cast<LiteralExpression*>(tern->condition.get());
    ASSERT_NE(cond, nullptr);
    EXPECT_EQ(cond->value, "1");
}

TEST(ParserTest, ParsePostIncrementAndDecrement) {
    const char* src = "int i = 0; i++; i--;";
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto program = parser.parse();

    ASSERT_EQ(program->statements.size(), 3u);

    auto* incStmt = dynamic_cast<ExpressionStatement*>(program->statements[1].get());
    ASSERT_NE(incStmt, nullptr);
    auto* inc = dynamic_cast<PostfixExpression*>(incStmt->expression.get());
    ASSERT_NE(inc, nullptr);
    EXPECT_EQ(inc->op, "++");

    auto* decStmt = dynamic_cast<ExpressionStatement*>(program->statements[2].get());
    ASSERT_NE(decStmt, nullptr);
    auto* dec = dynamic_cast<PostfixExpression*>(decStmt->expression.get());
    ASSERT_NE(dec, nullptr);
    EXPECT_EQ(dec->op, "--");
}

TEST(ParserTest, ParseLogicalAndBitwiseExpressions) {
    const char* src = "bit a = 1b && 0b || 1b; int b = 1 & 2 | 3 ^ 4; bit c = ~0b; bit d = !1b;";
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto program = parser.parse();

    ASSERT_EQ(program->statements.size(), 4u);

    auto* a = dynamic_cast<VariableDeclaration*>(program->statements[0].get());
    ASSERT_NE(a, nullptr);
    auto* binOr = dynamic_cast<BinaryExpression*>(a->initializer.get());
    ASSERT_NE(binOr, nullptr);
    EXPECT_EQ(binOr->op, "||");
    auto* leftAnd = dynamic_cast<BinaryExpression*>(binOr->left.get());
    ASSERT_NE(leftAnd, nullptr);
    EXPECT_EQ(leftAnd->op, "&&");

    auto* b = dynamic_cast<VariableDeclaration*>(program->statements[1].get());
    ASSERT_NE(b, nullptr);
    auto* binOr2 = dynamic_cast<BinaryExpression*>(b->initializer.get());
    ASSERT_NE(binOr2, nullptr);
    EXPECT_EQ(binOr2->op, "|");
    auto* leftAnd2 = dynamic_cast<BinaryExpression*>(binOr2->left.get());
    ASSERT_NE(leftAnd2, nullptr);
    EXPECT_EQ(leftAnd2->op, "&");
    auto* rightXor = dynamic_cast<BinaryExpression*>(binOr2->right.get());
    ASSERT_NE(rightXor, nullptr);
    EXPECT_EQ(rightXor->op, "^");

    auto* c = dynamic_cast<VariableDeclaration*>(program->statements[2].get());
    ASSERT_NE(c, nullptr);
    auto* unaryTilde = dynamic_cast<UnaryExpression*>(c->initializer.get());
    ASSERT_NE(unaryTilde, nullptr);
    EXPECT_EQ(unaryTilde->op, "~");

    auto* d = dynamic_cast<VariableDeclaration*>(program->statements[3].get());
    ASSERT_NE(d, nullptr);
    auto* unaryBang = dynamic_cast<UnaryExpression*>(d->initializer.get());
    ASSERT_NE(unaryBang, nullptr);
    EXPECT_EQ(unaryBang->op, "!");
}

TEST(ParserTest, ParseArrayTypesAndIndexing) {
    const char* src =
        "int[] a; int[5] b; int[] c = {0,1,2}; function main() -> void { echo(c[1]); }";
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto program = parser.parse();

    ASSERT_GE(program->statements.size(), 3u);
    auto* a = dynamic_cast<VariableDeclaration*>(program->statements[0].get());
    auto* b = dynamic_cast<VariableDeclaration*>(program->statements[1].get());
    auto* c = dynamic_cast<VariableDeclaration*>(program->statements[2].get());
    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);
    ASSERT_NE(c, nullptr);
    auto* aType = dynamic_cast<ArrayType*>(a->varType.get());
    ASSERT_NE(aType, nullptr);
    auto* aElem = dynamic_cast<PrimitiveType*>(aType->elementType.get());
    ASSERT_NE(aElem, nullptr);
    EXPECT_EQ(aElem->name, "int");
    EXPECT_EQ(aType->size, -1);

    auto* bType = dynamic_cast<ArrayType*>(b->varType.get());
    ASSERT_NE(bType, nullptr);
    auto* bElem = dynamic_cast<PrimitiveType*>(bType->elementType.get());
    ASSERT_NE(bElem, nullptr);
    EXPECT_EQ(bElem->name, "int");
    EXPECT_EQ(bType->size, 5);

    auto* cInit = dynamic_cast<ArrayLiteralExpression*>(c->initializer.get());
    ASSERT_NE(cInit, nullptr);
    ASSERT_EQ(cInit->elements.size(), 3u);

    ASSERT_EQ(program->functions.size(), 1u);
    auto* func = program->functions[0].get();
    ASSERT_NE(func->body, nullptr);
    ASSERT_EQ(func->body->statements.size(), 1u);
    auto* echo = dynamic_cast<EchoStatement*>(func->body->statements[0].get());
    ASSERT_NE(echo, nullptr);
    auto* idx = dynamic_cast<IndexExpression*>(echo->value.get());
    ASSERT_NE(idx, nullptr);
}

TEST(ParserTest, RejectNegativeArrayIndexLiteral) {
    const char* src = "function main() -> void { int[] a = {1,2,3}; echo(a[-1]); }";
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    EXPECT_THROW((void)parser.parse(), BlochError);
}

TEST(ParserTest, ParseArrayElementAssignment) {
    const char* src = "int[] a = {1,2,3}; a[1] = 5;";
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto program = parser.parse();

    ASSERT_EQ(program->statements.size(), 2u);
    auto* assignStmt = dynamic_cast<ExpressionStatement*>(program->statements[1].get());
    ASSERT_NE(assignStmt, nullptr);
    auto* arrAssign = dynamic_cast<ArrayAssignmentExpression*>(assignStmt->expression.get());
    ASSERT_NE(arrAssign, nullptr);
    auto* collVar = dynamic_cast<VariableExpression*>(arrAssign->collection.get());
    ASSERT_NE(collVar, nullptr);
    EXPECT_EQ(collVar->name, "a");
}

TEST(ParserTest, ParsesImportsAndClasses) {
    const char* src = R"(
import foo.bar;
class Base {
    public virtual function ping() -> void;
}
class Derived extends foo.bar.Base {
    public override function ping() -> void { super.ping(); this.helper(); }
    private function helper() -> void { }
}
)";
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto program = parser.parse();

    ASSERT_EQ(program->imports.size(), 1u);
    EXPECT_EQ(program->imports[0]->path.size(), 2u);
    EXPECT_EQ(program->imports[0]->path[0], "foo");
    EXPECT_EQ(program->imports[0]->path[1], "bar");

    ASSERT_EQ(program->classes.size(), 2u);
    auto* base = program->classes[0].get();
    auto* derived = program->classes[1].get();
    EXPECT_EQ(base->name, "Base");
    ASSERT_EQ(base->members.size(), 1u);
    auto* basePing = dynamic_cast<MethodDeclaration*>(base->members[0].get());
    ASSERT_NE(basePing, nullptr);
    EXPECT_TRUE(basePing->isVirtual);
    EXPECT_EQ(derived->name, "Derived");
    ASSERT_EQ(derived->baseName.size(), 3u);
    EXPECT_EQ(derived->baseName.back(), "Base");
    ASSERT_EQ(derived->members.size(), 2u);
    auto* ping = dynamic_cast<MethodDeclaration*>(derived->members[0].get());
    ASSERT_NE(ping, nullptr);
    EXPECT_TRUE(ping->isOverride);
    ASSERT_NE(ping->body, nullptr);
    ASSERT_EQ(ping->body->statements.size(), 2u);
    auto* superCallStmt = dynamic_cast<ExpressionStatement*>(ping->body->statements[0].get());
    ASSERT_NE(superCallStmt, nullptr);
    auto* superCall = dynamic_cast<CallExpression*>(superCallStmt->expression.get());
    ASSERT_NE(superCall, nullptr);
    auto* superAccess = dynamic_cast<MemberAccessExpression*>(superCall->callee.get());
    ASSERT_NE(superAccess, nullptr);
    ASSERT_NE(dynamic_cast<SuperExpression*>(superAccess->object.get()), nullptr);
    auto* thisCallStmt = dynamic_cast<ExpressionStatement*>(ping->body->statements[1].get());
    ASSERT_NE(thisCallStmt, nullptr);
    auto* thisCall = dynamic_cast<CallExpression*>(thisCallStmt->expression.get());
    ASSERT_NE(thisCall, nullptr);
    auto* thisAccess = dynamic_cast<MemberAccessExpression*>(thisCall->callee.get());
    ASSERT_NE(thisAccess, nullptr);
    ASSERT_NE(dynamic_cast<ThisExpression*>(thisAccess->object.get()), nullptr);
}

TEST(ParserTest, RejectsMultipleExtends) {
    const char* src = "class A extends B extends C { }";
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    EXPECT_THROW((void)parser.parse(), BlochError);
}

TEST(ParserTest, StaticClassRequiresStaticMethods) {
    const char* src = R"(
static class Util {
    public function f() -> void { }
}
)";
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    EXPECT_THROW((void)parser.parse(), BlochError);
}

TEST(ParserTest, StaticClassAllowsStaticFields) {
    const char* src = R"(
static class Math {
    public static final float PI = 3.14f;
}
)";
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto program = parser.parse();

    ASSERT_EQ(program->classes.size(), 1u);
    auto* math = program->classes[0].get();
    ASSERT_EQ(math->members.size(), 1u);
    auto* field = dynamic_cast<FieldDeclaration*>(math->members[0].get());
    ASSERT_NE(field, nullptr);
    EXPECT_TRUE(field->isStatic);
    EXPECT_TRUE(field->isFinal);
}

TEST(ParserTest, StaticClassRejectsNonStaticFields) {
    const char* src = R"(
static class Math {
    public float PI = 3.14f;
}
)";
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    EXPECT_THROW((void)parser.parse(), BlochError);
}

TEST(ParserTest, ConstructorReturnTypeMustMatchClass) {
    const char* src = R"(
class Foo {
    public constructor() -> Bar { }
}
)";
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    EXPECT_THROW((void)parser.parse(), BlochError);
}

TEST(ParserTest, DestructorValidation) {
    const char* badReturn = R"(
class Foo {
    public destructor -> int { }
}
)";
    Lexer lexer1(badReturn);
    auto tokens1 = lexer1.tokenize();
    Parser parser1(std::move(tokens1));
    EXPECT_THROW((void)parser1.parse(), BlochError);

    const char* badParams = R"(
class Foo {
    public destructor(x) -> void { }
}
)";
    Lexer lexer2(badParams);
    auto tokens2 = lexer2.tokenize();
    Parser parser2(std::move(tokens2));
    EXPECT_THROW((void)parser2.parse(), BlochError);
}

TEST(ParserTest, ParsesNewAndDestroyWithClassVariables) {
    const char* src = R"(
class Foo { }
function main() -> void {
    Foo f = new Foo();
    destroy f;
}
)";
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto program = parser.parse();

    ASSERT_EQ(program->functions.size(), 1u);
    auto* mainFn = program->functions[0].get();
    ASSERT_NE(mainFn->body, nullptr);
    ASSERT_EQ(mainFn->body->statements.size(), 2u);
    auto* decl = dynamic_cast<VariableDeclaration*>(mainFn->body->statements[0].get());
    ASSERT_NE(decl, nullptr);
    auto* newExpr = dynamic_cast<NewExpression*>(decl->initializer.get());
    ASSERT_NE(newExpr, nullptr);
    auto* destroyStmt = dynamic_cast<DestroyStatement*>(mainFn->body->statements[1].get());
    ASSERT_NE(destroyStmt, nullptr);
    auto* destroyVar = dynamic_cast<VariableExpression*>(destroyStmt->target.get());
    ASSERT_NE(destroyVar, nullptr);
    EXPECT_EQ(destroyVar->name, "f");
}

TEST(ParserTest, ParsesDefaultConstructor) {
    const char* src = R"(
class Foo {
    int x;
    public constructor(int x) -> Foo = default;
}
)";
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto program = parser.parse();

    ASSERT_EQ(program->classes.size(), 1u);
    auto* foo = program->classes[0].get();
    ASSERT_EQ(foo->members.size(), 2u);
    auto* ctor = dynamic_cast<ConstructorDeclaration*>(foo->members[1].get());
    ASSERT_NE(ctor, nullptr);
    EXPECT_TRUE(ctor->isDefault);
    EXPECT_EQ(ctor->params.size(), 1u);
    EXPECT_EQ(ctor->params[0]->name, "x");
    EXPECT_EQ(ctor->body, nullptr);
}

TEST(ParserTest, ParsesDefaultDestructor) {
    const char* src = R"(
class Foo {
    public destructor -> void = default;
}
)";
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto program = parser.parse();

    ASSERT_EQ(program->classes.size(), 1u);
    auto* foo = program->classes[0].get();
    ASSERT_EQ(foo->members.size(), 1u);
    auto* dtor = dynamic_cast<DestructorDeclaration*>(foo->members[0].get());
    ASSERT_NE(dtor, nullptr);
    EXPECT_TRUE(dtor->isDefault);
    EXPECT_EQ(dtor->body, nullptr);
}
