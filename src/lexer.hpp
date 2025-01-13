/**
 * @file lexer.hpp
 * @brief
 */

#pragma once

static_assert(__cplusplus >= 2020'00ULL);

#include <cstdint>
#include <filesystem>
#include <list>
#include <string>

/**
 * @brief Takes a block of text and yields a token stream
 */
class Lexer {
public:
  struct Token {
    std::string text;
    std::string type = "ID";
    std::filesystem::path file;
    uint64_t line, col;

    Token(const std::string &_text,
          const std::filesystem::path &_file,
          const uint64_t &_line, const uint64_t &_col)
        : text(_text), file(_file), line(_line), col(_col) {}
    Token(const Token &_other, const std::string &_new_text)
        : text(_new_text), file(_other.file), line(_other.line),
          col(_other.col) {}

    inline bool operator==(const std::string &_o) const {
      return text == _o;
    }
    inline bool operator!=(const std::string &_o) const {
      return text != _o;
    }
    inline operator std::string() const { return text; }
  };

  /**
   * @brief
   */
  std::list<Token> lex(const std::string &_text,
                       const std::filesystem::path &_path,
                       uint64_t &_line, uint64_t &_col);

protected:
};
