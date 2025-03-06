#include "oakc.hpp"
#include "debug.hpp"
#include "lexer.hpp"
#include "macro.hpp"
#include "package.hpp"
#include "parser.hpp"
#include "rule.hpp"
#include "settings.hpp"
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
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
         "    | Verbose      |Arg| Description\n"
         "----|--------------|---|-------------------------------------\n"
         " -A |              |   | Uninstall Acorn\n"
         " -c | --compile    |   | Translate and compile to object file\n"
         " -C | --cd         | 1 | Change to the given directory\n"
         " -d | --debug      |   | Toggle debug mode (default off)\n"
         " -D | --dialect    | 1 | Use some dialect file\n"
         " -e | --clean      |   | Clean all local build files\n"
         " -E | --execute    |   | Translate, compile, link, and execute\n"
         " -g | --exe_debug  |   | Use debugging flag at compile-time\n"
         " -h | --help       |   | Print help (this)\n"
         " -i | --install    | 1 | Install some package\n"
         " -l | --link       |   | Translate, compile, and link\n"
         " -n | --no_save    |   | Produce nothing, just do syntax check\n"
         " -o | --output     | 1 | Set output file\n"
         " -O | --optimize   |   | Use optimization flag at compile-time\n"
         " -p | --prettify   |   | Use clang-format on the produced C\n"
         " -q | --quit       |   | Quit without error immediately\n"
         " -r | --reinstall  | 1 | Reinstall some package\n"
         " -R | --remove     | 1 | Remove a package\n"
         " -s | --size       |   | Show Acorn/Oak disk usage\n"
         " -S | --install    | 1 | Install some package\n"
         " -t | --translate  |   | Translate to C\n"
         " -T | --test       | * | Toggle testing mode\n"
         " -u | --dump       |   | Save dump files\n"
         " -U |              |   | Save rule log files\n"
         " -v | --version    |   | Show version and halt\n"
         " -w | --new        |   | Create a new package\n"
         " -y | --no_confirm |   | Always allow compile_time::system!\n"
         " -x | --syntax     |   | Toggle syntax checks (default on)\n"
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

void internal_uninstall() {
  char choice = 'n';

  std::cout
      << "Are you sure you want to uninstall acorn? [y/N] ";
  std::cin >> choice;

  if (std::tolower(choice) != 'y') {
    std::cout << "Aborting.\n";
    return;
  }

  std::cout << "Should installed packages be deleted? [y/N] ";
  std::cin >> choice;

  if (std::tolower(choice) == 'y') {
    std::cout << "Removing packages...\n";
    std::filesystem::remove_all("/usr/include/oak");
  }

  for (const auto &p :
       {"/usr/bin/oak2c", "/usr/bin/oak2c-debug",
        "/usr/bin/acorn", "/usr/bin/acorn-debug"}) {
    if (std::filesystem::exists(p) &&
        !std::filesystem::is_directory(p)) {
      std::cout << "Removing " << p << "...\n";
      std::filesystem::remove(p);
    }
  }

  std::cout << "Acorn has been erased. Farewell!\n";
}

/// Register some uninstallation lambda to run after this
/// process has ceased
void OakCompiler::uninstall_acorn() noexcept {
  std::atexit(internal_uninstall);
}

/// Purge all temporary files
void OakCompiler::clean() {
  std::list<std::filesystem::path> to_erase;
  for (const auto &path :
       std::filesystem::recursive_directory_iterator(
           std::filesystem::current_path())) {
    if (!std::filesystem::is_directory(path)) {
      const std::string filename =
          path.path().filename().string();
      if (filename.find(".oak.") != std::string::npos ||
          filename == "acorn_test.log") {
        to_erase.push_back(path.path());
        std::cout << path.path() << '\n';
      }
    }
  }

  std::cout << "\nThis will erase all the above files. Are you "
               "sure? [y/N] ";
  char choice = std::cin.get();
  if (std::tolower(choice) != 'y') {
    throw std::runtime_error("clean aborted");
  }

  for (const auto &item : to_erase) {
    if (!std::filesystem::remove(item)) {
      std::cout << "Failed to remove " << item << '\n';
    }
  }
}

/// Install some package globally
/// To be called from the command line, so IO is acceptable
void OakCompiler::install_package(const std::string &_name) {
  PackageManager::install_package(_name,
                                  settings.compile_settings());
}

