#include "oakc.hpp"
#include "debug.hpp"
#include "lexer.hpp"
#include "macro.hpp"
#include "parser.hpp"
#include "rule.hpp"
#include "settings.hpp"
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

std::filesystem::path OakCompiler::resolve_path(
    const std::string &_requested,
    const std::filesystem::path &_cur_file) {
  const auto local = _cur_file.parent_path() / _requested;
  const auto global =
      settings.compile_settings().include_path / _requested;
  const bool global_exists = std::filesystem::exists(global);

  if (std::filesystem::exists(local)) {
    if (global_exists &&
        std::filesystem::canonical(global) !=
            std::filesystem::canonical(local)) {
      settings.warn("Choosing local file " + _requested +
                    " over package file of same name");
    }
    return std::filesystem::canonical(local);
  } else if (global_exists) {
    return std::filesystem::canonical(global);
  } else {
    throw std::runtime_error("Failed to resolve file '" +
                             _requested + "'");
  }
}

/**
 * @brief Runs a command, writes its * output CHAR-BY-CHAR to
 * the given output stream, and returns its exit code. This may
 * be very inefficient!
 * @param _cmd The command to run
 * @param _to The stream to write output to
 * @returns The exit code: 0 on success, -1 for popen error
 */
int strm_cmd_output(const std::string &_cmd,
                    std::ostream &_to) noexcept {
  debug_print();

  auto cout_buffer = std::cout.rdbuf();
  std::stringstream cout_sstream;
  std::cerr.rdbuf(cout_sstream.rdbuf());

  FILE *pipe = popen(_cmd.c_str(), "r");
  if (!pipe) {
    _to << cout_sstream.str();
    std::cerr.rdbuf(cout_buffer);
    return -1;
  }
  while (true) {
    int c = fgetc(pipe);
    if (c == EOF) {
      break;
    }
    _to.put(c);
  }

  _to << cout_sstream.str();
  std::cerr.rdbuf(cout_buffer);
  return pclose(pipe) / 256;
}

/// Print the version of Acorn
void OakCompiler::print_version() noexcept {
  debug_print();
  std::cout << "Acorn version " << ACORN_VERSION << "\n"
            << "MIT Licensed\n";
}

/// Print the help text for Acorn
void OakCompiler::print_help_text() noexcept {
  debug_print();

  /*
  Used:   AcCdDeEghilnoOpqrRsStTuUvwxy
  Unused: abBfFGHIjJkKLmMNPQVWXYzZ
  */

  // clang-format off
  std::cout
      << "Acorn\n"
         "Translator for the Oak programming language\n"
         "github.com/jorbDehmel/oak\n"
         "\n"
         "-------------------------------------------------------------\n"
         "\n"
         "Compilation mode"
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
         " -A | --uninstall  |   | Uninstall Acorn\n"
         " -c | --compile    |   | Translate and compile to object file\n"
         " -C | --cd         | 1 | Change to the given directory\n"
         " -d | --debug      |   | Toggle debug mode (default off)\n"
         " -D | --dialect    | 1 | Use some dialect file\n"
         " -e | --clean      |   | Clean all local build files\n"
         " -E | --execute    |   | Translate, compile, link, execute\n"
         " -g | --exe_debug  |   | Use debugging flag at compile-time\n"
         " -h | --help       |   | Print help (this)\n"
         " -i | --install    | 1 | Install some package\n"
         " -I | --no_warn    |   | Ignore/silence all warnings\n"
         " -l | --link       |   | Translate, compile, and link\n"
         " -m | --time       |   | Print the time taken\n"
         " -n | --no_save    |   | Syntax check and halt\n"
         " -o | --output     | 1 | Set output file\n"
         " -O | --optimize   |   | Use optimization flag at compile\n"
         " -p | --prettify   |   | Use clang-format on the produced C\n"
         " -q | --quit       |   | Quit without error immediately\n"
         " -Q | --query      |   | List all installed packages\n"
         " -r | --reinstall  | 1 | Reinstall some package\n"
         " -R | --remove     | 1 | Remove a package\n"
         " -s | --size       |   | Show Acorn/Oak disk usage\n"
         " -S | --install    | 1 | Install some package\n"
         " -t | --translate  |   | Translate to C\n"
         " -T | --test       | * | Testing mode\n"
         " -u | --dump       |   | Save dump files\n"
         " -U | --rule_logs  |   | Save rule log files\n"
         " -v | --version    |   | Show version and halt\n"
         " -w | --new        |   | Create a new package\n"
         " -W | --werror     |   | Warnings to errors\n"
         " -x | --syntax     |   | Toggle syntax checks (default on)\n"
         " -y | --no_confirm |   | Always allow compile_time::system!\n"
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
         "\t`acorn -E -D foo.oak -o a.oak`\n"
         "\tOR `acorn -DoE foo.oak a.oak`\n"
         "\n"
         "-------------------------------------------------------------\n"
         "\n"
         "Version and License\n"
         "\n";
  // clang-format on
  print_version();
}

