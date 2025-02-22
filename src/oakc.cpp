#include "oakc.hpp"
#include "debug.hpp"
#include "lexer.hpp"
#include "macro.hpp"
#include "package.hpp"
#include "parser.hpp"
#include "rule.hpp"
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <stdexcept>
#include <utility>

/// Print the version of Acorn
void OakCompiler::print_version() noexcept {
  std::cout << "Acorn version " << ACORN_VERSION << "\n"
            << "MIT Licensed\n";
}

/// Print the help text for Acorn
void OakCompiler::print_help_text() noexcept {
  // clang-format off
  std::cout
      << "Acorn\n"
         "Translator for the Oak programming language\n"
         "github.com/jorbDehmel/oak\n"
         "\n"
         "-------------------------------------------------------------\n"
         "\n"
         "1. Compilation mode"
         "\n"
         "Compilation mode is the default Acorn mode. Any non-flags\n"
         "will replace the entry point (an Oak file). Acorn takes in\n"
         "exactly one entry point and produces exactly one target: This\n"
         "target may be a C file (translation mode), an object file\n"
         "(translate and call gcc), or an executable file (produce an\n"
         "object, then link via g++). It can also optionally run the\n"
         "produced executable.\n"
         "\n"
         "1.1. List of all flags\n"
         "\n"
         "    | Verbose     |Arg| Description\n"
         "----|-------------|---|-------------------------------------\n"
         " -a |             |   | Update Acorn\n"
         " -A |             |   | Uninstall Acorn\n"
         " -c | --compile   |   | Translate and compile to object file\n"
         " -C | --cd        | 1 | Change to the given directory\n"
         " -d | --debug     |   | Toggle debug mode (default off)\n"
         " -D | --dialect   | 1 | Use some dialect file\n"
         " -e | --clean     |   | Clean all local build files\n"
         " -E | --execute   |   | Translate, compile, link, and execute\n"
         " -g | --exe_debug |   | Use debugging flag at compile-time\n"
         " -h | --help      |   | Print help (this)\n"
         " -i | --install   | 1 | Install some package\n"
         " -l | --link      |   | Translate, compile, and link\n"
         " -n | --no_save   |   | Produce nothing, just do syntax check\n"
         " -o | --output    | 1 | Set output file\n"
         " -O | --optimize  |   | Use optimization flag at compile-time\n"
         " -p | --prettify  |   | Use clang-format on the produced C\n"
         " -q | --quit      |   | Quit without error immediately\n"
         " -Q | --query     | 1 | Query some installed package\n"
         " -r | --reinstall | 1 | Reinstall some package\n"
         " -R | --remove    | 1 | Remove a package\n"
         " -s | --size      |   | Show Acorn/Oak disk usage\n"
         " -S | --install   | 1 | Install some package\n"
         " -t | --translate |   | Translate to C\n"
         " -T | --test      | * | Toggle testing mode\n"
         " -u | --dump      |   | Save dump files\n"
         " -U |             |   | Save rule log files\n"
         " -v | --version   |   | Show version and halt\n"
         " -w | --new       |   | Create a new package\n"
         " -x | --syntax    |   | Toggle syntax checks (default on)\n"
         "\n"
         "1.2. Compilation Examples\n"
         "\n"
         "Compile 'a.oak' to 'a.out':\n"
         "\t`acorn a.oak -o a.out`\n"
         "Link 'foo.oak' to 'fizz.o' with debugging and optimization:\n"
         "\t`acorn -o fizz.o foo.oak -g -O`\n"
         "\tOR `acorn -gOo fizz.o foo.oak`\n"
         "Translate 'foo.oak' to 'foo.c' without syntax checking:\n"
         "\t`acorn --translate --output foo.c foo.oak --syntax`\n"
         "\tOR `acorn -t -o foo.c foo.oak -x`\n"
         "\tOR `acorn -tox foo.c foo.oak`\n"
         "Compile and execute 'a.oak' using dialect './foo.oakd':\n"
         "\t`acorn -E -D foo.oakd -o a.oak`"
         "\tOR `acorn --execute --dialect foo.oakd --output a.oak`"
         "\tOR `acorn -DoE foo.oakd a.oak`"
         "\n"
         "-------------------------------------------------------------\n"
         "\n"
         "2. Testing mode\n"
         "\n"
         "2.1. Overview\n"
         "\n"
         "Testing mode will run 'test suites', which are folders named\n"
         "'tests' which contain zero or more '.oak' files. By default,\n"
         "the cwd will be searched non-recursively for test suites.\n"
         "However, if any non-flag arguments are provided in testing\n"
         "mode, they will be treated as additional paths to search.\n"
         "If a second test flag ('-T' or '--test') is provided after\n"
         "the first, the compiler will halt after the first failure,\n"
         "rather than proceeding to every test. The default testing\n"
         "behaviour is compile-only, but if the execution flag ('-E'\n"
         "or '--execute') is provided the compiled tests will also be\n"
         "executed.\n"
         "\n"
         "2.2. Testing mode examples\n"
         "\n"
         "Compile, but do not run, the local test suite './tests/':\n"
         "\t`acorn -T`\n"
         "Compile and run the local test suite and './fizz/tests/':\n"
         "\t`acorn -TE fizz\n"
         "Compile and run the local, 'fizz', 'buzz', and 'foo' suites,\n"
         "halting after the first failure:\n"
         "\t`acorn -TTE fizz foo buzz`\n"
         "\tOR `acorn --test --test --execute fizz foo buzz`\n"
         "\tNOT `acorn fizz --test --test --execute foo buzz`\n"
         "\t(suites must come AFTER entering test mode)\n"
         "\n"
         "-------------------------------------------------------------\n"
         "\n"
         "3. Version and License\n"
         "\n";
  // clang-format on
  print_version();
}

