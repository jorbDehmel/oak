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
#include <stdexcept>
#include <string>
#include <variant>

/**
 * @brief Takes a block of text and yields a token stream
 */
class Lexer {
public:
  /// Disallow normal instantation: This is a static class
  Lexer() = delete;

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
          &out.file, (i8 *)file.c_str());

      out.line = line;
      out.col = col;

      return out;
    }

    /// The literal string value
    std::string text;

    /// Where it came from
    std::filesystem::path file;

    /// The line it comes from
    uint64_t line;

    /// The column it started in
    uint64_t col;

    /// Construct with all parameters specified
    Token(const std::string &_text,
          const std::filesystem::path &_file,
          const uint64_t &_line, const uint64_t &_col)
        : text(_text), file(_file), line(_line), col(_col) {
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
        : text(_new_text), file(_other.file), line(_other.line),
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
  static std::list<Lexer::Token>
  raw_lex(const std::string &_text,
          const std::filesystem::path &_path, uint64_t &_line,
          uint64_t &_col, const bool &_is_original = false);

  /**
   * @brief Breaks some input file text
   * into a token stream
   */
  static class TokenStream
  lex(const std::string &_text,
      const std::filesystem::path &_path, uint64_t &_line,
      uint64_t &_col, const bool &_is_original = false);

  /**
   * @brief Gets the type of a given literal, given that it is
   * one. If not, returns nothing and does not modify _t. If it
   * is, the literal will be adjusted into C form (EG u64 ->
   * UL).
   * @param _t The possible literal to examine.
   */
  static std::optional<Type> get_literal_type(Lexer::Token &_t);
};

/**
 * @brief Error handling wrapper for iterating over token
 * streams as produced by a Lexer
 */
class TokenStream {
private:
  std::vector<Lexer::Token> raw_stream;
  size_t cur_pos;

  friend class Lexer;

public:
  /// @param _binding The Lexer::Token iterable to bind to
  TokenStream(const std::list<Lexer::Token> &_binding)
      : cur_pos(0) {
    for (const auto &tok : _binding) {
      raw_stream.push_back(tok);
    }
  }

  /// @param _binding The Lexer::Token iterable to bind to
  TokenStream(const TokenStream &_binding) : cur_pos(0) {
    for (const auto &tok : _binding) {
      raw_stream.push_back(tok);
    }
  }

  /// Go to the next token, never advancing past the end
  void next() noexcept;

  /// Get the token _n ahead (.cur() is 0, default is 1)
  Lexer::Token peek(const int &_n = 1) const noexcept;

  /// Go to the previous token, never advancing past the
  /// beginning
  void prev() noexcept;

  /// @returns true iff we are on EOF
  bool done() const noexcept;

  /// @returns true iff we cannot call prev
  bool at_beg() const noexcept;

  /**
   * @brief Get the current token, returning EOF if we are
   * beyond the end.
   * @returns The token or EOF
   */
  const Lexer::Token cur() const noexcept;

  /// Assert that the cur token is in the given set and advance
  inline void expect(const std::set<std::string> &_allowed) {
    if (!_allowed.contains(cur())) {
      throw std::runtime_error("Unexpected token '" +
                               cur().text + "'");
    }
    next();
  }

  /**
   * @brief
   * @returns
   */
  Lexer::Token &cur_mut();

  /// Low-level position access. Be very careful with these!
  /// They are indices into the internal array, so you won't end
  /// up on a discarded section of linked list, but you may
  /// not end up in the place you expect! If you delete or add
  /// anything before a told position it will be invalidated!
  size_t tell() noexcept;

  /// Create a COPY of the given range.
  TokenStream
  copy_snippet(const size_t &_start,
               const size_t &_first_after) const noexcept;

  /// Replaces a range with some formatted data, then moves to
  /// point to the first after. The new length will be given by
  /// tell() - _start. If an entry in _fmt is a token, that
  /// token will be inserted. Otherwise if it is a token stream,
  /// that entire range will be copied and inserted. These token
  /// streams will probably come from copy_snippet, but they
  /// don't have to. In the case of a token stream, the line and
  /// col will be overwritten with the most recent values.
  void rangef(
      const size_t &_start, const size_t &_first_after,
      const std::list<std::variant<Lexer::Token, TokenStream>>
          &_fmt);

  /// Low-level position control. Be very careful! See `tell`
  /// for more details.
  void seek(const size_t &_where) noexcept;

  /// Begin iteration
  inline std::vector<Lexer::Token>::const_iterator
  begin() const {
    return raw_stream.begin();
  }

  /// End iteration
  inline std::vector<Lexer::Token>::const_iterator end() const {
    return raw_stream.end();
  }
};
