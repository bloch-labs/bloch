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

#include "bloch/compiler/lexer/lexer.hpp"

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string_view>
#include <unordered_map>

namespace bloch::compiler {

    using support::BlochError;
    using support::ErrorCategory;
    Lexer::Lexer(const std::string_view source) noexcept
        : m_source(source), m_position(0), m_line(1), m_column(1) {}

    std::vector<Token> Lexer::tokenize() {
        std::vector<Token> tokens;
        // We repeatedly skip trivia and scan the next meaningful token
        // until we run out of input. Always append an explicit EOF token.
        while (m_position < m_source.size()) {
            skipWhitespace();
            if (m_position < m_source.size()) {
                tokens.push_back(scanToken());
            }
        }
        tokens.push_back(makeToken(TokenType::Eof, ""));
        return tokens;
    }

    char Lexer::peek() const noexcept {
        return m_position < m_source.size() ? m_source[m_position] : '\0';
    }

    char Lexer::peekNext() const noexcept {
        return (m_position + 1) < m_source.size() ? m_source[m_position + 1] : '\0';
    }

    char Lexer::advance() noexcept {
        char c = m_source[m_position++];
        m_column++;
        return c;
    }

    bool Lexer::match(char expected) noexcept {
        if (m_position >= m_source.size() || m_source[m_position] != expected) {
            return false;
        }
        m_position++;
        m_column++;
        return true;
    }

    void Lexer::skipWhitespace() {
        // Eat spaces, tabs and newlines. Treat // as a line comment.
        while (m_position < m_source.size()) {
            char c = peek();
            if (std::isspace(static_cast<unsigned char>(c))) {
                if (c == '\n') {
                    (void)advance();
                    m_line++;
                    m_column = 1;
                    continue;
                }
                (void)advance();
            } else if (c == '/' && peekNext() == '/') {
                (void)advance();
                (void)advance();
                skipComment();
            } else {
                break;
            }
        }
    }

    void Lexer::skipComment() {
        // Consume the rest of the current line.
        while (m_position < m_source.size() && m_source[m_position] != '\n') {
            (void)advance();
        }
    }

    void Lexer::reportError(const std::string& msg) {
        throw BlochError(ErrorCategory::Lexical, m_line, m_column, msg);
    }

    Token Lexer::makeToken(TokenType type, const std::string& value) {
        // Column is adjusted so error spans point to token start.
        return Token{type, value, m_line, m_column - static_cast<int>(value.length())};
    }

    Token Lexer::scanToken() {
        char c = advance();

        // Fast paths for common leading characters
        if (std::isdigit(static_cast<unsigned char>(c)))
            return scanNumber();
        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_')
            return scanIdentifierOrKeyword();

        switch (c) {
            case '=':
                return match('=') ? makeToken(TokenType::EqualEqual, "==")
                                  : makeToken(TokenType::Equals, "=");
            case '!':
                return match('=') ? makeToken(TokenType::BangEqual, "!=")
                                  : makeToken(TokenType::Bang, "!");
            case '+':
                return match('+') ? makeToken(TokenType::PlusPlus, "++")
                                  : makeToken(TokenType::Plus, "+");
            case '&':
                return match('&') ? makeToken(TokenType::AmpersandAmpersand, "&&")
                                  : makeToken(TokenType::Ampersand, "&");
            case '|':
                return match('|') ? makeToken(TokenType::PipePipe, "||")
                                  : makeToken(TokenType::Pipe, "|");
            case '^':
                return makeToken(TokenType::Caret, "^");
            case '~':
                return makeToken(TokenType::Tilde, "~");
            case '-':
                if (match('>'))
                    return makeToken(TokenType::Arrow, "->");
                return match('-') ? makeToken(TokenType::MinusMinus, "--")
                                  : makeToken(TokenType::Minus, "-");
            case '*':
                return makeToken(TokenType::Star, "*");
            case '/':
                return makeToken(TokenType::Slash, "/");
            case '%':
                return makeToken(TokenType::Percent, "%");
            case '>':
                return match('=') ? makeToken(TokenType::GreaterEqual, ">=")
                                  : makeToken(TokenType::Greater, ">");
            case '<':
                return match('=') ? makeToken(TokenType::LessEqual, "<=")
                                  : makeToken(TokenType::Less, "<");
            case '?':
                return makeToken(TokenType::Question, "?");
            case ':':
                return makeToken(TokenType::Colon, ":");
            case '.':
                return makeToken(TokenType::Dot, ".");
            case ';':
                return makeToken(TokenType::Semicolon, ";");
            case ',':
                return makeToken(TokenType::Comma, ",");
            case '@':
                return makeToken(TokenType::At, "@");
            case '"':
                return scanString();
            case '\'':
                return scanChar();
            case '(':
                return makeToken(TokenType::LParen, "(");
            case ')':
                return makeToken(TokenType::RParen, ")");
            case '{':
                return makeToken(TokenType::LBrace, "{");
            case '}':
                return makeToken(TokenType::RBrace, "}");
            case '[':
                return makeToken(TokenType::LBracket, "[");
            case ']':
                return makeToken(TokenType::RBracket, "]");
            default:
                return makeToken(TokenType::Unknown, std::string(1, c));
        }
    }