/// Print the total disk usage of Acorn
void OakCompiler::print_size() noexcept {
  const static std::list<std::filesystem::path> files_to_check =
      {"/usr/bin/acorn", "/usr/bin/acorn-debug",
       "/usr/bin/oak2c", "/usr/bin/oak2c-debug",
       "/usr/include/oak"};

  /// Global used by the recursive function to ensure no
  /// double-counting
  std::set<std::filesystem::path> visited;

  /// Recursive file/dir size getter
  const static std::function<uintmax_t(
      const std::filesystem::path &)>
      size_in_bytes =
          [&](const std::filesystem::path &_what) -> uintmax_t {
    if (visited.contains(_what)) {
      return 0;
    }
    visited.insert(_what);

    uintmax_t total = 0;
    if (std::filesystem::is_regular_file(_what)) {
      total += std::filesystem::file_size(_what);
    } else {
      for (const auto &item :
           std::filesystem::directory_iterator(_what)) {

        total += size_in_bytes(item);
      }
    }
    return total;
  };

  /// Formats and prints a byte size
  const static auto print_bytes = [](const uintmax_t &_bytes) {
    const static uintmax_t KiB = 1024;
    const static uintmax_t MiB = KiB * KiB;
    const static uintmax_t GiB = KiB * MiB;
    const static uintmax_t TiB = KiB * GiB;
    const static uintmax_t PiB = KiB * TiB;

    if (_bytes <= MiB) {
      std::cout << (double)_bytes / KiB << " KiB\n";
    } else if (_bytes <= GiB) {
      std::cout << (double)_bytes / MiB << " MiB\n";
    } else if (_bytes <= TiB) {
      std::cout << (double)_bytes / GiB << " GiB\n";
    } else if (_bytes <= PiB) {
      std::cout << (double)_bytes / TiB << " TiB\n";
    } else {
      std::cout << (double)_bytes / PiB << " PiB\n";
    }
  };

  uintmax_t total_size = 0;
  for (const auto &path : files_to_check) {
    std::cout << path << '\t';
    if (std::filesystem::exists(path)) {
      auto size = size_in_bytes(path);
      total_size += size;
      print_bytes(size);
    } else {
      std::cout << "DNE\n";
    }
  }

  std::cout << "\nTotal:\t";
  print_bytes(total_size);
}

/// Register some update lambda to run after this process has
/// ceased
void OakCompiler::update_acorn() noexcept {
  assert(false);
}

/// Register some uninstallation lambda to run after this
/// process has ceased
void OakCompiler::uninstall_acorn() noexcept {
  assert(false);
}

/// Purge all temporary files
void OakCompiler::clean() {
  assert(false);
}

/// Find and print the list of all viable installation
/// candidates for some set of restrictions
void OakCompiler::query_package(const std::string &_name) {
  assert(false);
}

/// Install some package globally
/// To be called from the command line, so IO is acceptable
void OakCompiler::install_package(const std::string &_name) {
  std::cerr << "WARNING: Installing local package '" << _name
            << "'. This may or may not be what you want!\n";
  PackageManager::install_package(
      _name, settings.compile_settings().include_path);
}

