/**
 * @brief Frontend for the Acorn compiler.
 */

static_assert(__cplusplus >= 2020'00ULL);

#include "oakc.hpp"
#include <filesystem>
#include <iostream>
#include <stdexcept>

/**
 * @brief Parses CLI args into usable settings
 * @param _c The number of CLI args
 * @param _v The CLI args
 * @param _oakc
 * @returns True if the compiler should be run, false otherwise
 */
bool parse_args(const int _c, const char *const _v[],
                OakCompiler &_oakc) {
  if (_c == 1) {
    _oakc.print_help_text();
    return false;
  }

  for (int i = 1; i < _c; ++i) {
    const std::string arg = _v[i];

    if (arg.size() > 2 && arg.substr(0, 2) == "--") {
      // Translate and compile
      if (arg == "--compile") {
        _oakc.settings.compile_settings().mode = OakCompiler::
            Settings::CompileSettings::TRANSLATE_AND_COMPILE;
      }

      // CD somewhere
      else if (arg == "--cd") {
        std::filesystem::current_path(_v[++i]);
      }

      // Activate debug mode
      else if (arg == "--debug") {
        _oakc.settings.debug = !_oakc.settings.debug;
      }

      // Use dialect file
      else if (arg == "--dialect") {
        _oakc.settings.dialect = _v[++i];
      }

      // Clean
      else if (arg == "--clean") {
        _oakc.clean();
      }

      // Translate, compile, link, and execute
      else if (arg == "--execute") {
        _oakc.settings.compile_settings().mode =
            OakCompiler::Settings::CompileSettings::
                TRANSLATE_COMPILE_LINK_AND_EXECUTE;
      }

      // Add -g debug flag
      else if (arg == "--exe_debug") {
        _oakc.settings.compile_settings()
            .compile_flags.push_back("-g");
      }

      // Help
      else if (arg == "--help") {
        _oakc.print_help_text();
        return false;
      }

      // Translate, compile, and link
      else if (arg == "--link") {
        _oakc.settings.compile_settings().mode =
            OakCompiler::Settings::CompileSettings::
                TRANSLATE_COMPILE_AND_LINK;
      }

      // Only syntax checking
      else if (arg == "--no_save") {
        _oakc.settings.compile_settings().mode =
            OakCompiler::Settings::CompileSettings::NOTHING;
      }

      // Set output
      else if (arg == "--output") {
        _oakc.settings.compile_settings().target = _v[++i];
      }

      // Use -O3 optimization flag
      else if (arg == "--optimize") {
        _oakc.settings.compile_settings()
            .compile_flags.push_back("-O3");
      }

      // Use clang-format on translated files
      else if (arg == "--prettify") {
        _oakc.settings.compile_settings().prettify = true;
      }

      // Quit immediately
      else if (arg == "--quit") {
        return false;
      }

      // Query an installed package
      else if (arg == "--query") {
        _oakc.query_package(_v[++i]);
      }

      // Reinstall the given package
      else if (arg == "--reinstall") {
        _oakc.uninstall_package(_v[++i]);
        _oakc.install_package(_v[i]);
      }

      // Uninstall a package
      else if (arg == "--remove") {
        _oakc.uninstall_package(_v[++i]);
      }

      // List Oak's disk usage
      else if (arg == "--size") {
        _oakc.print_size();
        return false;
      }

      // Install package
      else if (arg == "--install") {
        _oakc.install_package(_v[++i]);
      }

      // Translate only
      else if (arg == "--translate") {
        _oakc.settings.compile_settings().mode = OakCompiler::
            Settings::CompileSettings::TRANSLATE_ONLY;
      }

      // Run test suite(s)
      else if (arg == "--test") {
        _oakc.settings.test_settings();
      }

      // Save dump file
      else if (arg == "--dump") {
        if (_oakc.settings.compile_settings()
                .dump_file.has_value()) {
          _oakc.settings.compile_settings().dump_file.reset();
        } else {
          _oakc.settings.compile_settings().dump_file =
              std::make_shared<std::ofstream>(
                  _oakc.settings.compile_settings()
                      .entry_point.string() +
                  ".acorn_dump");
        }
      }

      // Print Oak version
      else if (arg == "--version") {
        _oakc.print_version();
        return false;
      }

      // Create new package
      else if (arg == "--new") {
        _oakc.new_package(_v[++i]);
      }

      // Toggle syntax checking
      else if (arg == "--syntax") {
        _oakc.settings.compile_settings().do_syntax_check =
            !_oakc.settings.compile_settings().do_syntax_check;
      }

      else {
        throw std::runtime_error("Unknown verbose flag '" +
                                 arg + "'");
      }
    } else if (arg.size() > 1 && arg[0] == '-') {
      // Abbreviated flag
      for (uint j = 1; j < arg.size(); ++j) {
        char flag = arg[j];

        switch (flag) {
        case 'a': // Update
          _oakc.update_acorn();
          return false;
        case 'A': // Uninstall
          _oakc.uninstall_acorn();
          return false;
        case 'c': // Translate and compile to object
          _oakc.settings.compile_settings().mode = OakCompiler::
              Settings::CompileSettings::TRANSLATE_AND_COMPILE;
          break;
        case 'C': // CD somewhere
          std::filesystem::current_path(_v[++i]);
          break;
        case 'd': // Debug
          _oakc.settings.debug = !_oakc.settings.debug;
          break;
        case 'D': // Use dialect file
          _oakc.settings.dialect = _v[++i];
          break;
        case 'e': // Clean
          _oakc.clean();
          break;
        case 'E': // Translate, compile, link, and execute
          _oakc.settings.compile_settings().mode =
              OakCompiler::Settings::CompileSettings::
                  TRANSLATE_COMPILE_LINK_AND_EXECUTE;
          break;
        case 'g': // Use -g debugging flag
          _oakc.settings.compile_settings()
              .link_flags.push_back("-g");
          break;
        case 'h': // Help
          _oakc.print_help_text();
          return false;
        case 'l': // Translate, compile, and link
          _oakc.settings.compile_settings().mode =
              OakCompiler::Settings::CompileSettings::
                  TRANSLATE_COMPILE_AND_LINK;
          break;
        case 'M': // Used for macro compilation
          break;
        case 'n': // Produce nothing: Just error checking
          _oakc.settings.compile_settings().mode =
              OakCompiler::Settings::CompileSettings::NOTHING;
          break;
        case 'o': // Set output
          _oakc.settings.compile_settings().target = _v[++i];
          break;
        case 'O': // Use -O3 optimization flag
          _oakc.settings.compile_settings()
              .link_flags.push_back("-O3");
          break;
        case 'p': // Use clang-format on C
          _oakc.settings.compile_settings().prettify =
              !_oakc.settings.compile_settings().prettify;
          break;
        case 'q': // Quit immediately
          return false;
        case 'Q': // Query some installed package
          _oakc.query_package(_v[++i]);
          break;
        case 'r': // Reinstall a package
          _oakc.uninstall_package(_v[++i]);
          _oakc.install_package(_v[++i]);
          break;
        case 'R': // Uninstall a package
          _oakc.uninstall_package(_v[++i]);
          break;
        case 's': // Show Oak disk usage
          _oakc.print_size();
          return false;
        case 't': // Translate only
          _oakc.settings.compile_settings().mode = OakCompiler::
              Settings::CompileSettings::TRANSLATE_ONLY;
          break;
        case 'T': // Test
          _oakc.settings.test_settings();
          break;
        case 'u': // Save dump file
          if (_oakc.settings.compile_settings()
                  .dump_file.has_value()) {
            _oakc.settings.compile_settings().dump_file.reset();
          } else {
            _oakc.settings.compile_settings().dump_file =
                std::make_shared<std::ofstream>(
                    _oakc.settings.compile_settings()
                        .entry_point.string() +
                    ".acorn_dump");
          }
          break;
        case 'U': // Save rule log files
          _oakc.settings.compile_settings().rule_logs =
              !_oakc.settings.compile_settings().rule_logs;
          break;
        case 'v': // Show version
          _oakc.print_version();
          return false;
        case 'w': // Create a new package
          _oakc.new_package(_v[++i]);
          break;
        case 'x': // Ignore syntax errors
          _oakc.settings.compile_settings().do_syntax_check =
              !_oakc.settings.compile_settings()
                   .do_syntax_check;
          break;

        case 'i': // Install package
        case 'S':
          _oakc.install_package(_v[++i]);
          break;

        default:
          throw std::runtime_error(
              "Unknown abbreviated flag '" +
              std::to_string(flag) + "'");
          break;
        }
      }
    } else {
      // Regular arg
      if (_oakc.settings.is_compile()) {
        // Input file
        _oakc.settings.compile_settings().entry_point = arg;
      } else {
        // Dir to run tests in
        _oakc.settings.test_settings().dirs.push_back(arg);
      }
    }
  }

  return true;
}

/**
 * @brief Main function providing a frontend for the OakC system
 * @param _c The number of CLI args
 * @param _v The CLI args
 * @returns 0 on success, nonzero on failure
 */
int main(int _c, char *_v[]) {
  OakCompiler oakc;

  // Parse CLI args
  try {
    bool res = parse_args(_c, _v, oakc);
    if (!res) {
      return 0;
    }
  } catch (std::runtime_error &e) {
    std::cerr << "Argument parsing error:\n"
              << e.what() << '\n';
    return 1;
  } catch (...) {
    std::cerr
        << "An unknown argument parsing error occurred.\n";
    return 1;
  }

  // Compile with collected settings
  try {
    oakc();
  } catch (std::runtime_error &e) {
    std::cerr << "Compiler error:\n" << e.what() << '\n';
    return 2;
  } catch (...) {
    std::cerr << "An unknown compiler error occurred.\n";
    return 3;
  }

  return 0;
}
