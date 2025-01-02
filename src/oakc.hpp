/**
 * @file oakc.hpp
 * @brief
 */

#pragma once

#include <list>
#include <optional>
#include <string>
#include <variant>

/**
 * @class OakCompiler
 * @brief
 */
class OakCompiler {
public:
  /**
   * @struct CompileSettings
   * @brief
   */
  struct CompileSettings {
    ///
    std::string target = "a.out";

    ///
    std::string entry_point = "main.oak";

    ///
    enum {
      NOTHING,
      TRANSLATE_ONLY,
      TRANSLATE_AND_COMPILE,
      TRANSLATE_COMPILE_AND_LINK,
      TRANSLATE_COMPILE_LINK_AND_EXECUTE,
    } mode = TRANSLATE_COMPILE_AND_LINK;

    ///
    enum {
      NO_DOCS,
      DUMP,
      MARKDOWN,
    } doc_mode = NO_DOCS;

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
    std::list<std::string> objects;

    ///
    std::list<std::string> libs;

    ///
    std::optional<std::string> dialect_file;
  };

  /**
   * @struct TestSettings
   * @brief
   */
  struct TestSettings {
    ///
    std::list<std::string> dirs;

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
  class Settings {
  public:
    ///
    bool debug = false;

    ///
    std::optional<std::string> dialect;

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

  ///
  static void print_version() noexcept;

  ///
  static void print_help_text() noexcept;

  ///
  static void print_size() noexcept;

  ///
  static void update_acorn() noexcept;

  ///
  static void uninstall_acorn() noexcept;

  ///
  static void clean() noexcept;

  ///
  static void query_package(const std::string &_name) noexcept;

  ///
  static void
  install_package(const std::string &_name) noexcept;

  ///
  static void
  uninstall_package(const std::string &_name) noexcept;

  ///
  static void new_package(const std::string &_name) noexcept;

  //////////////////////////////////////////////////////////////

  ///
  Settings settings;

  /**
   * @brief
   */
  void operator()();

protected:
};