/// Remove some globally-install package
void OakCompiler::uninstall_package(const std::string &_name) {
  assert(false);
}

/// Create a new template package with the given name
void OakCompiler::new_package(const std::string &_name) {
  assert(false);
}

/// Compile according to settings
void OakCompiler::operator()() {
  debug_print();
  if (settings.is_compile()) {
    do_compilation();
  } else {
    do_testing();
  }
}

void OakCompiler::do_compilation() {
  debug_print();
  // Variables needed by the entire process
  auto &csettings = settings.compile_settings();
  std::filesystem::path translated_file = "N/A";
  std::filesystem::path compiled_file = "N/A";
  std::filesystem::path linked_file = "N/A";

  switch (csettings.mode) {
  case Settings::CompileSettings::TRANSLATE_ONLY:
    translated_file = csettings.target;
    break;
  case Settings::CompileSettings::TRANSLATE_AND_COMPILE:
    translated_file = csettings.entry_point.string() + ".c";
    compiled_file = csettings.target;
    break;
  default: // It doesn't matter that NOTHING is included here
    translated_file = csettings.entry_point.string() + ".c";
    compiled_file = csettings.entry_point.string() + ".c.o";
    linked_file = csettings.target;
    break;
  }

  // If requested, load dialect file
  if (settings.dialect.has_value()) {
    try {
      load_dialect_file(settings.dialect.value());
    } catch (std::runtime_error &e) {
      throw std::runtime_error(
          "Error occurred while loading dialect "
          "file '" +
          settings.dialect.value().string() + "':\n" +
          e.what());
    } catch (...) {
      throw std::runtime_error(
          "An unknown error occurred while loading dialect "
          "file '" +
          settings.dialect.value().string() + "'");
    }
  }

  try {
    do_file(csettings.entry_point, csettings);
  } catch (std::runtime_error &e) {
    throw std::runtime_error(
        "Error occurred while loading entry point " +
        csettings.entry_point.string() + ":\n" + e.what());
  } catch (...) {
    throw std::runtime_error(
        "An unknown error occurred while loading entry point " +
        csettings.entry_point.string());
  }

  // If requested, translate
  if (csettings.mode >=
      Settings::CompileSettings::TRANSLATE_ONLY) {
    // Translate Oak token stream to C directly to file
    std::ofstream target_file(translated_file);
    translate(target_file);
  }

  // If requested, call compiler
  if (csettings.mode >=
      Settings::CompileSettings::TRANSLATE_AND_COMPILE) {

    // Prepare command
    std::string command;
    bool skip = false;
    for (const auto &c : csettings.compilation_command) {
      if (skip) {
        command += c;
        skip = false;
      } else if (c == '\\') {
        skip = true;
      } else if (c == '^') {
        command += translated_file;
        for (const auto &obj : csettings.objects) {
          command += " " + obj.string();
        }
      } else if (c == '@') {
        command += compiled_file;
      } else {
        command += c;
      }
    }

    // Append flags
    for (const auto &flag : csettings.compile_flags) {
      command += " " + flag;
    }

    // Run command
    int compilation_result = system(command.c_str());
    if (compilation_result != 0) {
      throw std::runtime_error(
          "Compilation command '" + command +
          "' failed with exit code " +
          std::to_string(compilation_result));
    }
  }

  // If requested, call linker
  if (csettings.mode >=
      Settings::CompileSettings::TRANSLATE_COMPILE_AND_LINK) {

    // Prepare command
    std::string command;
    bool skip = false;
    for (const auto &c : csettings.linkage_command) {
      if (skip) {
        command += c;
        skip = false;
      } else if (c == '\\') {
        skip = true;
      } else if (c == '^') {
        command += compiled_file;
      } else if (c == '@') {
        command += linked_file;
      } else {
        command += c;
      }
    }

    // Append flags and libraries
    for (const auto &flag : csettings.link_flags) {
      command += " " + flag;
    }
    for (const auto &lib : csettings.libs) {
      command += " " + lib;
    }

    // Run command
    int linkage_result = system(command.c_str());
    if (linkage_result != 0) {
      throw std::runtime_error("Linkage command '" + command +
                               "' failed with exit code " +
                               std::to_string(linkage_result));
    }
  }

  // If requested, execute
  if (csettings.mode >=
      Settings::CompileSettings::
          TRANSLATE_COMPILE_LINK_AND_EXECUTE) {
    int execution_result = system(linked_file.c_str());
    if (execution_result != 0) {
      throw std::runtime_error(
          "Execution of file '" + linked_file.string() +
          "' failed with exit code " +
          std::to_string(execution_result));
    }
  }
}

