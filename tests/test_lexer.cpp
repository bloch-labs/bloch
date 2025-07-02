#include <gtest/gtest.h>
#include "bloch/error/bloch_runtime_error.hpp"
#include "bloch/lexer/lexer.hpp"

using namespace bloch;

TEST(LexerTest, Identifiers) {
    Lexer lexer("hello world");
    auto tokens = lexer.tokenize();

    ASSERT_GE(tokens.size(), 3);
    EXPECT_EQ(tokens[0].type, TokenType::Identifier);
    EXPECT_EQ(tokens[0].value, "hello");

    EXPECT_EQ(tokens[1].type, TokenType::Identifier);
    EXPECT_EQ(tokens[1].value, "world");

    EXPECT_EQ(tokens.back().type, TokenType::Eof);
}

TEST(LexerTest, IntegerLiteral) {
    Lexer lexer("12345");
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0].type, TokenType::IntegerLiteral);
    EXPECT_EQ(tokens[0].value, "12345");
}

TEST(LexerTest, FloatLiteral) {
    Lexer lexer("3.14f");
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0].type, TokenType::FloatLiteral);
    EXPECT_EQ(tokens[0].value, "3.14f");
}

TEST(LexerTest, KeywordDetection) {
    Lexer lexer("int float return");
    auto tokens = lexer.tokenize();

    EXPECT_EQ(tokens[0].type, TokenType::Int);
    EXPECT_EQ(tokens[1].type, TokenType::Float);
    EXPECT_EQ(tokens[2].type, TokenType::Return);
}

TEST(LexerTest, LogicalKeyword) {
    Lexer lexer("logical");
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0].type, TokenType::Logical);
    EXPECT_EQ(tokens[0].value, "logical");
}

TEST(LexerTest, Operators) {
    Lexer lexer("-> + - * / ;");
    auto tokens = lexer.tokenize();

    EXPECT_EQ(tokens[0].type, TokenType::Arrow);
    EXPECT_EQ(tokens[1].type, TokenType::Plus);
    EXPECT_EQ(tokens[2].type, TokenType::Minus);
    EXPECT_EQ(tokens[3].type, TokenType::Star);
    EXPECT_EQ(tokens[4].type, TokenType::Slash);
    EXPECT_EQ(tokens[5].type, TokenType::Semicolon);
}

TEST(LexerTest, StringLiteral) {
    Lexer lexer("\"hello\"");
    auto tokens = lexer.tokenize();

    EXPECT_EQ(tokens[0].type, TokenType::StringLiteral);
    EXPECT_EQ(tokens[0].value, "\"hello\"");
}

TEST(LexerTest, CharLiteral) {
    Lexer lexer("'a'");
    auto tokens = lexer.tokenize();

    EXPECT_EQ(tokens[0].type, TokenType::CharLiteral);
    EXPECT_EQ(tokens[0].value, "'a'");
}

TEST(LexerTest, UnterminatedStringThrows) {
    Lexer lexer("\"hello");

    EXPECT_THROW({ lexer.tokenize(); }, BlochRuntimeError);
}

TEST(LexerTest, UnterminatedCharThrows) {
    Lexer lexer("'a");

    EXPECT_THROW(lexer.tokenize(), BlochRuntimeError);
}

TEST(LexerTest, MalformedFloatThrows) {
    Lexer lexer("3.14");

    EXPECT_THROW(lexer.tokenize(), BlochRuntimeError);
}

TEST(LexerTest, LineAndColumnTracking) {
    Lexer lexer("a\nb");
    auto tokens = lexer.tokenize();

    ASSERT_GE(tokens.size(), 3);
    EXPECT_EQ(tokens[0].line, 1);
    EXPECT_EQ(tokens[0].column, 1);
    EXPECT_EQ(tokens[1].line, 2);
    EXPECT_EQ(tokens[1].column, 1);
}

TEST(LexerTest, SkipsComments) {
    Lexer lexer("int x // comment\ny");
    auto tokens = lexer.tokenize();

    ASSERT_GE(tokens.size(), 4);
    EXPECT_EQ(tokens[0].type, TokenType::Int);
    EXPECT_EQ(tokens[1].type, TokenType::Identifier);
    EXPECT_EQ(tokens[1].value, "x");
    EXPECT_EQ(tokens[2].type, TokenType::Identifier);
    EXPECT_EQ(tokens[2].value, "y");
}