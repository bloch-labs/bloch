// Copyright 2025-2026 Akshay Pal (https://bloch-labs.com)
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

#include <string>
#include <utility>
#include <vector>

#include "bloch/compiler/lexer/lexer.hpp"
#include "bloch/support/error/bloch_error.hpp"
#include "test_framework.hpp"

using namespace bloch::compiler;
using bloch::support::BlochError;

TEST(LexerTest, Identifiers) {
    Lexer lexer("hello world");
    auto tokens = lexer.tokenize();

    ASSERT_GE(tokens.size(), 3u);
    EXPECT_EQ(tokens[0].type, TokenType::Identifier);
    EXPECT_EQ(tokens[0].value, "hello");

    EXPECT_EQ(tokens[1].type, TokenType::Identifier);
    EXPECT_EQ(tokens[1].value, "world");

    EXPECT_EQ(tokens.back().type, TokenType::Eof);
}

TEST(LexerTest, IntegerLiteral) {
    Lexer lexer("12345");
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].type, TokenType::IntegerLiteral);
    EXPECT_EQ(tokens[0].value, "12345");
}

TEST(LexerTest, FloatLiteral) {
    Lexer lexer("3.14f");
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].type, TokenType::FloatLiteral);
    EXPECT_EQ(tokens[0].value, "3.14f");
}

TEST(LexerTest, FloatLiteralIntegerF) {
    Lexer lexer("3f");
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].type, TokenType::FloatLiteral);
    EXPECT_EQ(tokens[0].value, "3f");
}

TEST(LexerTest, BitLiteral) {
    Lexer lexer("1b");
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].type, TokenType::BitLiteral);
    EXPECT_EQ(tokens[0].value, "1b");
}

TEST(LexerTest, KeywordDetection) {
    Lexer lexer("int float return");
    auto tokens = lexer.tokenize();

    EXPECT_EQ(tokens[0].type, TokenType::Int);
    EXPECT_EQ(tokens[1].type, TokenType::Float);
    EXPECT_EQ(tokens[2].type, TokenType::Return);
}

TEST(LexerTest, NullKeyword) {
    Lexer lexer("null");
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].type, TokenType::Null);
}

TEST(LexerTest, Operators) {
    Lexer lexer("-> + - * / == != ;");
    auto tokens = lexer.tokenize();

    EXPECT_EQ(tokens[0].type, TokenType::Arrow);
    EXPECT_EQ(tokens[1].type, TokenType::Plus);
    EXPECT_EQ(tokens[2].type, TokenType::Minus);
    EXPECT_EQ(tokens[3].type, TokenType::Star);
    EXPECT_EQ(tokens[4].type, TokenType::Slash);
    EXPECT_EQ(tokens[5].type, TokenType::EqualEqual);
    EXPECT_EQ(tokens[6].type, TokenType::BangEqual);
    EXPECT_EQ(tokens[7].type, TokenType::Semicolon);
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

    EXPECT_THROW({ (void)lexer.tokenize(); }, BlochError);
}

TEST(LexerTest, UnterminatedCharThrows) {
    Lexer lexer("'a");

    EXPECT_THROW((void)lexer.tokenize(), BlochError);
}

TEST(LexerTest, MalformedFloatThrows) {
    Lexer lexer("3.14");

    EXPECT_THROW((void)lexer.tokenize(), BlochError);
}

TEST(LexerTest, LineAndColumnTracking) {
    Lexer lexer("a\nb");
    auto tokens = lexer.tokenize();

    ASSERT_GE(tokens.size(), 3u);
    EXPECT_EQ(tokens[0].line, 1);
    EXPECT_EQ(tokens[0].column, 1);
    EXPECT_EQ(tokens[1].line, 2);
    EXPECT_EQ(tokens[1].column, 1);
}

TEST(LexerTest, SkipsComments) {
    Lexer lexer("int x // comment\ny");
    auto tokens = lexer.tokenize();

    ASSERT_GE(tokens.size(), 4u);
    EXPECT_EQ(tokens[0].type, TokenType::Int);
    EXPECT_EQ(tokens[1].type, TokenType::Identifier);
    EXPECT_EQ(tokens[1].value, "x");
    EXPECT_EQ(tokens[2].type, TokenType::Identifier);
    EXPECT_EQ(tokens[2].value, "y");
}

TEST(LexerTest, IncrementDecrement) {
    Lexer lexer("i++ j--");
    auto tokens = lexer.tokenize();

    ASSERT_GE(tokens.size(), 5u);
    EXPECT_EQ(tokens[0].type, TokenType::Identifier);
    EXPECT_EQ(tokens[1].type, TokenType::PlusPlus);
    EXPECT_EQ(tokens[2].type, TokenType::Identifier);
    EXPECT_EQ(tokens[3].type, TokenType::MinusMinus);
}

TEST(LexerTest, LogicalAndBitwiseOperators) {
    Lexer lexer("&& || & | ^ ~ !");
    auto tokens = lexer.tokenize();

    EXPECT_EQ(tokens[0].type, TokenType::AmpersandAmpersand);
    EXPECT_EQ(tokens[1].type, TokenType::PipePipe);
    EXPECT_EQ(tokens[2].type, TokenType::Ampersand);
    EXPECT_EQ(tokens[3].type, TokenType::Pipe);
    EXPECT_EQ(tokens[4].type, TokenType::Caret);
    EXPECT_EQ(tokens[5].type, TokenType::Tilde);
    EXPECT_EQ(tokens[6].type, TokenType::Bang);
}

TEST(LexerTest, ClassSystemKeywords) {
    std::vector<std::pair<std::string, TokenType>> keywords = {
        {"class", TokenType::Class},
        {"public", TokenType::Public},
        {"private", TokenType::Private},
        {"protected", TokenType::Protected},
        {"static", TokenType::Static},
        {"extends", TokenType::Extends},
        {"abstract", TokenType::Abstract},
        {"virtual", TokenType::Virtual},
        {"override", TokenType::Override},
        {"super", TokenType::Super},
        {"this", TokenType::This},
        {"import", TokenType::Import},
        {"new", TokenType::New},
        {"constructor", TokenType::Constructor},
        {"destructor", TokenType::Destructor},
        {"destroy", TokenType::Destroy}};

    std::string src;
    for (const auto& kv : keywords) {
        if (!src.empty())
            src += " ";
        src += kv.first;
    }

    Lexer lexer(src);
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), keywords.size() + 1);
    for (size_t i = 0; i < keywords.size(); ++i) {
        EXPECT_EQ(tokens[i].type, keywords[i].second);
        EXPECT_EQ(tokens[i].value, keywords[i].first);
    }
    EXPECT_EQ(tokens.back().type, TokenType::Eof);
}

TEST(LexerTest, ClassSystemKeywordLookalikesStayIdentifiers) {
    std::vector<std::string> identifiers = {
        "classy",     "publicize",    "privateer",   "protectedness", "statico",  "extendsion",
        "abstracted", "virtualized",  "overridee",   "superposition", "thisness", "importer",
        "newton",     "constructorx", "destructora", "destroyer"};
    std::string src;
    for (const auto& id : identifiers) {
        if (!src.empty())
            src += " ";
        src += id;
    }

    Lexer lexer(src);
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), identifiers.size() + 1);
    for (size_t i = 0; i < identifiers.size(); ++i) {
        EXPECT_EQ(tokens[i].type, TokenType::Identifier);
        EXPECT_EQ(tokens[i].value, identifiers[i]);
    }
    EXPECT_EQ(tokens.back().type, TokenType::Eof);
}