void OakCompiler::do_testing() {
  debug_print();

  // Variables needed by entire process
  auto &tsettings = settings.test_settings();

  // Each key is a `test` dir
  // Each entry is a 4-tuple: (tried to compile, tried to run,
  // compiled successfully, ran successfully)
  std::map<std::filesystem::path,
           std::tuple<uint, uint, uint, uint>>
      dirs;
  std::list<std::filesystem::path> compile_problems,
      run_problems;

  // Collect locations to test
  if (std::filesystem::exists(std::filesystem::current_path() /
                              "tests") &&
      std::filesystem::is_directory(
          std::filesystem::current_path() / "tests")) {
    tsettings.dirs.push_back(std::filesystem::current_path());
  }

  // Run all tests
  for (const auto &dir_to_search : tsettings.dirs) {
    if (!std::filesystem::exists(dir_to_search)) {
      throw std::runtime_error("Requested testing dir " +
                               dir_to_search.string() +
                               " does not exist.");
    } else if (!std::filesystem::is_directory(dir_to_search)) {
      throw std::runtime_error(
          "Requested testing dir " + dir_to_search.string() +
          " exists, but is not a directory.");
    }

    const auto test_path = dir_to_search / "tests";

    if (!std::filesystem::exists(test_path)) {
      throw std::runtime_error("Requested test dir " +
                               test_path.string() +
                               " does not exist.");
    } else if (!std::filesystem::is_directory(test_path)) {
      throw std::runtime_error(
          "Requested test dir " + test_path.string() +
          " exists, but is not a directory.");
    }

    // Compile test files, possibly running
    for (const auto &test_file :
         std::filesystem::directory_iterator(test_path)) {

      if (std::filesystem::is_regular_file(test_file) &&
          test_file.path().string().ends_with(".oak")) {

        OakCompiler comp;
        comp.settings = settings;

        const std::filesystem::path target =
            test_file.path().string() + ".out";

        comp.settings.compile_settings().entry_point =
            test_file;
        comp.settings.compile_settings().target = target;

        try {
          comp();

          int res = system(target.root_path().c_str());
          if (res != 0) {
            run_problems.push_back(test_file);
          }
        } catch (...) {
          compile_problems.push_back(test_file);
          continue;
        }
      }
    }
  }

  // Output information
  if (dirs.empty()) {
    throw std::runtime_error(
        "Failed to run any test directories!\n");
  }
  uint total_compiles_tried = 0;
  uint total_compiles_succeeded = 0;
  uint total_runs_tried = 0;
  uint total_runs_succeeded = 0;

  for (const auto &p : dirs) {
    const auto path = p.first;
    const auto tried_comp = std::get<0>(p.second);
    const auto tried_run = std::get<1>(p.second);
    const auto did_comp = std::get<2>(p.second);
    const auto did_run = std::get<3>(p.second);

    std::cout << path << ":\n"
              << "\tCompiled " << did_comp << "/" << tried_comp
              << " (" << 100.0 * (double)did_comp / tried_comp
              << "%)\n"
              << "\tRan      " << did_run << "/" << tried_run
              << " (" << 100.0 * (double)did_run / tried_run
              << "%)\n";

    total_compiles_tried += tried_comp;
    total_runs_tried += tried_run;
    total_compiles_succeeded += did_comp;
    total_runs_succeeded += did_run;
  }

  std::cout << "Over all test directories:\n"
            << "\tCompiled " << total_compiles_succeeded << "/"
            << total_compiles_tried << " ("
            << 100.0 * (double)total_compiles_succeeded /
                   total_compiles_tried
            << "%)\n"
            << "\tRan      " << total_runs_succeeded << "/"
            << total_runs_tried << " ("
            << 100.0 * (double)total_runs_succeeded /
                   total_runs_tried
            << "%)\n";

  for (const auto &i : compile_problems) {
    std::cout << "compile-time error: " << i << "\n";
  }
  for (const auto &i : run_problems) {
    std::cout << "run-time error:     " << i << "\n";
  }

  if (!compile_problems.empty() || !run_problems.empty()) {
    throw std::runtime_error(
        "One or more errors occurred; Test failed.");
  } else {
    std::cout << "All tests passed!\n";
  }
}

