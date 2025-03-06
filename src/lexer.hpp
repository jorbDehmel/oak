/**
 * @file lexer.hpp
 * @brief Defines the Lexer class, which strings into token
 * streams
 */

#pragma once

static_assert(__cplusplus >= 2020'00ULL);

#include "type.hpp"
#include <cstdint>
#include <filesystem>
#include <list>
#include <optional>
#include <string>

/**
 * @brief Takes a block of text and yields a token stream
 */
class Lexer {
public:
  /**
   * @struct Lexer::Token
   * @brief A single token in a token stream
   */
  struct Token {
    /// The literal string value
    std::string text;

    /// The category of symbol
    std::string type = "ID";

    /// Where it came from
    std::filesystem::path file;

    /// The line it comes from
    uint64_t line;

    /// The column it started in
    uint64_t col;

    /// Construct with all parameters specified
    Token(const std::string &_text,
          const std::filesystem::path &_file,
          const uint64_t &_line, const uint64_t &_col,
          const std::string &_type = "ID")
        : text(_text), type(_type), file(_file), line(_line),
          col(_col) {
    }

    /// Construct with the other as a 'template', but with some
    /// new text
    Token(const Token &_other, const std::string &_new_text)
        : text(_new_text), file(_other.file), line(_other.line),
          col(_other.col) {
    }

    /// Construct with the other as a 'template', but with some
    /// new text
    Token(const Token &_other, const Lexer::Token &_new_text)
        : text(_new_text), type(_new_text.type),
          file(_other.file), line(_other.line),
          col(_other.col) {
    }

    /// Returns true iff the texts match
    inline bool operator==(const std::string &_o) const {
      return text == _o;
    }

    /// Returns true iff the texts don't match
    inline bool operator!=(const std::string &_o) const {
      return text != _o;
    }

    /// Casts to a normal std::string
    inline operator std::string() const {
      return text;
    }
  };

  /**
   * @brief Breaks some input file text into a token stream
   */
  std::list<Token> lex(const std::string &_text,
                       const std::filesystem::path &_path,
                       uint64_t &_line, uint64_t &_col);

  /**
   * @brief Gets the type of a given literal, given that it is
   * one. If not, returns nothing and does not modify _t. If it
   * is, the literal will be adjusted into C form (EG u64 ->
   * UL).
   * @param _t The possible literal to examine.
   */
  static std::optional<Type> get_literal_type(Token &_t);
};
