/**
 * @file
 * @brief Defines the OakCompiler class
 */

#pragma once

#include "lexer.hpp"
#include "macro.hpp"
#include "parser.hpp"
#include "rule.hpp"
#include "settings.hpp"
#include <filesystem>
#include <iostream>
#include <list>
#include <stdexcept>
#include <string>

const static std::string ACORN_VERSION = "1.0.0";

/**
 * @class OakCompiler
 * @brief Preprocessor and management. This is NOT a parser, but
 * handles entry points and whatnot.
 */
class OakCompiler {
public:
  /**
   * @brief Given a requested path, return the actual path to
   * (possibly) visit
   * @param _requested The raw path: EG "std/io.oak"
   * @param _cur_file The file which is requesting to resolve
   * the path. This is where all local paths will be from
   * @returns The canonical (fully qualified and standardized)
   * path to visit: Might be local, might be global.
   */
  std::filesystem::path
  resolve_path(const std::string &_requested,
               const std::filesystem::path &_cur_file);

  /**
   * @class RunError
   * @brief Thrown when we try and fail to run. If the caught
   * exception is a std::runtime_error and not this, it can be
   * assumed to be a compile-time error.
   */
  class RunError : public std::runtime_error {
  public:
    /// Initialize given some message and (presumably nonzero)
    /// runtime exit code
    RunError(const std::string &_msg, const int &_exit_code)
        : std::runtime_error(_msg), exit_code(_exit_code) {
    }

    /// The exit code that caused the issues
    const int exit_code;
  };

  /**
   * @brief Initialize a compiler instance
   * @param _strm The stream to use as cout
   */
  OakCompiler(std::ostream &_strm = std::cout)
      : settings(_strm) {
  }

  /// The parser
  Parser p;

  /// Rule definitions
  RuleRunner rules;

  /// Macro definitions
  MacroManager macros;

  //////////////////////////////////////////////////////////////

  /// Print the version of Acorn
  static void print_version() noexcept;

  /// Print the help text for Acorn
  static void print_help_text() noexcept;

  /// Print the total disk usage of Oak
  static void print_size() noexcept;

  /// Register some uninstallation lambda to run after this
  /// process has ceased
  static void uninstall_acorn() noexcept;

  /// Purge all temporary files
  static void clean();

  /// Create a new template package with the given name
  void new_package(const std::string &_name);

  //////////////////////////////////////////////////////////////

  /// The settings to run after argument parsing
  Settings settings;

  /**
   * @brief Compile according to settings
   */
  void operator()();

  /**
   * @brief Preprocess until a fixed point is reached
   */
  uint64_t preprocess(std::list<Lexer::Token> &_token_stream);

protected:
  /**
   * @brief Do an entire translation unit according to the
   * loaded settings. This is NOT the same as doing the entry
   * point as a file!
   */
  void do_compilation();

  /**
   * @brief Parse and turn all math into operator calls
   */
  void fix_math(std::list<Lexer::Token> &_token_stream);

  /**
   * @brief Load the given file, following any includes found
   * within and doing any preprocessor rules as expected. This
   * is called by do_compilation, and should not be called
   * outside of it!
   */
  void do_file(const std::string &_path,
               const std::filesystem::path &_cur_file);

  /**
   * @brief Runs in testing mode
   */
  void do_testing();

  /**
   * @brief Tests the input file contents for validity
   */
  void syntax_check(const std::filesystem::path &_fp,
                    const std::string &_text) const;

  /**
   * @brief Load a given dialect file and apply it
   */
  void load_dialect_file(const std::filesystem::path &_file);

  /**
   * @brief Write the parsed information to the given stream in
   * C format.
   */
  void translate(std::ostream &_into);
};