/**
 * @brief
 */
void OakCompiler::syntax_check(
    const std::list<Lexer::Token> &_token_stream) const {
  debug_print();
  // All detected errors: Maps
  // positions to messages
  std::list<std::pair<std::list<Lexer::Token>::const_iterator,
                      std::string>>
      errors;

  // Scan for syntax errors here

  uint64_t paren_count = 0, square_bracket_count = 0,
           curly_bracket_count = 0;
  for (auto t = _token_stream.begin(); t != _token_stream.end();
       ++t) {
    if (*t == "{") {
      ++curly_bracket_count;
    } else if (*t == "}") {
      if (curly_bracket_count == 0) {
        errors.push_back({t, "Too many ending curly brackets"});
        break;
      }
      --curly_bracket_count;
    }

    else if (*t == "(") {
      ++paren_count;
    } else if (*t == ")") {
      if (paren_count == 0) {
        errors.push_back({t, "Too many ending parentheses"});
        break;
      }
      --paren_count;
    }

    else if (*t == "[") {
      ++square_bracket_count;
    } else if (*t == "]") {
      if (square_bracket_count == 0) {
        errors.push_back(
            {t, "Too many ending square brackets"});
        break;
      }
      --square_bracket_count;
    }
  }

  // End scanning

  // If errors were found, throw them
  if (!errors.empty()) {
    for (const auto &p : errors) {
      std::cerr << p.first->file << ":" << p.first->line << "."
                << p.first->col << "> `";

      // Calculate region
      auto begin_region = p.first,
           end_region = std::next(p.first);
      for (uint i = 0; i < 20; ++i) {
        if (begin_region == _token_stream.begin()) {
          break;
        } else {
          begin_region = std::prev(begin_region);
        }
      }
      for (uint i = 0; i < 20; ++i) {
        if (std::next(end_region) == _token_stream.end()) {
          break;
        } else {
          begin_region = std::next(begin_region);
        }
      }

      // Print region
      for (auto it = begin_region; it != end_region; ++it) {
        std::cerr << it->text;
        if (std::next(it) != end_region) {
          std::cerr << ' ';
        }
      }
      std::cerr << "`: '" << p.second << "'\n";
    }

    throw std::runtime_error(std::to_string(errors.size()) +
                             " syntax error occurred.");
  }
}

/**
 * @brief
 */
