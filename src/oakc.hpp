/**
 * @file oakc.hpp
 * @brief Defines the OakCompiler class
 */

#pragma once

static_assert(__cplusplus >= 2020'00ULL);

#include "lexer.hpp"
#include "macro.hpp"
#include "parser.hpp"
#include "rule.hpp"
#include "settings.hpp"
#include <filesystem>
#include <iostream>
#include <list>
#include <string>

const static std::string ACORN_VERSION = "0.8.0";

/**
 * @class OakCompiler
 * @brief Preprocessor and management. This is NOT a parser, but
 * handles entry points and whatnot.
 */
class OakCompiler {
public:
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

  /// Install some package globally
  /// To be called from the command line, so IO is acceptable
  void install_package(const std::string &_name);

  /// Remove some globally-install package
  void uninstall_package(const std::string &_name);

  /// Create a new template package with the given name
  void new_package(const std::string &_name);

  //////////////////////////////////////////////////////////////

  /// The settings to run after argument parsing
  Settings settings;

  /**
   * @brief Compile according to settings
   */
  void operator()();

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
  void do_file(const std::filesystem::path &_path,
               Settings &_settings);

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
   * @brief Preprocess until a fixed point is reached
   */
  uint64_t preprocess(std::list<Lexer::Token> &_token_stream,
                      Settings::CompileSettings &_csettings);

  /**
   * @brief Load a given dialect file and apply it
   */
  void load_dialect_file(const std::filesystem::path &_file);

  /**
   * @brief Write the parsed information to the given stream in
   * C format.
   */
  void translate(std::ostream &_into) const;
};
