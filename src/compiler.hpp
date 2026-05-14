/**
 * @file
 * @brief Defines the OakCompiler class, which manages
 * preprocessing and parsing so that it can be easily called by
 * a CLI.
 */

#pragma once

#include "parser.hpp"
#include "settings.hpp"
#include <iostream>
#include <stdexcept>
#include <string>

/// The Acorn version
const static std::string acorn_version = "0.8.0";

/**
 * @class OakCompiler
 * @brief Preprocessor and management. This is NOT a parser! It
 * dispatches to parser, runner, testing, etc. depending on the
 * settings.
 */
class OakCompiler {
public:
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
      : settings(_strm), p(settings) {
  }

  /// Print the version of Acorn
  static void print_version() noexcept;

  /// Print the help text for Acorn
  static void print_help_text() noexcept;

  /// Print the total disk usage of Oak
  static void print_size() noexcept;

  /// Register an uninstallation lambda to uninstall acorn after
  /// this process has ceased
  static void uninstall_acorn() noexcept;

  /// Purge all temporary files
  static void clean();

  /// Create a new template package with the given name
  void new_package(const std::string &_name);

  /// The settings to run after argument parsing
  Settings settings;

  /// The parser
  Parser p;

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
   * @brief Runs in testing mode
   */
  void do_testing();

  /**
   * @brief Write the parsed information to the given stream in
   * C format.
   */
  inline void translate(std::ostream &_into) {
    p.reconstruct(_into);
  }
};