/// Remove some globally-install package
void OakCompiler::uninstall_package(const std::string &_name) {
  assert(false);
}

/// Create a new template package with the given name
void OakCompiler::new_package(const std::string &_name) {
  const std::filesystem::path path(_name);

  // Package dir
  if (std::filesystem::exists(path)) {
    if (!std::filesystem::is_directory(path)) {
      throw std::runtime_error(
          "Cannot create package '" + path.string() +
          "': A non-directory of that name already exists.");
    }
  } else {
    std::filesystem::create_directory(path);
  }

  // `tests` dir
  if (std::filesystem::exists(path / "tests")) {
    if (!std::filesystem::is_directory(path / "tests")) {
      throw std::runtime_error(
          "Cannot create package test dir '" +
          (path / "tests").string() +
          "': A non-directory of that name already exists.");
    }
  } else {
    std::filesystem::create_directory(path / "tests");
  }

  // Spec file
  if (!std::filesystem::exists(path / "spec.oak")) {
    std::ofstream f(path / "spec.oak");
    // clang-format off
    f << "/*\n"
         "Generated by acorn\n"
         "*/\n\n"
         "pragma!(\"no_dialect\");\n\n";
    // clang-format on

    std::map<std::string, std::string> key_value_pairs;
    key_value_pairs["INSTALL!"] = _name + "/install.oak";
    for (const auto &key :
         {"ABOUT!", "AUTHOR!", "EMAIL", "LICENSE!", "SOURCE!",
          "VERSION!", "YEAR!"}) {
      std::string response;
      std::cout << "Value for '" << key << "': ";
      std::getline(std::cin, response);
      key_value_pairs[key] = response;
    }

    for (const auto &p : key_value_pairs) {
      if (!p.second.empty()) {
        f << "let " << _name << "::" << p.first << " = \""
          << p.second << "\";\n";
      }
    }
  }

  // Install file
  if (!std::filesystem::exists(path / "install.oak")) {
    std::ofstream f(path / "install.oak");
    // clang-format off
    f << "/*\n"
         "Generated by acorn\n"
         "*/\n\n"
         "pragma!(\"no_dialect\");\n"
         "\n"
         "compile_time::system!(\n"
         "  \"echo 'Put commands here'\"\n"
         ");\n";
    // clang-format on
  }
}

/// Compile according to settings
void OakCompiler::operator()() {
  debug_print();
  bool did_fail = false;
  if (settings.is_compile()) {
    try {
      do_compilation();
    } catch (...) {
      if (!settings.compile_settings()
               .pragmas[settings.compile_settings().entry_point]
               .contains("compile_should_fail")) {
        throw;
      }
      did_fail = true;
    }
    if (!did_fail &&
        settings.compile_settings()
            .pragmas[settings.compile_settings().entry_point]
            .contains("compile_should_fail")) {
      throw std::runtime_error(
          "Compilation succeeded with "
          "pragma!(\"compile_should_fail\")");
    }
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
    }

    catch (...) {
      throw std::runtime_error(
          "An unknown error occurred while loading dialect "
          "file '" +
          settings.dialect.value().string() + "'");
    }
  }

  try {
    do_file(csettings.entry_point, settings);
  } catch (std::runtime_error &e) {
    throw std::runtime_error(
        "Error occurred while loading entry point " +
        csettings.entry_point.string() + ":\n" + e.what());
  }

  catch (...) {
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
        for (const auto &obj : csettings.objects) {
          command += " " + obj.string();
        }
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
              TRANSLATE_COMPILE_LINK_AND_EXECUTE &&
      !settings.compile_settings()
           .pragmas[settings.compile_settings().entry_point]
           .contains("no_run")) {
    bool should_succeed =
        !settings.compile_settings()
             .pragmas[settings.compile_settings().entry_point]
             .contains("run_should_fail");

    int execution_result = system(("." / linked_file).c_str());

    if (should_succeed) {
      if (execution_result != 0) {
        throw std::runtime_error(
            "Execution of file '" + linked_file.string() +
            "' failed with exit code " +
            std::to_string(execution_result));
      }
    } else {
      if (execution_result == 0) {
        throw std::runtime_error(
            "Execution of file '" + linked_file.string() +
            "' should have failed, but ran successfully");
      }
    }
  }
}

