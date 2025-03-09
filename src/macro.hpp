/**
 * @file
 * @brief Resources for managing macros
 * @author Jordan Dehmel
 */

#pragma once

#include "lexer.hpp"
#include <filesystem>
#include <map>
#include <optional>
#include <set>
#include <variant>

/**
 * @class MacroManager
 * @brief Manages the registration and substitution of macros
 */
class MacroManager {
public:
  /// Internal oak macros which are deferred to parse time
  /// (EG size!, type!)
  const static std::set<std::string> reserved_macro_names;

  /**
   * @brief Replace the given region according to known macro
   * rules
   * @param _whole The iterand itself
   * @param _it The current position in the iterand
   * @param _end The end of the iterand
   */
  void
  replace(std::list<Lexer::Token> &_whole,
          std::list<Lexer::Token>::iterator &_it,
          const std::list<Lexer::Token>::iterator &_end) const;

  /// Erases AND STRIPS QUOTES OFF OF a macro occurrence's
  /// args. Then returns those args.
  static std::list<Lexer::Token>
  get_macro_args(std::list<Lexer::Token> &_whole,
                 std::list<Lexer::Token>::iterator &_it,
                 const std::list<Lexer::Token>::iterator &_end);

  /// STRIPS QUOTES OFF OF a macro occurrence's
  /// args. Then returns those args WITHOUT ERASURE.
  static std::list<Lexer::Token> get_macro_args(
      const std::list<Lexer::Token>::const_iterator &_beg,
      const std::list<Lexer::Token>::const_iterator &_end);

  /**
   * @brief Strips string literal delinators off a string
   * literal. For example: "fizz" -> fizz, 'buzz' -> buzz.
   * @param _str_lit The string literal to strip
   * @returns The stripped string literal
   */
  static std::string
  strip_string_literal(const std::string &_str_lit);

  /**
   * @brief Process and remove a macro definition at the given
   * location
   * @param _whole The iterand
   * @param _it The current position in the iterand
   * @param _end The end of the iterand
   */
  void process_definition(
      std::list<Lexer::Token> &_whole,
      std::list<Lexer::Token>::iterator &_it,
      const std::list<Lexer::Token>::iterator &_end);

protected:
  /**
   * @struct MacroManager::Alias
   * @brief Holds information pertaining to inline/alias macros
   * (EG LINE!, FILE!, etc)
   */
  struct Alias {
    /// The thing the macro should be replaced with
    std::list<Lexer::Token> contents;
  };

  /**
   * @struct MacroManager::Compiled
   * @brief Holds information for compiled (EG assert!(...))
   * macros
   */
  struct Compiled {
    /// The path to the compiled macro
    std::filesystem::path executable;
  };

  /// Maps macro names to their data
  std::map<std::string, std::variant<Alias, Compiled>> macros;
};
