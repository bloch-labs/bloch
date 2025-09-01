#pragma once

#include <string>
#include <string_view>
#include <vector>
#include "../error/bloch_error.hpp"
#include "token.hpp"

namespace bloch {
    // The Lexer turns raw source into a flat stream of tokens.
    // It is intentionally simple: single-pass, no backtracking, and
    // only enough lookahead for two-character operators and // comments.
    class Lexer {
       public:
        explicit Lexer(const std::string_view source) noexcept;
        [[nodiscard]] std::vector<Token> tokenize();

       private:
        std::string_view m_source;
        size_t m_position;
        int m_line;
        int m_column;

        // Character helpers
        [[nodiscard]] char peek() const noexcept;
        [[nodiscard]] char peekNext() const noexcept;
        [[nodiscard]] char advance() noexcept;
        [[nodiscard]] bool match(char expected) noexcept;

        // Trivia and errors
        void skipWhitespace();
        void skipComment();
        [[noreturn]] void reportError(const std::string& msg);

        // Token producers
        [[nodiscard]] Token makeToken(TokenType type, const std::string& value);
        [[nodiscard]] Token scanToken();
        [[nodiscard]] Token scanNumber();
        [[nodiscard]] Token scanIdentifierOrKeyword();
        [[nodiscard]] Token scanString();
        [[nodiscard]] Token scanChar();
    };
}