void OakCompiler::do_testing() {
  debug_print();

  // Variables needed by entire process
  auto &tsettings = settings.test_settings();

  // Test log file
  const static std::filesystem::path test_log_path =
      "acorn_test.log";
  std::ofstream test_log(test_log_path);

  // Each key is a `test` dir
  // Each entry is a 4-tuple: (tried to compile, tried to run,
  // compiled successfully, ran successfully)
  std::map<std::filesystem::path,
           std::tuple<uint, uint, uint, uint>>
      dirs;
  std::list<std::filesystem::path> compile_problems,
      run_problems;

  settings.ostream << "Running test cases...\n";

  // Collect locations to test
  if (std::filesystem::exists(std::filesystem::current_path() /
                              "tests") &&
      std::filesystem::is_directory(
          std::filesystem::current_path() / "tests")) {
    tsettings.dirs.push_back(std::filesystem::current_path());
  }

  // Run all tests
  for (const auto &dir_to_search : tsettings.dirs) {
    test_log << "Testing dir " << dir_to_search << "\n\n";

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
    uint64_t tried_to_compile = 0, tried_to_run = 0,
             compiled_successfully = 0, ran_successfully = 0;

    std::set<std::filesystem::directory_entry> cases;
    for (const auto &test_file :
         std::filesystem::directory_iterator(test_path)) {
      cases.insert(test_file);
    }

    for (const auto &test_file : cases) {
      if (std::filesystem::is_regular_file(test_file) &&
          test_file.path().string().ends_with(".oak") &&
          !test_file.path().string().ends_with(".macro.oak")) {
        test_log << "Testing file " << test_file << "\n\n";

        ++tried_to_compile;
        ++compiled_successfully;

        const std::filesystem::path target =
            test_file.path().string() + ".out";

        std::chrono::high_resolution_clock::time_point start =
            std::chrono::high_resolution_clock::now();

        OakCompiler comp(test_log);
        comp.settings.compile_settings().entry_point =
            test_file;
        comp.settings.compile_settings().target = target;
        bool compilation_succeeded = true;
        int run_result = 0;

        try {
          comp();
        } catch (...) {
          compilation_succeeded = false;
          compile_problems.push_back(test_file);
          --compiled_successfully;

          if (tsettings.halt_on_compiler_failure) {
            throw std::runtime_error(
                "Compilation failed on file " +
                test_file.path().string());
          }
        }

        if (compilation_succeeded &&
            tsettings.mode !=
                Settings::TestSettings::COMPILE_ONLY) {
          ++tried_to_run;
          ++ran_successfully;

          run_result = system(target.root_path().c_str());
          if (run_result != 0) {
            run_problems.push_back(test_file);
            --ran_successfully;

            if (tsettings.mode != Settings::TestSettings::
                                      EXECUTE_IGNORE_FAILURE) {
              throw std::runtime_error(
                  "Run failed on file " +
                  test_file.path().string());
            }
          }
        }

        std::chrono::high_resolution_clock::time_point stop =
            std::chrono::high_resolution_clock::now();

        std::cout << '[';

        if (compilation_succeeded) {
          std::cout << run_result;
        } else {
          std::cout << 'F';
        }

        std::cout << "]" << std::fixed << std::setprecision(3)
                  << std::right << std::setw(10)
                  << (std::chrono::duration_cast<
                          std::chrono::microseconds>(stop -
                                                     start)
                          .count() /
                      1'000.0)
                  << " ms | " << test_file.path().string()
                  << "\n";
      }
    }

    dirs[dir_to_search] = {tried_to_compile, tried_to_run,
                           compiled_successfully,
                           ran_successfully};
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

    settings.ostream << '\n'
                     << std::fixed << std::setprecision(2)
                     << path << ":\n"
                     << "\tCompiled " << did_comp << "/"
                     << tried_comp << " ("
                     << 100.0 * (double)did_comp /
                            (tried_comp ? tried_comp : 1)
                     << "%)\n"
                     << "\tRan      " << did_run << "/"
                     << tried_run << " ("
                     << 100.0 * (double)did_run /
                            (tried_run ? tried_run : 1)
                     << "%)\n";

    total_compiles_tried += tried_comp;
    total_runs_tried += tried_run;
    total_compiles_succeeded += did_comp;
    total_runs_succeeded += did_run;
  }

  settings.ostream
      << std::fixed << std::setprecision(2)
      << "Over all test directories:\n"
      << "\tCompiled " << total_compiles_succeeded << "/"
      << total_compiles_tried << " ("
      << 100.0 * (double)total_compiles_succeeded /
             (total_compiles_tried ? total_compiles_tried : 1)
      << "%)\n"
      << "\tRan      " << total_runs_succeeded << "/"
      << total_runs_tried << " ("
      << 100.0 * (double)total_runs_succeeded /
             (total_runs_tried ? total_runs_tried : 1)
      << "%)\n\n";

  db_assert(total_runs_tried <= total_compiles_succeeded);

  if (!compile_problems.empty() || !run_problems.empty()) {
    settings.ostream << "Errors:\n";

    for (const auto &i : compile_problems) {
      settings.ostream << "C | " << i.string() << "\n";
    }
    for (const auto &i : run_problems) {
      settings.ostream << "R | " << i.string() << "\n";
    }
    settings.ostream << '\n';

    throw std::runtime_error("One or more errors occurred; "
                             "Testing mode failed.");
  } else {
    settings.ostream << "All tests passed!\n";
  }
}

/**
 * @brief
 */
void OakCompiler::syntax_check(const std::filesystem::path &_fp,
                               const std::string &_text) const {
  debug_print();
  // All detected errors: line, col, message
  std::list<std::tuple<uint64_t, uint64_t, std::string>> errors;

  // Scan for syntax errors here

  uint64_t line = 1, col = 0;
  std::stack<char> enclosure;

  for (size_t i = 0; i < _text.size(); ++i, ++col) {
    // Comments
    if (_text.at(i) == '/' && i + 1 < _text.size() &&
        _text.at(i + 1) == '/') {
      while (i + 1 < _text.size() && _text.at(i + 1) != '\n') {
        ++i, ++col;
      }
      if (_text.at(i) == '\n') {
        ++line, col = 0;
      }
    } else if (_text.at(i) == '/' && i + 1 < _text.size() &&
               _text.at(i + 1) == '*') {
      while (i + 1 < _text.size() &&
             !(_text.at(i) == '*' && _text.at(i + 1) == '/')) {
        if (_text.at(i) == '\n') {
          ++line, col = 0;
        }
        ++i, ++col;
      }
      ++i, ++col;
    }

    // Strings
    if (_text.at(i) == '\'') {
      bool skip = false;
      ++i, ++col;
      while (i < _text.size()) {
        if (skip) {
          skip = false;
        } else if (_text.at(i) == '\\') {
          skip = true;
          ++i, ++col;
          continue;
        } else if (_text.at(i) == '\'') {
          break;
        } else if (_text.at(i) == '\n') {
          ++line, col = 0;
          break;
        }
        ++i, ++col;
      }
    } else if (_text.at(i) == '"') {
      bool skip = false;
      ++i, ++col;
      while (i < _text.size()) {
        if (skip) {
          skip = false;
        } else if (_text.at(i) == '\\') {
          skip = true;
          ++i, ++col;
          continue;
        } else if (_text.at(i) == '"') {
          break;
        } else if (_text.at(i) == '\n') {
          ++line, col = 0;
          break;
        }
        ++i, ++col;
      }
    } else if (_text.at(i) == '`') {
      bool skip = false;
      ++i, ++col;
      while (i < _text.size()) {
        if (skip) {
          skip = false;
        } else if (_text.at(i) == '\\') {
          skip = true;
          ++i, ++col;
          continue;
        } else if (_text.at(i) == '`') {
          break;
        } else if (_text.at(i) == '\n') {
          ++line, col = 0;
          break;
        }
        ++i, ++col;
      }
    }

    else {
      // Everything else
      switch (_text.at(i)) {
      case '\n':
        ++line, col = 0;
        break;
      case '[':
      case '(':
      case '{':
        enclosure.push(_text.at(i));
        break;
      case ']':
        if (enclosure.empty()) {
          errors.push_back({line, col, "Too many ']'"});
        } else if (enclosure.top() != '[') {
          errors.push_back({line, col,
                            std::string("Tried to end '") +
                                enclosure.top() +
                                "' with ']'"});
        } else {
          enclosure.pop();
        }
        break;
      case ')':
        if (enclosure.empty()) {
          errors.push_back({line, col, "Too many ')'"});
        } else if (enclosure.top() != '(') {
          errors.push_back({line, col,
                            std::string("Tried to end '") +
                                enclosure.top() +
                                "' with ')'"});
        } else {
          enclosure.pop();
        }
        break;
      case '}':
        if (enclosure.empty()) {
          errors.push_back({line, col, "Too many '}'"});
        } else if (enclosure.top() != '{') {
          errors.push_back({line, col,
                            std::string("Tried to end '") +
                                enclosure.top() +
                                "' with '}'"});
        } else {
          enclosure.pop();
        }
        break;
      }

      if (col == 65) {
        errors.push_back({line, col, "Line too long!"});
      }
    }
  }

  // If errors were found, throw them
  if (!errors.empty()) {
    for (const auto &p : errors) {
      settings.ostream << _fp.string() << ":" << std::get<0>(p)
                       << "." << std::get<1>(p) << "> '"
                       << std::get<2>(p) << "'\n";
    }

    throw std::runtime_error(std::to_string(errors.size()) +
                             " syntax error(s) occurred.");
  }
}

void OakCompiler::fix_math(
    std::list<Lexer::Token> &_token_stream) {
  // Iterate through the token stream, replace all instances of
  // the given operator with the given op fn call name (EG '+'
  // -> 'Add'). Precedence is embedded in the order in which you
  // call this lambda
  const auto resolve_binary_operator =
      [&](const std::string &_operator,
          const std::string &_op_name) {
        // Scan strm
        for (auto it = _token_stream.begin();
             it != _token_stream.end(); ++it) {
          // On match
          if (it->type == "OPERATOR" && it->text == _operator) {
            std::list<Lexer::Token>::iterator
                first_of_lhs,         // First tok in lhs
                first_after_lhs = it, // The single-token op
                first_after_rhs;      // First tok after rhs

            // Find lhs
            first_of_lhs = std::prev(it);
            if (it == _token_stream.begin() ||
                *first_of_lhs == "(") {
              throw std::runtime_error(
                  "At " + it->file.string() + ":" +
                  std::to_string(it->line) + "." +
                  std::to_string(it->col) +
                  "> Malformed operator LHS");
            } else if (*first_of_lhs == ")") {
              int depth = 0;
              do {
                if (*first_of_lhs == "(") {
                  ++depth;
                } else if (*first_of_lhs == ")") {
                  --depth;
                }
                --first_of_lhs;
              } while (depth != 0);
              if (first_of_lhs->type != "ID") {
                ++first_of_lhs;
              }
            }
            while (std::prev(first_of_lhs)->text == ".") {
              first_of_lhs = std::prev(first_of_lhs, 2);
            }

            // Find rhs
            first_after_rhs = std::next(it);
            if (it == _token_stream.begin() ||
                *first_after_rhs == ")") {
              throw std::runtime_error(
                  "Malformed operator LHS");
            } else if (std::next(first_after_rhs)->text ==
                       "(") {
              int depth = 0;
              do {
                ++first_after_rhs;
                if (*first_after_rhs == "(") {
                  ++depth;
                } else if (*first_after_rhs == ")") {
                  --depth;

                  if (depth == 0) {
                    ++first_after_rhs;
                  }
                }
              } while (depth != 0);
            } else {
              ++first_after_rhs;
            }

            while (first_after_rhs->text == ".") {
              first_after_rhs = std::next(first_after_rhs, 2);
            }

            // Operate
            // "lhs _operator rhs" -> "_op_name ( lhs , rhs )"
            // Beginning of call: "Name ("
            _token_stream.insert(
                first_of_lhs,
                Lexer::Token(_op_name, it->file, it->line,
                             it->col, "ID"));
            _token_stream.insert(first_of_lhs,
                                 Lexer::Token("(", it->file,
                                              it->line, it->col,
                                              "OPERATOR"));

            // Separating comma
            first_after_lhs->text = ",";
            first_after_lhs->type = "OPERATOR";

            // End parenthesis
            _token_stream.insert(first_after_rhs,
                                 Lexer::Token(")", it->file,
                                              it->line, it->col,
                                              "OPERATOR"));
          }
        }
      };

  // Same, but for prefix unary operators
  // Note: There are no suffix unary operators in oak
  const auto resolve_unary_operator =
      [&](const std::string &_operator,
          const std::string &_op_name) {
        // Scan strm
        for (auto it = _token_stream.begin();
             it != _token_stream.end(); ++it) {
          // On match
          if (it->type == "OPERATOR" && it->text == _operator) {
            std::list<Lexer::Token>::iterator
                first_after_lhs = it, // The single-token op
                first_after_rhs;      // First tok after rhs

            // Find rhs
            first_after_rhs = std::next(it);
            if (it == _token_stream.begin() ||
                *first_after_rhs == ")") {
              throw std::runtime_error(
                  "Malformed operator LHS");
            } else if (std::next(first_after_rhs)->text ==
                       "(") {
              int depth = 0;
              do {
                ++first_after_rhs;
                if (*first_after_rhs == "(") {
                  ++depth;
                } else if (*first_after_rhs == ")") {
                  --depth;

                  if (depth == 0) {
                    ++first_after_rhs;
                  }
                }
              } while (depth != 0);
            } else {
              ++first_after_rhs;
            }

            while (first_after_rhs->text == ".") {
              first_after_rhs = std::next(first_after_rhs, 2);
            }

            // Operate
            // "_operator rhs" -> "_op_name ( rhs )"
            // Beginning of call: "Name ("
            _token_stream.insert(
                first_after_lhs,
                Lexer::Token(_op_name, it->file, it->line,
                             it->col, "ID"));
            first_after_lhs->text = "(";
            first_after_lhs->type = "OPERATOR";

            // End parenthesis
            _token_stream.insert(first_after_rhs,
                                 Lexer::Token(")", it->file,
                                              it->line, it->col,
                                              "OPERATOR"));
          }
        }
      };

  // Unary operator precedence
  const std::list<std::pair<std::string, std::string>>
      unary_precedence = {
          {"!", "Not"}, {"++", "Incr"}, {"--", "Decr"}};

  // Binary operator precedence
  const std::list<std::pair<std::string, std::string>>
      binary_precedence = {
          {"*", "Mult"}, {"/", "Div"},  {"%", "Mod"},
          {"+", "Add"},  {"-", "Sub"},  {"&&", "Andd"},
          {"||", "Orr"}, {"=", "Copy"}, {"==", "Eq"},
          {"!=", "Neq"}, {"<", "Less"}, {">", "Great"},
          {"<=", "Leq"}, {">", "Greq"},
      };

  for (const auto &i : unary_precedence) {
    resolve_unary_operator(i.first, i.second);
  }

  for (const auto &i : binary_precedence) {
    resolve_binary_operator(i.first, i.second);
  }
}

uint64_t
OakCompiler::preprocess(std::list<Lexer::Token> &_token_stream,
                        Settings::CompileSettings &_csettings) {
  debug_print();
  bool did_change = false;
  uint64_t passes = 0;

  std::optional<std::shared_ptr<std::ostream>> log;
  if (_csettings.rule_logs) {
    if (_csettings.dump_file.has_value()) {
      log = _csettings.dump_file.value();
    } else {
      log = std::make_shared<std::ofstream>("acorn_rules.log");
    }

    **log << "Raw lexed :\n ";
    uint64_t prev_line = 0;
    std::filesystem::path prev_path;
    for (const auto &tok : _token_stream) {
      if (tok.file != prev_path) {
        **log << '\n' << tok.file << ":\n";
        prev_path = tok.file;
      }
      if (tok.line != prev_line) {
        **log << "\n" << tok.line << "\t|";
        prev_line = tok.line;
      }
      **log << ' ' << tok.text;
    }
    **log << '\n';
  }

  do {
    ++passes;
    did_change = false;

    // Macro definitions
    bool saw_let = false;
    for (auto it = _token_stream.begin();
         it != _token_stream.end(); ++it) {
      try {
        if (*it == "let") {
          saw_let = true;
        } else if (saw_let && *it != "!" &&
                   it->text.find('!') != std::string::npos) {
          macros.process_definition(_token_stream, it,
                                    _token_stream.end());
          saw_let = false;
          --it;
        } else {
          saw_let = false;
        }
      } catch (std::runtime_error &e) {
        if (it == _token_stream.end()) {
          throw e;
        }
        throw std::runtime_error(
            "At " + it->file.string() + ":" +
            std::to_string(it->line) + "." +
            std::to_string(it->col) + "\n" + e.what());
      }

      catch (...) {
        if (it == _token_stream.end()) {
          throw;
        }
        throw std::runtime_error(
            "At " + it->file.string() + ":" +
            std::to_string(it->line) + "." +
            std::to_string(it->col) + "\nUnknown error");
      }
    }

    // Resolve includes and packages
    for (auto it = _token_stream.begin();
         it != _token_stream.end(); ++it) {

      try {

        if (*it == "include!") {
          const auto args = MacroManager::get_macro_args(
              _token_stream, it, _token_stream.end());

          try {
            for (const auto &f : args) {
              std::filesystem::path path(f);

              if (std::filesystem::exists(
                      _csettings.include_path / path)) {
                // Package file exists
                if (std::filesystem::exists(path)) {
                  settings.ostream
                      << "Warning: Including local file "
                      << path
                      << " over package file of same name\n";
                } else {
                  path = _csettings.include_path / path;
                }
              }

              const auto backup = rules.purge_entry_points();

              do_file(path, settings);

              rules.purge_entry_points();
              for (const auto &item : backup) {
                rules.add_entry_point(item);
              }
            }
          } catch (std::runtime_error &e) {
            throw std::runtime_error(
                "In file included from " + it->file.string() +
                ":" + std::to_string(it->line) + "." +
                std::to_string(it->col) + "\n" + e.what());
          }

          catch (...) {
            throw std::runtime_error(
                "In file included from " + it->file.string() +
                ":" + std::to_string(it->line) + "." +
                std::to_string(it->col) + "\nUnknown error");
          }

        } else if (*it == "link!") {
          const auto args = MacroManager::get_macro_args(
              _token_stream, it, _token_stream.end());
          for (const auto &f : args) {

            std::filesystem::path p(f);
            if (std::filesystem::exists(
                    _csettings.include_path / p)) {
              // Package file exists
              if (std::filesystem::exists(p)) {
                settings.ostream
                    << "Warning: Linking local object " << p
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
            if (std::filesystem::exists(
                    _csettings.include_path / p)) {
              // Package file exists
              if (std::filesystem::exists(p)) {
                settings.ostream
                    << "Warning: Linking local object " << p
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
          _csettings.pragmas[it->file][args.front().text] =
              std::next(args.begin())->text;
        } else if (*it == "rule_new!") {
          const auto args = MacroManager::get_macro_args(
              _token_stream, it, _token_stream.end());

          Rule to_add;
          std::string name;

          if (args.size() < 3) {
            throw std::runtime_error(
                "Malformed rule::new! call: Arguments must "
                "be "
                "rule_name, input_rule, output_rule, "
                "[engine_name], [prerequisites...]");
          }

          // Name, input, output (using sapling engine)
          name = args.front();
          to_add.input_pattern = *std::next(args.begin());
          to_add.output_pattern = *std::next(args.begin(), 2);
          to_add.engine = "sapling";

          if (args.size() == 4) {
            // Name, input, output, engine
            to_add.engine = *std::next(args.begin(), 3);
          } else if (args.size() > 4) {
            // Name, input, output, engine, prerequisites
            to_add.engine = *std::next(args.begin(), 3);
            for (auto it = std::next(args.begin(), 4);
                 it != args.end(); ++it) {
              to_add.prereqs.push_back(*it);
            }
          }

          rules.register_rule(name, to_add);
        } else if (*it == "rule_use!") {
          did_change |= true;
          const auto args = MacroManager::get_macro_args(
              _token_stream, it, _token_stream.end());
          for (const auto &arg : args) {
            rules.add_entry_point(arg);
          }
        } else if (*it == "rule_remove!") {
          const auto args = MacroManager::get_macro_args(
              _token_stream, it, _token_stream.end());
          for (const auto &arg : args) {
            rules.remove_entry_point(arg);
          }
        } else if (*it == "rule_bundle!") {
          const auto args = MacroManager::get_macro_args(
              _token_stream, it, _token_stream.end());

          std::list<std::string> entails;
          for (auto it = std::next(args.begin());
               it != args.end(); ++it) {
            entails.push_back(*it);
          }

          rules.register_bundle(args.front(), entails);
        } else if (*it == "compile_time_system!") {
          settings.ostream
              << it->file.string() << ":" << it->line << "."
              << it->col
              << "> compile_time::system! is running "
                 "system command `";

          const auto args = MacroManager::get_macro_args(
              _token_stream, it, _token_stream.end());
          std::string cmd;
          for (const auto &arg : args) {
            if (!cmd.empty()) {
              cmd.push_back(' ');
            }
            cmd += arg.text;
          }

          settings.ostream << cmd << "`\n" << std::flush;

          if (!_csettings.no_confirm) {
            settings.ostream << "Allow? [N/y/a] ";
            char choice = std::cin.get();

            switch (choice) {
            default:
              throw std::runtime_error("Abort!");
            case 'a':
            case 'A':
              settings.ostream << "Not asking again!\n";
              _csettings.no_confirm = true;
            case 'y':
            case 'Y':
              break;
            }
          } else {
            settings.ostream
                << "(no_confirm is enabled, so running "
                   "without asking)\n";
          }

          const auto old_cwd = std::filesystem::current_path();
          std::filesystem::current_path(it->file.parent_path());

          auto result = system(cmd.c_str());

          std::filesystem::current_path(old_cwd);

          if (result != 0) {
            throw std::runtime_error(
                "System call '" + cmd +
                "' exited with nonzero exit code " +
                std::to_string(result));
          }
        }
      } catch (std::runtime_error &e) {
        if (it == _token_stream.end()) {
          throw e;
        }
        throw std::runtime_error(
            "At " + it->file.string() + ":" +
            std::to_string(it->line) + "." +
            std::to_string(it->col) + "\n" + e.what());
      }

      catch (...) {
        if (it == _token_stream.end()) {
          throw;
        }
        throw std::runtime_error(
            "At " + it->file.string() + ":" +
            std::to_string(it->line) + "." +
            std::to_string(it->col) + "\nUnknown error");
      }
    }

    // Resolve macros (functional and inline)
    for (auto it = _token_stream.begin();
         it != _token_stream.end(); ++it) {
      try {
        if (*it != "!" && it->type == "ID" &&
            it->text.find('!') != std::string::npos &&
            !MacroManager::reserved_macro_names.contains(
                it->text)) {
          did_change |= true;
          macros.replace(_token_stream, it,
                         _token_stream.end());
        }
      } catch (std::runtime_error &e) {
        if (it == _token_stream.end()) {
          throw e;
        }
        throw std::runtime_error(
            "At " + it->file.string() + ":" +
            std::to_string(it->line) + "." +
            std::to_string(it->col) + "\n" + e.what());
      }

      catch (...) {
        if (it == _token_stream.end()) {
          throw;
        }
        throw std::runtime_error(
            "At " + it->file.string() + ":" +
            std::to_string(it->line) + "." +
            std::to_string(it->col) + "\nUnknown error");
      }
    }

    // Fix math
    fix_math(_token_stream);

    // Apply ruleset
    did_change |= rules.process_text(_token_stream);

    if (log.has_value()) {
      **log << "\nAfter pass " << passes << ":\n";
      uint64_t prev_line = 0;
      std::filesystem::path prev_path;
      for (const auto &tok : _token_stream) {
        if (tok.file != prev_path) {
          **log << '\n' << tok.file << ":\n";
          prev_path = tok.file;
        }
        if (tok.line != prev_line) {
          **log << "\n" << tok.line << "\t|";
          prev_line = tok.line;
        }
        **log << ' ' << tok.text;
      }
      **log << '\n';
    }
  } while (did_change &&
           passes < _csettings.preprocess_pass_limit);

  return passes;
}

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
void OakCompiler::do_file(const std::filesystem::path &_path,
                          Settings &_settings) {
  debug_print();

  Settings::CompileSettings &csettings =
      _settings.compile_settings();

  if (csettings.visited.contains(_path)) {
    return;
  }
  csettings.visited.insert(_path);

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
    // If requested, dump
    if (csettings.dump_file.has_value()) {
      p.dump(*csettings.dump_file.value(), token_stream);
    }

    throw std::runtime_error("Error occurred while lexing " +
                             _path.string() + ":\n" + e.what());
  }

  catch (...) {
    // If requested, dump
    if (csettings.dump_file.has_value()) {
      p.dump(*csettings.dump_file.value(), token_stream);
    }

    throw std::runtime_error(
        "An unknown error occurred while lexing " +
        _path.string() + "");
  }

  // Preprocess (including includes)
  preprocess(token_stream, csettings);

  // If requested, syntax check
  if (csettings.do_syntax_check) {
    syntax_check(_path, text);
  }

  // Do actual parsing here
  debug_print();
  p.parse_global(token_stream, settings);

  // If requested, dump
  if (csettings.dump_file.has_value()) {
    p.dump(*csettings.dump_file.value(), token_stream);
  }
}