uint64_t
OakCompiler::preprocess(std::list<Lexer::Token> &_token_stream,
                        Settings::CompileSettings &_csettings) {
  debug_print();
  bool did_change = false;
  uint64_t passes = 0;

  do {
    ++passes;

    // Macro definitions
    bool saw_let = false;
    for (auto it = _token_stream.begin();
         it != _token_stream.end(); ++it) {
      if (*it == "let") {
        saw_let = true;
      } else if (saw_let && *it != "!" &&
                 it->text.find('!') != std::string::npos) {
        macros.process_definition(_token_stream, it,
                                  _token_stream.end());
        saw_let = false;
      } else {
        saw_let = false;
      }
    }

    // Resolve includes and packages
    for (auto it = _token_stream.begin();
         it != _token_stream.end(); ++it) {
      if (*it == "include!") {
        const auto args = MacroManager::get_macro_args(
            _token_stream, it, _token_stream.end());
        for (const auto &f : args) {
          std::filesystem::path p(f);
          if (std::filesystem::exists(_csettings.include_path /
                                      p)) {
            // Package file exists
            if (std::filesystem::exists(p)) {
              std::cout << "Warning: Including local file " << p
                        << " over package file of same name\n";
            } else {
              p = _csettings.include_path / p;
            }
          }
          do_file(p, _csettings);
        }
      } else if (*it == "link!") {
        const auto args = MacroManager::get_macro_args(
            _token_stream, it, _token_stream.end());
        for (const auto &f : args) {

          std::filesystem::path p(f);
          if (std::filesystem::exists(_csettings.include_path /
                                      p)) {
            // Package file exists
            if (std::filesystem::exists(p)) {
              std::cout << "Warning: Linking local object " << p
                        << " over package file of same name\n";
            } else {
              p = _csettings.include_path / p;
            }
          }
          _csettings.objects.push_back(p);
        }
      } else if (*it == "flag!") {
        const auto args = MacroManager::get_macro_args(
            _token_stream, it, _token_stream.end());
        for (const auto &f : args) {

          std::filesystem::path p(f);
          if (std::filesystem::exists(_csettings.include_path /
                                      p)) {
            // Package file exists
            if (std::filesystem::exists(p)) {
              std::cout << "Warning: Linking local object " << p
                        << " over package file of same name\n";
            } else {
              p = _csettings.include_path / p;
            }
          }
          _csettings.link_flags.push_back(p);
        }
      } else if (*it == "pragma!") {
        auto args = MacroManager::get_macro_args(
            _token_stream, it, _token_stream.end());
        if (args.size() == 1) {
          args.push_back(Lexer::Token(args.front(), ""));
        }
        _csettings.pragmas[args.front().text] =
            std::next(args.begin())->text;
      } else if (*it == "new_rule!") {
        const auto args = MacroManager::get_macro_args(
            _token_stream, it, _token_stream.end());

        Rule to_add;
        throw std::runtime_error(__FILE__);

        rules.register_rule(args.front(), to_add);
      } else if (*it == "use_rule!") {
        const auto args = MacroManager::get_macro_args(
            _token_stream, it, _token_stream.end());
        for (const auto &arg : args) {
          rules.add_entry_point(arg);
        }
      } else if (*it == "del_rule!") {
        const auto args = MacroManager::get_macro_args(
            _token_stream, it, _token_stream.end());
        for (const auto &arg : args) {
          rules.remove_entry_point(arg);
        }
      } else if (*it == "bundle!") {
        const auto args = MacroManager::get_macro_args(
            _token_stream, it, _token_stream.end());

        std::list<std::string> entails;
        for (auto it = std::next(args.begin());
             it != args.end(); ++it) {
          entails.push_back(*it);
        }

        rules.register_bundle(args.front(), entails);
      }
    }

    // Resolve macros (functional and inline)
    for (auto it = _token_stream.begin();
         it != _token_stream.end(); ++it) {
      if (*it != "!" && it->type == "ID" &&
          it->text.find('!') != std::string::npos) {
        macros.replace(_token_stream, it, _token_stream.end());
      }
    }

    // Apply ruleset
    rules.process_text(_token_stream,
                       _csettings.rule_pass_limit);
  } while (did_change &&
           passes < _csettings.preprocess_pass_limit);

  return passes;
}

/**
 * @brief
 */
void OakCompiler::load_dialect_file(
    const std::filesystem::path &_file) {
  debug_print();
  throw std::runtime_error(__FUNCTION__);
}

/**
 * @brief
 */
void OakCompiler::translate(std::ostream &_into) const {
  debug_print();
  p.reconstruct(_into);
}

/**
 * @brief Load the given file, following any includes found
 * within and doing any preprocessor rules as expected. This
 * is called by do_compilation, and should not be called
 * outside of it!
 */
void OakCompiler::do_file(
    const std::filesystem::path &_path,
    Settings::CompileSettings &_csettings) {
  debug_print();

  if (_csettings.visited.contains(_path)) {
    return;
  }
  _csettings.visited.insert(_path);

  // Load and lex
  if (!std::filesystem::exists(_path)) {
    throw std::runtime_error("File " + _path.string() +
                             " does not exist.");
  } else if (!std::filesystem::is_regular_file(_path)) {
    throw std::runtime_error(
        "File " + _path.string() +
        " exists, but is not a regular file.");
  }

  std::string text;
  std::ifstream source(_path);
  if (!source.is_open()) {
    throw std::runtime_error("Failed to open file " +
                             _path.string());
  }

  text.assign(std::istreambuf_iterator<char>(source),
              std::istreambuf_iterator<char>());
  source.close();

  Lexer l;
  uint64_t line = 1, col = 0;
  std::list<Lexer::Token> token_stream;

  try {
    token_stream = l.lex(text, _path, line, col);
  } catch (std::runtime_error &e) {
    throw std::runtime_error("Error occurred while lexing " +
                             _path.string() + ":\n" + e.what());
  } catch (...) {
    throw std::runtime_error(
        "An unknown error occurred while lexing " +
        _path.string() + "");
  }

  // If requested, syntax check
  if (_csettings.do_syntax_check) {
    syntax_check(token_stream);
  }

  // Preprocess (including includes)
  preprocess(token_stream, _csettings);

  // Do actual parsing here
  debug_print();
  p.parse_global(token_stream);
}
