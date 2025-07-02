/**
 * @file
 * @brief Defines the Lexer class, which breaks strings into
 * token streams
 */

#pragma once

#include "../std/token.h"
#include "type.hpp"
#include <cstdint>
#include <filesystem>
#include <list>
#include <optional>
#include <set>
#include <string>

/**
 * @brief Takes a block of text and yields a token stream
 */
class Lexer {
public:
  /// Statics for lexing

  /// Whitespace characters for lexing
  const static std::set<char> whitespace;

  /// Conjoinable operator characters for lexing
  const static std::set<char> operators;

  /// Operator characters which are always alone (or at least
  /// have to be handled by an addition pass, e.g. >>) for
  /// lexing
  const static std::set<char> singleton_operators;

  /**
   * @struct Lexer::Token
   * @brief A single token in a token stream
   */
  struct Token {
    /// Returns the Oak interfacial version
    inline ::TokenList as_oak() {
      ::TokenList out;

      // Deep copies
      Copy_FN_PTR_String_JOIN_ARR_i8_MAPS_PTR_String(
          &out.text, (i8 *)text.c_str());
      Copy_FN_PTR_String_JOIN_ARR_i8_MAPS_PTR_String(
          &out.type, (i8 *)type.c_str());
      Copy_FN_PTR_String_JOIN_ARR_i8_MAPS_PTR_String(
          &out.file, (i8 *)file.c_str());

      out.line = line;
      out.col = col;

      return out;
    }

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
    Token(const Lexer::Token &_other,
          const std::string &_new_text)
        : text(_new_text), file(_other.file), line(_other.line),
          col(_other.col) {
    }

    /// Construct with the other as a 'template', but with some
    /// new text
    Token(const Lexer::Token &_other,
          const Lexer::Token &_new_text)
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
   * without merging or removing comments. Still does type
   * classification.
   */
  std::list<Lexer::Token>
  raw_lex(const std::string &_text,
          const std::filesystem::path &_path, uint64_t &_line,
          uint64_t &_col);

  /**
   * @brief Breaks some input file text
   * into a token stream
   */
  class TokenStream lex(const std::string &_text,
                        const std::filesystem::path &_path,
                        uint64_t &_line, uint64_t &_col);

  /**
   * @brief Gets the type of a given literal, given that it is
   * one. If not, returns nothing and does not modify _t. If it
   * is, the literal will be adjusted into C form (EG u64 ->
   * UL).
   * @param _t The possible literal to examine.
   */
  static std::optional<Type> get_literal_type(Lexer::Token &_t);

  /**
   * @brief Set `_t.type` according to its contents. If it
   * already has a type, overwrites it.
   * @param _t The type to reclassify
   */
  static void classify_type(Lexer::Token &_t);

  /// Transmute a series of strings to tokens
  static TokenStream tokify(const std::list<std::string> &_what,
                            const std::filesystem::path &_where,
                            const uint64_t &_line,
                            const uint64_t &_col);
};

/**
 * @brief Error handling wrapper for iterating over token
 * streams as produced by a Lexer
 */
class TokenStream {
private:
  std::list<Lexer::Token> raw_stream;
  std::list<Lexer::Token>::iterator cur_pos;

  friend class Lexer;

public:
  /// @param _binding The list to bind to
  TokenStream(const std::list<Lexer::Token> &_binding)
      : raw_stream(_binding), cur_pos(raw_stream.begin()) {
  }

  /// Go to the next token, never advancing past the end
  void next() noexcept;

  /// Get the token _n ahead (.cur() is 0)
  Lexer::Token peek(const uint &_n) const noexcept;

  /// Go to the previous token, never advancing past the
  /// beginning
  void prev() noexcept;

  /// @returns true iff we are on EOF
  bool done() const noexcept;

  /// @returns true iff we cannot call prev
  bool at_beg() const noexcept;

  /// @returns The front as a token
  inline operator Lexer::Token() const noexcept {
    return cur();
  }

  /// Reset to the top
  inline void reset() noexcept {
    cur_pos = raw_stream.begin();
  }

  /**
   * @brief Get the current token, returning EOF if we are
   * beyond the end.
   * @returns The token or EOF
   */
  const Lexer::Token cur() const noexcept;

  /// Low-level position access
  std::list<Lexer::Token>::iterator tell() noexcept;

  /// Low-level position control
  void seek(
      const std::list<Lexer::Token>::iterator &_where) noexcept;

  /// Erases [_begin, _end)
  void erase(const std::list<Lexer::Token>::iterator &_begin,
             const std::list<Lexer::Token>::iterator &_end);

  /// Erases _end
  std::list<Lexer::Token>::iterator
  erase(const std::list<Lexer::Token>::iterator &_end);

  /// Inserts the given stream BEFORE _end
  void insert(const std::list<Lexer::Token>::iterator &_end,
              const TokenStream &_what);

  /// Inserts the given token BEFORE _end
  void insert(const std::list<Lexer::Token>::iterator &_end,
              const Lexer::Token &_what);

  /// Replace some range with some other token stream
  /// Erases [_begin, _end)
  void replace(const std::list<Lexer::Token>::iterator &_begin,
               const std::list<Lexer::Token>::iterator &_end,
               const TokenStream &_with);

  /// Begin iteration
  inline std::list<Lexer::Token>::iterator begin() {
    return raw_stream.begin();
  }

  /// End iteration
  inline std::list<Lexer::Token>::iterator end() {
    return raw_stream.end();
  }
};
