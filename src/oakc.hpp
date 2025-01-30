/**
 * @file oakc.hpp
 * @brief
 */

#pragma once

static_assert(__cplusplus >= 2020'00ULL);

#include "lexer.hpp"
#include "package.hpp"
#include "scope.hpp"
#include <filesystem>
#include <fstream>
#include <list>
#include <memory>
#include <optional>
#include <string>
#include <variant>

const static std::string ACORN_VERSION = "0.8.0";

/**
 * @class OakCompiler
 * @brief Preprocessor and management. This is NOT a parser, but
 * handles entry points and whatnot.
 */
class OakCompiler {
public:
  ///
  class Settings {
  public:
    /**
     * @struct CompileSettings
     * @brief
     */
    struct CompileSettings {
      ///
      std::filesystem::path target = "a.out";

      ///
      std::filesystem::path entry_point = "main.oak";

      ///
      uint preprocess_pass_limit = 0x10'00;

      ///
      std::string compilation_command = "gcc ^ -o @";

      ///
      std::string linkage_command = "g++ ^ -o @";

      ///
      enum {
        NOTHING = 0,
        TRANSLATE_ONLY = 1,
        TRANSLATE_AND_COMPILE = 2,
        TRANSLATE_COMPILE_AND_LINK = 3,
        TRANSLATE_COMPILE_LINK_AND_EXECUTE = 4,
      } mode = TRANSLATE_COMPILE_AND_LINK;

      ///
      std::optional<std::shared_ptr<std::ofstream>> dump_file;

      ///
      bool prettify = false;

      ///
      bool do_syntax_check = true;

      ///
      bool rule_logs = false;

      ///
      std::list<std::string> compile_flags;

      ///
      std::list<std::string> link_flags;

      ///
      std::list<std::filesystem::path> objects;

      ///
      std::list<std::string> libs;
    };

    /**
     * @struct TestSettings
     * @brief
     */
    struct TestSettings {
      ///
      std::list<std::filesystem::path> dirs;

      ///
      bool halt_on_compiler_failure = true;

      ///
      enum {
        COMPILE_ONLY,
        REGULAR_EXECUTE,
        EXECUTE_IGNORE_FAILURE,
      } mode = REGULAR_EXECUTE;
    };

    ///
    bool debug = false;

    ///
    std::optional<std::filesystem::path> dialect;

    /**
     * @brief
     */
    inline TestSettings &test_settings() noexcept {
      if (!std::holds_alternative<TestSettings>(internal)) {
        internal = TestSettings{};
      }
      return std::get<TestSettings>(internal);
    }

    /**
     * @brief
     */
    inline CompileSettings &compile_settings() noexcept {
      if (!std::holds_alternative<CompileSettings>(internal)) {
        internal = CompileSettings{};
      }
      return std::get<CompileSettings>(internal);
    }

    /**
     * @brief
     */
    inline bool is_compile() const noexcept {
      return std::holds_alternative<CompileSettings>(internal);
    }

  protected:
    ///
    std::variant<CompileSettings, TestSettings> internal =
        CompileSettings{};
  };

  //////////////////////////////////////////////////////////////

  /// Print the version of Acorn
  static void print_version() noexcept;

  /// Print the help text for Acorn
  static void print_help_text() noexcept;

  /// Print the total disk usage of Oak
  static void print_size() noexcept;

  /// Register some update lambda to run after this process has
  /// ceased
  static void update_acorn() noexcept;

  /// Register some uninstallation lambda to run after this
  /// process has ceased
  static void uninstall_acorn() noexcept;

  /// Purge all temporary files
  static void clean();

  /// Find and print the list of all viable installation
  /// candidates for some set of restrictions
  void query_package(const std::string &_name);

  /// Install some package globally
  /// To be called from the command line, so IO is acceptable
  void install_package(const std::string &_name);

  /// Remove some globally-install package
  void uninstall_package(const std::string &_name);

  /// Create a new template package with the given name
  void new_package(const std::string &_name);

  //////////////////////////////////////////////////////////////

  ///
  PackageManager package_manager;

  ///
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
   * @brief Load the given file, following any includes found
   * within and doing any preprocessor rules as expected. This
   * is called by do_compilation, and should not be called
   * outside of it!
   */
  TranslationUnit
  do_file(const std::filesystem::path &_path,
          const Settings::CompileSettings &_csettings) const;

  /**
   * @brief
   */
  void do_testing();

  /**
   * @brief
   */
  void syntax_check(
      const std::list<Lexer::Token> &_token_stream) const;

  /**
   * @brief
   */
  bool preprocess(std::list<Lexer::Token> &_token_stream) const;

  /**
   * @brief
   */
  void load_dialect_file(const std::filesystem::path &_file);

  /**
   * @brief Write the parsed information to the given stream in
   * C format.
   */
  void translate(const TranslationUnit &_unit,
                 std::ostream &_into) const;
};
