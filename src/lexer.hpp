#pragma once

#include <cstdint>
#include <string>

struct Token {
  std::string text, file, type;
  uint64_t line, col;
};

/**
 * @brief Takes a block of text and yields a token stream
 */
class Lexer {
public:
protected:
};
