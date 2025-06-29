#include "lexer.hpp"

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include "../error/bloch_runtime_error.hpp"

namespace bloch {
    Lexer::Lexer(const std::string& source) : source(source), position(0), line(1), column(1) {}

    std::vector<Token> Lexer::tokenize() {
        std::vector<Token> tokens;
        while (position < source.length()) {
            skipWhitespace();
            if (position < source.length()) {
                tokens.push_back(scanToken());
            }
        }
        tokens.push_back(makeToken(TokenType::Eof, ""));
        return tokens;
    }

    char Lexer::peek() const { return position < source.length() ? source[position] : '\0'; }

    char Lexer::advance() {
        char c = source[position++];
        column++;
        return c;
    }

    bool Lexer::match(char expected) {
        if (position >= source.length() || source[position] != expected) {
            return false;
        }
        position++;
        column++;
        return true;
    }

    void Lexer::skipWhitespace() {
        while (position < source.length()) {
            char c = peek();
            if (isspace(c)) {
                if (c == '\n') {
                    advance();
                    line++;
                    column = 1;
                    continue;
                }
                advance();
            } else if (c == '#') {
                skipComment();
            } else {
                break;
            }
        }
    }

    void Lexer::skipComment() {
        while (position < source.length() && source[position] != '\n') {
            advance();
        }
    }

    void Lexer::reportError(const std::string& msg) {
        throw BlochRuntimeError("Bloch Lexer Error", line, column, msg);
    }

    Token Lexer::makeToken(TokenType type, const std::string& value) {
        return Token{type, value, line, column - static_cast<int>(value.length())};
    }

    Token Lexer::scanToken() {
        char c = advance();

        if (isdigit(c))
            return scanNumber();
        if (isalpha(c) || c == '_')
            return scanIdentifierOrKeyword();

        switch (c) {
            case '=':
                return makeToken(TokenType::Equals, "=");
            case '+':
                return makeToken(TokenType::Plus, "+");
            case '-':
                return match('>') ? makeToken(TokenType::Arrow, "->")
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
            case ';':
                return makeToken(TokenType::Semicolon, ";");
            case ',':
                return makeToken(TokenType::Comma, ",");
            case '.':
                return makeToken(TokenType::Dot, ".");
            case ':':
                return makeToken(TokenType::Colon, ":");
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
        size_t start = position - 1;
        while (isdigit(peek())) advance();

        if (peek() == '.') {
            advance();
            while (isdigit(peek())) advance();
            if (peek() == 'f') {
                advance();
                return makeToken(TokenType::FloatLiteral, source.substr(start, position - start));
            } else {
                reportError("Float literal must end with 'f'");
                return makeToken(TokenType::Unknown, source.substr(start, position - start));
            }
        }
        return makeToken(TokenType::IntegerLiteral, source.substr(start, position - start));
    }

    Token Lexer::scanIdentifierOrKeyword() {
        size_t start = position - 1;
        while (isalnum(peek()) || peek() == '_') advance();

        std::string text = source.substr(start, position - start);

        static const std::unordered_map<std::string, TokenType> keywords = {

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
            {"import", TokenType::Import},
            {"return", TokenType::Return},
            {"if", TokenType::If},
            {"for", TokenType::For},
            {"class", TokenType::Class},
            {"measure", TokenType::Measure},
            {"final", TokenType::Final},
            {"reset", TokenType::Reset},
            {"public", TokenType::Public},
            {"private", TokenType::Private},

            // Annotation Values
            {"quantum", TokenType::Quantum},
            {"adjoint", TokenType::Adjoint},
            {"state", TokenType::State},
            {"members", TokenType::Members},
            {"methods", TokenType::Methods},

            // Built ins
            {"echo", TokenType::Echo}};

        auto it = keywords.find(text);
        if (it != keywords.end()) {
            return makeToken(it->second, text);
        }

        return makeToken(TokenType::Identifier, text);
    }

    Token Lexer::scanString() {
        size_t start = position;
        while (position < source.length() && peek() != '"') {
            if (peek() == '\n')
                line++;
            advance();
        }

        if (peek() == '"') {
            advance();
            return makeToken(TokenType::StringLiteral,
                             source.substr(start - 1, position - start + 1));
        }

        reportError("Unterminated string literal");
        return makeToken(TokenType::Unknown, source.substr(start - 1, position - start + 1));
    }

    Token Lexer::scanChar() {
        size_t start = position;
        if (position < source.length())
            advance();

        if (peek() == '\'') {
            advance();
            return makeToken(TokenType::CharLiteral, source.substr(start - 1, 3));
        }

        reportError("Unterminated char literal");
        return makeToken(TokenType::Unknown, source.substr(start - 1, position - start + 1));
    }
}