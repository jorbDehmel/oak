/**
 * @file
 * @brief Resources for managing macros
 */

#pragma once

#include "lexer.hpp"
#include "settings.hpp"
#include <filesystem>
#include <map>
#include <set>
#include <stdexcept>
#include <variant>

/**
 * @brief An error class thrown when we surpass the PPP limit.
 */
class OutOfPPPLError : public std::runtime_error {
public:
  /// Initialize
  OutOfPPPLError(const std::string &_what)
      : std::runtime_error(_what) {
  }
};

/**
 * @brief Runs a command, asserts it succeeded, and captures
 * its stdout.
 * @param _cmd The command to run
 * @returns The string output of the command
 */
std::string get_cmd_output(const std::string &_cmd);

/// Forward definition to avoid loop inclusion
class OakCompiler;

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
   * @param _it The current position in the iterand
   * @param _csettings Settings for debugging
   * @param _oakc The compiler to be used for recursive
   * preprocessing
   */
  void replace(TokenStream &_pos, const Settings &_csettings,
               OakCompiler &_oakc) const;

  /// Erases and returns a macro occurrence's args.
  static std::list<std::list<Lexer::Token>>
  get_macro_args(TokenStream &_pos);

  /// STRIPS QUOTES OFF OF a macro occurrence's
  /// args. Then returns those args WITHOUT ERASURE and
  /// WITHOUT recursion! This should only be used after all
  /// preprocessing!
  static std::list<Lexer::Token>
  get_macro_args_no_erase(TokenStream &_pos);

  /**
   * @brief Strips string literal delimiters off a string
   * literal. For example: "fizz" -> fizz, 'buzz' -> buzz.
   * @param _str_lit The string literal to strip
   * @returns The stripped string literal
   */
  static std::string
  strip_string_literal(const std::string &_str_lit);

  /**
   * @brief Inverse of strip_string_literal.
   * @param _contents The contents to embed in double quotes
   * @returns The string literal
   */
  static std::string
  make_string_literal(const std::string &_contents);

  /**
   * @brief Process and remove a macro definition at the given
   * location
   * @param _preproc_passes_allowed Used to prevent infinite
   * macro recursion.
   */
  void
  process_definition(TokenStream &_pos,
                     const uint64_t &_preproc_passes_allowed);

  /**
   * @struct MacroManager::Alias
   * @brief Holds information pertaining to inline/alias
   * macros (EG LINE!, FILE!, etc)
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
