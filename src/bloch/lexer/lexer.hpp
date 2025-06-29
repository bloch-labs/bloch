#pragma once

#include "token.hpp"
#include <string>
#include <vector>

namespace bloch {
    struct LexerError {
  std::string message;
  int line;
  int column;
};

class Lexer {
public:
  explicit Lexer(const std::string &source);
  std::vector<Token> tokenize();

private:
  std::string source;
  size_t position;
  int line;
  int column;

  char peek() const;
  char advance();

  bool match(char expected);

  void skipWhitespace();
  void skipComment();
  void reportError(const std::string &msg);

  Token makeToken(TokenType type, const std::string &value);
  Token scanToken();
  Token scanNumber();
  Token scanIdentifierOrKeyword();
  Token scanString();
  Token scanChar();
};
}