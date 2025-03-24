/**
 * @file
 * @brief Defines settings structures for Oak compilation
 */

#pragma once

#include <filesystem>
#include <list>
#include <map>
#include <optional>
#include <ostream>
#include <set>
#include <variant>

/**
 * @class Settings
 * @brief Describes settings (either testing or compilation)
 * for an acorn CLI run.
 */
class Settings {
public:
  /// Construct with the given stream as cout
  Settings(std::ostream &_strm) : ostream(_strm) {
  }

  /**
   * @struct Settings::CompileSettings
   * @brief The compilation option of the settings enum. This
   * holds settings for any non-testing run of the CLI.
   */
  struct CompileSettings {
    /// The target file: Usually `.c`, `.o`, or `.out`
    std::filesystem::path target = "a.out";

    /// The entry point: Only main functions from this file
    /// will be kept at link-time
    std::filesystem::path entry_point = "main.oak";

    /// The system path where all oak packages are stored
    std::filesystem::path include_path = "/usr/include/oak";

    /// The max number of preprocessor passes to apply before
    /// erroring
    uint preprocess_pass_limit = 0x10'00;

    /// If true, allows unmonitored use of the
    /// compile_time::system! macro
    bool no_confirm = false;

    /// The command to call for compilation. '^' is replaced
    /// by the input file(s) and '@' is replaced by the
    /// target.
    std::string compilation_command = "gcc ^ -c -o @";

    /// The command to call for linkage. Follows same
    /// substitution rules as compilation.
    std::string linkage_command = "g++ ^ -o @";

    /// The mode of the run. Can be NOTHING (syntax check
    /// only) or specify some other options.
    enum {
      NOTHING = 0,
      TRANSLATE_ONLY = 1,
      TRANSLATE_AND_COMPILE = 2,
      TRANSLATE_COMPILE_AND_LINK = 3,
      TRANSLATE_COMPILE_LINK_AND_EXECUTE = 4,
    } mode = TRANSLATE_COMPILE_AND_LINK;

    /// If requested, where to write logs ('dumps')
    std::optional<std::shared_ptr<std::ofstream>> dump_file;

    /// If true, calls an autoformatter (clang-format) on the
    /// produced C files
    bool prettify = false;

    /// If true, enforces compliance to best practices. This
    /// is not always desirable (EG macros)
    bool do_syntax_check = true;

    /// If true, write logs of all rule operations
    bool rule_logs = false;

    /// A list of flags to put AFTER the compilation command
    std::list<std::string> compile_flags;

    /// A list of flags to put AFTER the linkage command
    std::list<std::string> link_flags;

    /// A list of object files ('.o') to include at link time
    std::list<std::filesystem::path> objects;

    /// Libraries requested for inclusion via g++ at link time
    std::list<std::string> libs;

    /// Maps filepaths to their pragma mappings
    std::map<std::filesystem::path,
             std::map<std::string, std::string>>
        pragmas;

    /// Keeps track of all files processed to avoid
    /// duplication
    std::set<std::filesystem::path> visited;
  };

  /**
   * @struct TestSettings
   * @brief The testing option of the settings variant. This
   * contains the settings for when acorn is running in test
   * mode (EG test suites)
   */
  struct TestSettings {
    /// A list of directories to look for tests in
    std::list<std::filesystem::path> dirs;

    /// If true, compilation errors cause us to stop in our
    /// tracks
    bool halt_on_compiler_failure = true;

    /// The testing mode: Allows compilation only or two modes
    /// of compilation+running
    enum {
      COMPILE_ONLY,           // T
      REGULAR_EXECUTE,        // TTE
      EXECUTE_IGNORE_FAILURE, // TE
    } mode = REGULAR_EXECUTE;
  };

  /// Where to write information to (usually cout)
  std::ostream &ostream;

  /// If true, logs more to cout
  bool debug = false;

  /// If desired, the dialect file to load before running
  std::optional<std::filesystem::path> dialect;

  /**
   * @brief Yields this object as a TestSettings variant
   */
  inline TestSettings &test_settings() noexcept {
    if (!std::holds_alternative<TestSettings>(internal)) {
      internal = TestSettings{};
    }
    return std::get<TestSettings>(internal);
  }

  /**
   * @brief Yields this object as a CompileSettings variant
   */
  inline CompileSettings &compile_settings() noexcept {
    if (!std::holds_alternative<CompileSettings>(internal)) {
      internal = CompileSettings{};
    }
    return std::get<CompileSettings>(internal);
  }

  /**
   * @brief Returns true iff we are in compile mode
   */
  inline bool is_compile() const noexcept {
    return std::holds_alternative<CompileSettings>(internal);
  }

protected:
  /// The underlying variant of the settings
  std::variant<CompileSettings, TestSettings> internal =
      CompileSettings{};
};
