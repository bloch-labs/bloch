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
#include "test_framework.hpp"

using namespace bloch;

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

TEST(ParserTest, StateAnnotationIsRejected) {
    const char* src = "@state(\"+\") qubit q;";
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
