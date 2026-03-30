/**
 * @file
 * @brief Defines settings structures for Oak compilation
 */

#pragma once

#include "type.hpp"
#include <cstdint>
#include <filesystem>
#include <list>
#include <map>
#include <optional>
#include <ostream>
#include <set>
#include <stdexcept>
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
    uint64_t preprocess_pass_limit = 0x1'00;

    /// The current return type (default void) for fn-parsing
    /// type checking. Must be an exact match.
    std::list<Type> cur_return_type = {Type({"void"})};

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

    /// Maps filepaths to their pragma mappings
    std::map<std::filesystem::path,
             std::map<std::string, std::string>>
        pragmas;

    /// Keeps track of all files processed to avoid
    /// duplication
    std::set<std::filesystem::path> visited;

    /// Holds the most recent modification of all visited files
    std::filesystem::file_time_type most_recent_mod_time;
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

    /// The testing mode: Distinguishes what constitutes an
    /// error when running test suites
    enum TestSettingsMode {
      COMPILE_IGNORE_FAILURE = 0,                        // T
      COMPILE = 1,                                       // TT
      COMPILE_IGNORE_FAILURE_EXECUTE_IGNORE_FAILURE = 2, // TE
      COMPILE_EXECUTE_IGNORE_FAILURE = 3,                // TTE
      COMPILE_IGNORE_FAILURE_EXECUTE = 4,                // TEE
      COMPILE_EXECUTE = 5,                               // TTEE
    };

    /// The current test mode: Default is T
    TestSettingsMode mode = COMPILE_IGNORE_FAILURE;

    /**
     * @brief Given some command-line flag, toggle the mode
     * @param _c The flag to process
     */
    inline void process_mode_flag(const char &_c) {
      switch (_c) {
      case 'T':
        if (mode % 2 == 0) {
          mode = static_cast<TestSettingsMode>(mode + 1);
        } else {
          mode = static_cast<TestSettingsMode>(mode - 1);
        }
        break;
      case 'E':
        mode = static_cast<TestSettingsMode>((mode + 2) % 6);
        break;
      default:
        break;
      }
    }

    /// Return whether or not tests should be executed after
    /// compilation
    inline bool should_execute() const {
      return mode > COMPILE;
    }

    /// Return whether or not testing mode should fail if a
    /// single compilation does
    inline bool fail_with_compile() const {
      return mode % 2 == 1;
    }

    /// Return whether or not testing mode should fail if a
    /// single execution does
    inline bool fail_with_execute() const {
      return mode > COMPILE_EXECUTE_IGNORE_FAILURE;
    }
  };

  /// Where to write information to (usually cout)
  std::ostream &ostream;

  /// If true, logs more to cout
  bool debug = false;

  /// If desired, the dialect file to load before running
  std::optional<std::string> dialect;

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

  /// How to handle warnings
  enum {
    NO_WARNINGS,     // Silence all warnings
    NORMAL_WARNINGS, // Print all warnings
    ERROR_WARNINGS,  // Warnings as errors
  } warning_mode = NORMAL_WARNINGS;

  /// Raise a warning according to our warning mode. The text
  /// under normal circumstances will be "Warning: _msg"
  inline void warn(const std::filesystem::path &_f,
                   const uint64_t &_line, const uint64_t &_col,
                   const std::string &_msg) const {
    switch (warning_mode) {
    case NO_WARNINGS:
      break;
    case NORMAL_WARNINGS:
      if (std::filesystem::exists(_f)) {
        ostream << _f.string();
        if (_line > 0) {
          ostream << ":" << _line;
          if (_col > 0) {
            ostream << "." << _col;
          }
        }
        ostream << "> ";
      }
      ostream << "Warning: " << _msg << '\n';
      break;
    case ERROR_WARNINGS:
      throw std::runtime_error("Warning: " + _msg +
                               " (treated as error)\n");
      break;
    }
  }

  /// If true, time whatever procedure is done
  bool do_time = false;

protected:
  /// The underlying variant of the settings
  std::variant<CompileSettings, TestSettings> internal =
      CompileSettings{};
};
