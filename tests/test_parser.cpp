#include <gtest/gtest.h>
#include "bloch/error/bloch_runtime_error.hpp"
#include "bloch/lexer/lexer.hpp"
#include "bloch/parser/parser.hpp"

using namespace bloch;

TEST(ParserTest, ParseImport) {
    Lexer lexer("import math;");
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto program = parser.parse();

    ASSERT_EQ(program->imports.size(), 1u);
    EXPECT_EQ(program->imports[0]->module, "math");
}

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
    EXPECT_THROW(parser.parse(), BlochRuntimeError);
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

TEST(ParserTest, ParseClass) {
    const char* src =
        "class Foo { @members(\"public\"): int x; @methods: function bar() -> int { return 5; } }";
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto program = parser.parse();

    ASSERT_EQ(program->classes.size(), 1u);

    auto* clazz = program->classes[0].get();
    EXPECT_EQ(clazz->name, "Foo");
    ASSERT_EQ(clazz->members.size(), 1u);
    EXPECT_EQ(clazz->members[0]->name, "x");
    ASSERT_EQ(clazz->methods.size(), 1u);
    EXPECT_EQ(clazz->methods[0]->name, "bar");
    EXPECT_FALSE(clazz->methods[0]->isConstructor);

    auto* retType = dynamic_cast<PrimitiveType*>(clazz->methods[0]->returnType.get());
    ASSERT_NE(retType, nullptr);
    EXPECT_EQ(retType->name, "int");
}

TEST(ParserTest, ParseConstructorFunction) {
    const char* src = "function *Foo() -> void { }";
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto program = parser.parse();

    ASSERT_EQ(program->functions.size(), 1u);

    auto* func = program->functions[0].get();
    EXPECT_TRUE(func->isConstructor);
    EXPECT_EQ(func->name, "Foo");
    ASSERT_EQ(func->params.size(), 0u);

    auto* retType = dynamic_cast<VoidType*>(func->returnType.get());
    ASSERT_NE(retType, nullptr);
    ASSERT_NE(func->body, nullptr);
    ASSERT_EQ(func->body->statements.size(), 0u);
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