#pragma once

#include <string>
#include <vector>
#include "../error/bloch_runtime_error.hpp"
#include "token.hpp"

namespace bloch {
    class Lexer {
       public:
        explicit Lexer(const std::string& source);
        std::vector<Token> tokenize();

       private:
        std::string m_source;
        size_t m_position;
        int m_line;
        int m_column;

        char peek() const;
        char peekNext() const;
        char advance();

        bool match(char expected);

        void skipWhitespace();
        void skipComment();
        void reportError(const std::string& msg);

        Token makeToken(TokenType type, const std::string& value);
        Token scanToken();
        Token scanNumber();
        Token scanIdentifierOrKeyword();
        Token scanString();
        Token scanChar();
    };
}