/// Print the total disk usage of Acorn
void OakCompiler::print_size() noexcept {
  debug_print();
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
  debug_print();
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
  debug_print();
  std::atexit(internal_uninstall);
}

/// Purge all temporary files
void OakCompiler::clean() {
  debug_print();
  std::list<std::filesystem::path> to_erase;
  for (const auto &path :
       std::filesystem::recursive_directory_iterator(
           std::filesystem::current_path())) {
    if (!std::filesystem::is_directory(path)) {
      const std::string filename =
          path.path().filename().string();
      if (filename.find(".oak.") != std::string::npos ||
          filename == "acorn_test.log" ||
          filename == "acorn_rules.log") {
        to_erase.push_back(path.path());
        std::cout << path.path() << '\n';
      }
    }
  }

  if (to_erase.empty()) {
    return;
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

/// Create a new template package with the given name
void OakCompiler::new_package(const std::string &_name) {
  debug_print();
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
    } catch (RunError &) {
      throw;
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

  // Standardize
  if (!std::filesystem::exists(csettings.entry_point)) {
    throw std::runtime_error("Entry point '" +
                             csettings.entry_point.string() +
                             "' does not exist");
  }
  csettings.entry_point =
      std::filesystem::canonical(csettings.entry_point);

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
    do_file(csettings.entry_point, csettings.entry_point);
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

    bool saw_main = false;
    const auto res = p.fetch_symbol("main");
    if (res.has_value() &&
        std::holds_alternative<std::list<Parser::FnInfo>>(
            res.value())) {
      for (const auto &def :
           std::get<std::list<Parser::FnInfo>>(res.value())) {
        if (!def.tags.contains("casual") &&
            def.tags.at("file") == csettings.entry_point) {
          saw_main = true;
          break;
        }
      }
    }

    if (!saw_main) {
      throw std::runtime_error(
          "Cannot link translation unit: No main function in "
          "entry point");
    }

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

    std::string command = "." / linked_file;
    if (settings.compile_settings()
            .pragmas[settings.compile_settings().entry_point]
            .contains("run_cmd")) {
      const auto fmt =
          settings.compile_settings()
              .pragmas[settings.compile_settings().entry_point]
              .at("run_cmd");
      bool skip = false;
      command = "";
      for (const auto &c : fmt) {
        if (skip) {
          command += c;
          skip = false;
        } else if (c == '\\') {
          skip = true;
        } else if (c == '%') {
          command += linked_file;
        } else {
          command += c;
        }
      }
    }

    int execution_result =
        strm_cmd_output(command, settings.ostream);

    if (should_succeed) {
      if (execution_result != 0) {
        throw RunError("Execution of file '" +
                           linked_file.string() +
                           "' failed with exit code " +
                           std::to_string(execution_result),
                       execution_result);
      }
    } else {
      if (execution_result == 0) {
        throw RunError(
            "Execution of file '" + linked_file.string() +
                "' should have failed, but ran successfully",
            -1);
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
  double total_ms = 0.0;
  double total_failed_ms = 0.0;

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

        if (!tsettings.should_execute()) {
          comp.settings.compile_settings().mode = Settings::
              CompileSettings::TRANSLATE_COMPILE_AND_LINK;
        } else {
          ++tried_to_run;
          ++ran_successfully;
          comp.settings.compile_settings().mode =
              Settings::CompileSettings::
                  TRANSLATE_COMPILE_LINK_AND_EXECUTE;
        }

        bool compilation_succeeded = true, run_succeeded = true;
        int run_result = 0;

        try {
          comp();
        } catch (RunError &e) {
          run_problems.push_back(test_file);
          --ran_successfully;
          run_succeeded = false;
          run_result = e.exit_code;

          if (tsettings.fail_with_execute()) {
            throw std::runtime_error(e.what());
          }
        } catch (...) {
          compilation_succeeded = false;
          compile_problems.push_back(test_file);
          --compiled_successfully;

          if (tsettings.should_execute()) {
            --tried_to_run;
            --ran_successfully;
          }

          if (tsettings.fail_with_compile()) {
            throw std::runtime_error(
                "Compilation failed on file " +
                test_file.path().string());
          }
        }

        std::chrono::high_resolution_clock::time_point stop =
            std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration_cast<
                        std::chrono::microseconds>(stop - start)
                        .count() /
                    1'000.0;
        total_ms += ms;
        if (!compilation_succeeded || !run_succeeded) {
          total_failed_ms += ms;
        }

        std::cout << '[';

        if (compilation_succeeded) {
          std::cout << run_result;
        } else {
          std::cout << 'F';
        }

        std::cout << "]" << std::fixed << std::setprecision(3)
                  << std::right << std::setw(10) << ms
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

  settings.ostream << "Total ms:        " << std::fixed
                   << std::setprecision(3) << std::right
                   << std::setw(10) << total_ms << '\n'
                   << "Mean ms:         " << std::fixed
                   << std::setprecision(3) << std::right
                   << std::setw(10)
                   << total_ms / total_compiles_tried << '\n';

  if (total_compiles_succeeded != 0) {
    settings.ostream << "Mean success ms: " << std::fixed
                     << std::setprecision(3) << std::right
                     << std::setw(10)
                     << (total_ms - total_failed_ms) /
                            total_compiles_succeeded
                     << '\n';
  }
  if (total_compiles_tried - total_compiles_succeeded != 0) {
    settings.ostream << "Mean failure ms: " << std::fixed
                     << std::setprecision(3) << std::right
                     << std::setw(10)
                     << total_failed_ms /
                            (total_compiles_tried -
                             total_compiles_succeeded)
                     << '\n';
  }
  settings.ostream << '\n';

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
  debug_print();
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
                  "> Malformed operator '" + _operator +
                  "' LHS");
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
            if (first_after_rhs == _token_stream.end() ||
                *first_after_rhs == ")") {
              throw std::runtime_error(
                  "At " + it->file.string() + ":" +
                  std::to_string(it->line) + "." +
                  std::to_string(it->col) +
                  "> Malformed operator '" + _operator +
                  "' LHS");
            } else if (std::next(first_after_rhs)->text ==
                       "(") {
              auto start_pos = first_after_rhs;
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

              // Erase matched parenthesis
              while (start_pos->text == "(" &&
                     std::prev(first_after_rhs)->text == ")") {
                std::cout << __FILE__ << ":" << __LINE__ << '\n'
                          << std::flush;
                start_pos = _token_stream.erase(start_pos);
                _token_stream.erase(std::prev(first_after_rhs));
              }
            } else {
              ++first_after_rhs;
            }

            while (first_after_rhs->text == ".") {
              first_after_rhs = std::next(first_after_rhs, 2);
            }

            // Operate
            // "lhs _operator rhs" -> "_op_name ( lhs , rhs )"
            // "lhs _operator (...)" -> "_op_name ( lhs , ... )"
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
                  "At " + it->file.string() + ":" +
                  std::to_string(it->line) + "." +
                  std::to_string(it->col) +
                  "> Malformed operator '" + _operator +
                  "' LHS");
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
      unary_precedence = {{"!", "Not"},
                          {"++", "Incr"},
                          {"--", "Decr"},
                          {"~", "Flip"}};

  // Binary operator precedence
  const std::list<std::pair<std::string, std::string>>
      binary_precedence = {
          {"&", "And"},     {"|", "Or"},       {"*", "Mult"},
          {"/", "Div"},     {"%", "Mod"},      {"+", "Add"},
          {"-", "Sub"},     {"==", "Eq"},      {"!=", "Neq"},
          {"<", "Less"},    {">", "Great"},    {"<=", "Leq"},
          {">=", "Greq"},   {"&&", "Andd"},    {"||", "Orr"},
          {"=", "Copy"},    {"&=", "AndEq"},   {"|=", "OrEq"},
          {"<<=", "LBSEq"}, {">>=", "RBSEq"},  {"*=", "MultEq"},
          {"/=", "DivEq"},  {"%=", "ModEq"},   {"+=", "AddEq"},
          {"-=", "SubEq"},  {"&&=", "AnddEq"}, {"||=", "OrrEq"},
      };

  for (const auto &i : unary_precedence) {
    resolve_unary_operator(i.first, i.second);
  }

  for (const auto &i : binary_precedence) {
    resolve_binary_operator(i.first, i.second);
  }
}

uint64_t OakCompiler::preprocess(
    std::list<Lexer::Token> &_token_stream) {
  debug_print();
  bool did_change = false;
  uint64_t passes = 0;
  auto &csettings = settings.compile_settings();

  std::optional<std::shared_ptr<std::ostream>> log;
  if (csettings.rule_logs) {
    if (csettings.dump_file.has_value()) {
      log = csettings.dump_file.value();
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
        **log << "\n"
              << std::right << std::setw(8) << tok.line << " |";
        prev_line = tok.line;
      }
      **log << ' ' << tok.text;
    }
    **log << '\n';
  }

  do {
    ++passes;
    if (passes >= csettings.preprocess_pass_limit) {
      throw std::runtime_error(
          "Ruleset failed to converge in " +
          std::to_string(csettings.preprocess_pass_limit) +
          " passes");
    }

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
          auto raw_args = MacroManager::get_macro_args(
              _token_stream, it, _token_stream.end());

          std::list<Lexer::Token> args;
          for (auto it = raw_args.begin(); it != raw_args.end();
               ++it) {
            preprocess(*it);
            Lexer::Token to_add = it->front();
            for (auto inner_it = std::next(it->begin());
                 inner_it != it->end(); ++inner_it) {
              to_add.text += ' ';
              to_add.text += inner_it->text;
            }
            to_add.text =
                MacroManager::strip_string_literal(to_add.text);
            args.push_back(to_add);
          }

          try {
            for (const auto &f : args) {
              const auto backup = rules.purge_entry_points();
              do_file(f.text, f.file);
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
          auto raw_args = MacroManager::get_macro_args(
              _token_stream, it, _token_stream.end());

          std::list<Lexer::Token> args;
          for (auto it = raw_args.begin(); it != raw_args.end();
               ++it) {
            preprocess(*it);
            Lexer::Token to_add = it->front();
            for (auto inner_it = std::next(it->begin());
                 inner_it != it->end(); ++inner_it) {
              to_add.text += ' ';
              to_add.text += inner_it->text;
            }
            to_add.text =
                MacroManager::strip_string_literal(to_add.text);
            args.push_back(to_add);
          }

          for (auto it = args.begin(); it != args.end(); ++it) {
            it->text = MacroManager::strip_string_literal(*it);
          }

          for (const auto &f : args) {
            csettings.objects.push_back(
                resolve_path(f.text, f.file));
          }
        } else if (*it == "flag!") {
          auto raw_args = MacroManager::get_macro_args(
              _token_stream, it, _token_stream.end());

          std::list<Lexer::Token> args;
          for (auto it = raw_args.begin(); it != raw_args.end();
               ++it) {
            preprocess(*it);
            Lexer::Token to_add = it->front();
            for (auto inner_it = std::next(it->begin());
                 inner_it != it->end(); ++inner_it) {
              to_add.text += ' ';
              to_add.text += inner_it->text;
            }
            to_add.text =
                MacroManager::strip_string_literal(to_add.text);
            args.push_back(to_add);
          }

          for (auto it = args.begin(); it != args.end(); ++it) {
            it->text = MacroManager::strip_string_literal(*it);
          }

          for (const auto &f : args) {
            csettings.link_flags.push_back(f.text);
          }
        } else if (*it == "pragma!") {
          auto raw_args = MacroManager::get_macro_args(
              _token_stream, it, _token_stream.end());

          std::list<Lexer::Token> args;
          for (auto it = raw_args.begin(); it != raw_args.end();
               ++it) {
            preprocess(*it);
            Lexer::Token to_add = it->front();
            for (auto inner_it = std::next(it->begin());
                 inner_it != it->end(); ++inner_it) {
              to_add.text += ' ';
              to_add.text += inner_it->text;
            }
            to_add.text =
                MacroManager::strip_string_literal(to_add.text);
            args.push_back(to_add);
          }

          for (auto it = args.begin(); it != args.end(); ++it) {
            it->text = MacroManager::strip_string_literal(*it);
          }

          if (args.size() == 1) {
            args.push_back(Lexer::Token(args.front(), ""));
          }
          csettings.pragmas[it->file][args.front().text] =
              std::next(args.begin())->text;
        } else if (*it == "rule_new!") {
          auto raw_args = MacroManager::get_macro_args(
              _token_stream, it, _token_stream.end());

          std::list<Lexer::Token> args;
          for (auto it = raw_args.begin(); it != raw_args.end();
               ++it) {
            preprocess(*it);
            Lexer::Token to_add = it->front();
            for (auto inner_it = std::next(it->begin());
                 inner_it != it->end(); ++inner_it) {
              to_add.text += ' ';
              to_add.text += inner_it->text;
            }
            to_add.text =
                MacroManager::strip_string_literal(to_add.text);
            args.push_back(to_add);
          }

          for (auto it = args.begin(); it != args.end(); ++it) {
            it->text = MacroManager::strip_string_literal(*it);
          }

          if (args.size() < 3) {
            throw std::runtime_error(
                "Malformed rule::new! call: Arguments must "
                "be "
                "rule_name, input_rule, output_rule, "
                "[engine_name], [prerequisites...]");
          }

          // Name, input, output (using sapling engine)
          std::string name = args.front();
          std::string engine = "sapling";
          std::list<std::string> prereqs;
          if (args.size() == 4) {
            // Name, input, output, engine
            engine = *std::next(args.begin(), 3);
          } else if (args.size() > 4) {
            // Name, input, output, engine, prerequisites
            engine = *std::next(args.begin(), 3);
            for (auto it = std::next(args.begin(), 4);
                 it != args.end(); ++it) {
              prereqs.push_back(*it);
            }
          }

          Rule to_add(*std::next(args.begin()),
                      *std::next(args.begin(), 2), prereqs,
                      engine);

          rules.register_rule(name, to_add);
        } else if (*it == "rule_use!") {
          did_change |= true;
          auto raw_args = MacroManager::get_macro_args(
              _token_stream, it, _token_stream.end());

          std::list<Lexer::Token> args;
          for (auto it = raw_args.begin(); it != raw_args.end();
               ++it) {
            preprocess(*it);
            Lexer::Token to_add = it->front();
            for (auto inner_it = std::next(it->begin());
                 inner_it != it->end(); ++inner_it) {
              to_add.text += ' ';
              to_add.text += inner_it->text;
            }
            to_add.text =
                MacroManager::strip_string_literal(to_add.text);
            args.push_back(to_add);
          }

          for (const auto &arg : args) {
            rules.add_entry_point(
                MacroManager::strip_string_literal(arg));
          }
        } else if (*it == "rule_remove!") {
          auto raw_args = MacroManager::get_macro_args(
              _token_stream, it, _token_stream.end());

          std::list<Lexer::Token> args;
          for (auto it = raw_args.begin(); it != raw_args.end();
               ++it) {
            preprocess(*it);
            Lexer::Token to_add = it->front();
            for (auto inner_it = std::next(it->begin());
                 inner_it != it->end(); ++inner_it) {
              to_add.text += ' ';
              to_add.text += inner_it->text;
            }
            to_add.text =
                MacroManager::strip_string_literal(to_add.text);
            args.push_back(to_add);
          }

          for (const auto &arg : args) {
            rules.remove_entry_point(
                MacroManager::strip_string_literal(arg));
          }
        } else if (*it == "rule_bundle!") {
          auto raw_args = MacroManager::get_macro_args(
              _token_stream, it, _token_stream.end());

          std::list<Lexer::Token> args;
          for (auto it = raw_args.begin(); it != raw_args.end();
               ++it) {
            preprocess(*it);
            Lexer::Token to_add = it->front();
            for (auto inner_it = std::next(it->begin());
                 inner_it != it->end(); ++inner_it) {
              to_add.text += ' ';
              to_add.text += inner_it->text;
            }
            to_add.text =
                MacroManager::strip_string_literal(to_add.text);
            args.push_back(to_add);
          }

          std::list<std::string> entails;
          for (auto it = std::next(args.begin());
               it != args.end(); ++it) {
            entails.push_back(
                MacroManager::strip_string_literal(*it));
          }

          rules.register_bundle(
              MacroManager::strip_string_literal(args.front()),
              entails);
        }

        else if (*it == "unstr!") {
          Lexer::Token to_add(*it);
          auto raw_args = MacroManager::get_macro_args(
              _token_stream, it, _token_stream.end());
          to_add.text.clear();

          for (auto it = raw_args.begin(); it != raw_args.end();
               ++it) {
            for (auto inner_it = it->begin();
                 inner_it != it->end(); ++inner_it) {
              if (!to_add.text.empty()) {
                to_add.text += ' ';
              }
              to_add.text += inner_it->text;
            }
          }

          to_add.text =
              MacroManager::strip_string_literal(to_add.text);

          Lexer lexer;
          uint64_t dummy_line = to_add.line,
                   dummy_col = to_add.col;
          auto to_insert = lexer.lex(to_add.text, to_add.file,
                                     dummy_line, dummy_col);
          for (auto arg = to_insert.begin();
               arg != to_insert.end(); ++arg) {
            arg->line = to_add.line;
            _token_stream.emplace(it, *arg);
          }
        } else if (*it == "str!") {
          Lexer::Token to_add(*it);
          auto raw_args = MacroManager::get_macro_args(
              _token_stream, it, _token_stream.end());
          to_add.text.clear();

          for (auto it = raw_args.begin(); it != raw_args.end();
               ++it) {
            for (auto inner_it = it->begin();
                 inner_it != it->end(); ++inner_it) {
              if (!to_add.text.empty()) {
                to_add.text += ' ';
              }
              to_add.text += inner_it->text;
            }
          }

          // Ensure exactly one set of enclosing quotes
          to_add.text = MacroManager::make_string_literal(
              MacroManager::strip_string_literal(to_add.text));
          Lexer::classify_type(to_add);
          _token_stream.emplace(it, to_add);
        }

        else if (*it == "compile_time_system!") {
          settings.ostream
              << it->file.string() << ":" << it->line << "."
              << it->col
              << ">\ncompile_time::system! asks to run `";

          auto raw_args = MacroManager::get_macro_args(
              _token_stream, it, _token_stream.end());

          std::list<Lexer::Token> args;
          for (auto it = raw_args.begin(); it != raw_args.end();
               ++it) {
            preprocess(*it);
            Lexer::Token to_add = it->front();
            for (auto inner_it = std::next(it->begin());
                 inner_it != it->end(); ++inner_it) {
              to_add.text += ' ';
              to_add.text += inner_it->text;
            }
            to_add.text =
                MacroManager::strip_string_literal(to_add.text);
            args.push_back(to_add);
          }

          for (auto it = args.begin(); it != args.end(); ++it) {
            it->text = MacroManager::strip_string_literal(*it);
          }

          std::string cmd;
          for (const auto &arg : args) {
            if (!cmd.empty()) {
              cmd.push_back(' ');
            }
            cmd += arg.text;
          }

          settings.ostream << cmd << "` at "
                           << it->file.parent_path() << "\n"
                           << std::flush;

          if (!csettings.no_confirm) {
            settings.ostream << "Allow? [N/y/a] ";
            char choice = std::cin.get();

            switch (choice) {
            default:
              throw std::runtime_error("Abort!");
            case 'a':
            case 'A':
              settings.ostream << "Not asking again!\n";
              csettings.no_confirm = true;
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
          macros.replace(_token_stream, it, _token_stream.end(),
                         settings, *this);
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
  } while (did_change);

  // Fix math
  fix_math(_token_stream);

  if (log.has_value()) {
    **log << "\nAfter operator substitution:\n";
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
void OakCompiler::translate(std::ostream &_into) {
  debug_print();
  p.reconstruct(_into, settings.compile_settings());
}

/**
 * @brief Load the given file, following any includes found
 * within and doing any preprocessor rules as expected. This
 * is called by do_compilation, and should not be called
 * outside of it!
 */
void OakCompiler::do_file(
    const std::string &_path,
    const std::filesystem::path &_cur_file) {
  debug_print();

  const auto path = resolve_path(_path, _cur_file);

  Settings::CompileSettings &csettings =
      settings.compile_settings();

  if (csettings.visited.contains(path)) {
    if (settings.debug) {
      settings.ostream << "Ignoring repeat inclusion " << path
                       << '\n';
    }
    return;
  }

  if (settings.debug) {
    settings.ostream << "Visiting file " << path << '\n';
  }
  csettings.visited.insert(path);

  // Load and lex
  if (!std::filesystem::exists(path)) {
    throw std::runtime_error("File " + path.string() +
                             " does not exist.");
  } else if (!std::filesystem::is_regular_file(path)) {
    throw std::runtime_error(
        "File " + path.string() +
        " exists, but is not a regular file.");
  }

  std::string text;
  std::ifstream source(path);
  if (!source.is_open()) {
    throw std::runtime_error("Failed to open file " +
                             path.string());
  }

  text.assign(std::istreambuf_iterator<char>(source),
              std::istreambuf_iterator<char>());
  source.close();

  Lexer l;
  uint64_t line = 1, col = 0;
  std::list<Lexer::Token> token_stream;

  try {
    token_stream = l.lex(text, path, line, col);
  } catch (std::runtime_error &e) {
    // If requested, dump
    if (csettings.dump_file.has_value()) {
      p.dump(*csettings.dump_file.value(), token_stream,
             settings.compile_settings());
    }

    throw std::runtime_error("Error occurred while lexing " +
                             path.string() + ":\n" + e.what());
  }

  catch (...) {
    // If requested, dump
    if (csettings.dump_file.has_value()) {
      p.dump(*csettings.dump_file.value(), token_stream,
             settings.compile_settings());
    }

    throw std::runtime_error(
        "An unknown error occurred while lexing " +
        path.string() + "");
  }

  // Preprocess (including includes)
  preprocess(token_stream);

  // If requested, syntax check
  if (csettings.do_syntax_check) {
    syntax_check(path, text);
  }

  // Do actual parsing here
  debug_print();
  p.parse_global(token_stream, settings);

  // If requested, dump
  if (csettings.dump_file.has_value()) {
    p.dump(*csettings.dump_file.value(), token_stream,
           settings.compile_settings());
  }
}
