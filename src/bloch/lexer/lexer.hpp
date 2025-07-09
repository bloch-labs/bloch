#pragma once

#include <string>
#include <vector>
#include "../error/bloch_runtime_error.hpp"
#include "token.hpp"

namespace bloch {
    class Lexer {
       public:
        explicit Lexer(const std::string_view source) noexcept;
        [[nodiscard]] std::vector<Token> tokenize();

       private:
        std::string_view m_source;
        size_t m_position;
        int m_line;
        int m_column;

        [[nodiscard]] char peek() const noexcept;
        [[nodiscard]] char peekNext() const noexcept;
        [[nodiscard]] char advance() noexcept;

        [[nodiscard]] bool match(char expected) noexcept;

        void skipWhitespace();
        void skipComment();
        [[noreturn]] void reportError(const std::string& msg);

        [[nodiscard]] Token makeToken(TokenType type, const std::string& value);
        [[nodiscard]] Token scanToken();
        [[nodiscard]] Token scanNumber();
        [[nodiscard]] Token scanIdentifierOrKeyword();
        [[nodiscard]] Token scanString();
        [[nodiscard]] Token scanChar();
    };
}