    Token Lexer::scanNumber() {
        // Integers by default; a trailing '.<digits>f' upgrades to float,
        // and a trailing 'b' turns a 0/1 into a bit literal.
        size_t start = m_position - 1;
        while (std::isdigit(static_cast<unsigned char>(peek()))) (void)advance();

        if (peek() == '.') {
            (void)advance();
            while (std::isdigit(static_cast<unsigned char>(peek()))) (void)advance();
            if (peek() == 'f') {
                (void)advance();
                return makeToken(TokenType::FloatLiteral,
                                 std::string(m_source.substr(start, m_position - start)));
            } else {
                reportError("float literals must end with 'f'");
                return makeToken(TokenType::Unknown,
                                 std::string(m_source.substr(start, m_position - start)));
            }
        }

        // Support integer part followed directly by 'f' (e.g., 3f)
        if (peek() == 'f') {
            (void)advance();
            return makeToken(TokenType::FloatLiteral,
                             std::string(m_source.substr(start, m_position - start)));
        }

        if (peek() == 'L') {
            (void)advance();
            return makeToken(TokenType::LongLiteral,
                             std::string(m_source.substr(start, m_position - start)));
        }

        if (peek() == 'b') {
            std::string_view digits = m_source.substr(start, m_position - start);
            if (digits != "0" && digits != "1") {
                reportError("bit literals must be 0b or 1b");
                (void)advance();
                return makeToken(TokenType::Unknown,
                                 std::string(m_source.substr(start, m_position - start)));
            }
            (void)advance();
            return makeToken(TokenType::BitLiteral,
                             std::string(m_source.substr(start, m_position - start)));
        }

        return makeToken(TokenType::IntegerLiteral,
                         std::string(m_source.substr(start, m_position - start)));
    }

    Token Lexer::scanIdentifierOrKeyword() {
        // Identifiers are [A-Za-z_][A-Za-z0-9_]*. Some of them are keywords.
        size_t start = m_position - 1;
        while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_') (void)advance();

        std::string_view text = m_source.substr(start, m_position - start);

        static const std::unordered_map<std::string_view, TokenType> keywords = {

            // Primitives
            {"null", TokenType::Null},
            {"int", TokenType::Int},
            {"long", TokenType::Long},
            {"float", TokenType::Float},
            {"string", TokenType::String},
            {"char", TokenType::Char},
            {"qubit", TokenType::Qubit},
            {"bit", TokenType::Bit},
            {"boolean", TokenType::Boolean},

            // Boolean literals (treated as keywords for convenience)
            {"true", TokenType::True},
            {"false", TokenType::False},

            // Keywords
            {"void", TokenType::Void},
            {"function", TokenType::Function},
            {"return", TokenType::Return},
            {"if", TokenType::If},
            {"else", TokenType::Else},
            {"for", TokenType::For},
            {"while", TokenType::While},
            {"measure", TokenType::Measure},
            {"final", TokenType::Final},
            {"reset", TokenType::Reset},
            {"default", TokenType::Default},

            // Annotation Values
            {"quantum", TokenType::Quantum},
            {"tracked", TokenType::Tracked},
            {"shots", TokenType::Shots},

            // Class System
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
            {"package", TokenType::Package},
            {"new", TokenType::New},
            {"constructor", TokenType::Constructor},
            {"destructor", TokenType::Destructor},
            {"destroy", TokenType::Destroy},

            // Built ins
            {"echo", TokenType::Echo}};

        auto it = keywords.find(text);
        if (it != keywords.end()) {
            return makeToken(it->second, std::string(text));
        }

        return makeToken(TokenType::Identifier, std::string(text));
    }

    Token Lexer::scanString() {
        // Strings are double-quoted and may span lines; we do not process escapes yet.
        size_t start = m_position;
        while (m_position < m_source.size() && peek() != '"') {
            if (peek() == '\n')
                m_line++;
            (void)advance();
        }

        if (peek() == '"') {
            (void)advance();
            return makeToken(TokenType::StringLiteral,
                             std::string(m_source.substr(start - 1, m_position - start + 1)));
        }

        reportError("unterminated string literal");
        return makeToken(TokenType::Unknown,
                         std::string(m_source.substr(start - 1, m_position - start + 1)));
    }

    Token Lexer::scanChar() {
        // Char literals are simple: '\'' X '\'' with no escaping support for now.
        size_t start = m_position;
        if (m_position < m_source.size())
            (void)advance();

        if (peek() == '\'') {
            (void)advance();
            return makeToken(TokenType::CharLiteral, std::string(m_source.substr(start - 1, 3)));
        }

        reportError("unterminated char literal");
        return makeToken(TokenType::Unknown,
                         std::string(m_source.substr(start - 1, m_position - start + 1)));
    }
}  // namespace bloch::compiler
