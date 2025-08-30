#include "lexer.hpp"

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string_view>
#include <unordered_map>
#include "../error/bloch_runtime_error.hpp"

namespace bloch {
    Lexer::Lexer(const std::string_view source) noexcept
        : m_source(source), m_position(0), m_line(1), m_column(1) {}

    std::vector<Token> Lexer::tokenize() {
        std::vector<Token> tokens;
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
        while (m_position < m_source.size()) {
            char c = peek();
            if (isspace(c)) {
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
        while (m_position < m_source.size() && m_source[m_position] != '\n') {
            (void)advance();
        }
    }

    void Lexer::reportError(const std::string& msg) {
        throw BlochRuntimeError("Bloch Lexer Error", m_line, m_column, msg);
    }

    Token Lexer::makeToken(TokenType type, const std::string& value) {
        return Token{type, value, m_line, m_column - static_cast<int>(value.length())};
    }

    Token Lexer::scanToken() {
        char c = advance();

        if (isdigit(c))
            return scanNumber();
        if (isalpha(c) || c == '_')
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
        size_t start = m_position - 1;
        while (isdigit(peek())) (void)advance();

        if (peek() == '.') {
            (void)advance();
            while (isdigit(peek())) (void)advance();
            if (peek() == 'f') {
                (void)advance();
                return makeToken(TokenType::FloatLiteral,
                                 std::string(m_source.substr(start, m_position - start)));
            } else {
                reportError("Float literal must end with 'f'");
                return makeToken(TokenType::Unknown,
                                 std::string(m_source.substr(start, m_position - start)));
            }
        }

        if (peek() == 'b') {
            std::string_view digits = m_source.substr(start, m_position - start);
            if (digits != "0" && digits != "1") {
                reportError("Bit literal must be '0b' or '1b'");
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
        size_t start = m_position - 1;
        while (isalnum(peek()) || peek() == '_') (void)advance();

        std::string_view text = m_source.substr(start, m_position - start);

        static const std::unordered_map<std::string_view, TokenType> keywords = {

            // Primitives
            {"int", TokenType::Int},
            {"float", TokenType::Float},
            {"string", TokenType::String},
            {"char", TokenType::Char},
            {"qubit", TokenType::Qubit},
            {"bit", TokenType::Bit},

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

            // Annotation Values
            {"quantum", TokenType::Quantum},
            {"adjoint", TokenType::Adjoint},
            {"tracked", TokenType::Tracked},

            // Built ins
            {"echo", TokenType::Echo}};

        auto it = keywords.find(text);
        if (it != keywords.end()) {
            return makeToken(it->second, std::string(text));
        }

        return makeToken(TokenType::Identifier, std::string(text));
    }

    Token Lexer::scanString() {
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

        reportError("Unterminated string literal");
        return makeToken(TokenType::Unknown,
                         std::string(m_source.substr(start - 1, m_position - start + 1)));
    }

    Token Lexer::scanChar() {
        size_t start = m_position;
        if (m_position < m_source.size())
            (void)advance();

        if (peek() == '\'') {
            (void)advance();
            return makeToken(TokenType::CharLiteral, std::string(m_source.substr(start - 1, 3)));
        }

        reportError("Unterminated char literal");
        return makeToken(TokenType::Unknown,
                         std::string(m_source.substr(start - 1, m_position - start + 1)));
    }
}