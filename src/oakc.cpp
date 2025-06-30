/**
 * @file
 */

#include "oakc.hpp"
#include "debug.hpp"
#include "parser.hpp"
#include "settings.hpp"
#include "symbols.hpp"
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
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

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

void OakCompiler::print_version() noexcept {
  debug_print();
  std::cout << "Acorn version " << acorn_version << "\n"
            << "MIT Licensed\n";
}

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

void OakCompiler::print_size() noexcept {
  debug_print();
  const static std::list<std::filesystem::path> files_to_check =
      {"/usr/bin/acorn", "/usr/bin/acorn-debug",
       "/usr/bin/oak2c", "/usr/bin/oak2c-debug",
       "/usr/include/oak"};

  // Global used by the recursive function to ensure no
  // double-counting
  std::set<std::filesystem::path> visited;

  // Recursive file/dir size getter
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

  // Formats and prints a byte size
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

/// A small wrapper that asks for confirmation before
/// uninstalling. This can safely be run after object deletion
/// (e.g. CLI closing)
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

void OakCompiler::uninstall_acorn() noexcept {
  debug_print();
  std::atexit(internal_uninstall);
}

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
         {"ABOUT!", "AUTHOR!", "EMAIL!", "LICENSE!", "SOURCE!",
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

void OakCompiler::operator()() {
  debug_print();
  bool did_fail = false;
  if (settings.is_compile()) {
    if (settings.debug) {
      settings.ostream
          << "Compiler detected compilation mode.\n";
    }

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
    if (settings.debug) {
      settings.ostream << "Compiler detected testing mode.\n";
    }

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

  if (settings.debug) {
    settings.ostream << "Source:     " << csettings.entry_point
                     << "\nTarget:     " << translated_file
                     << "\nObject:     " << compiled_file
                     << "\nExecutable: " << linked_file << "\n";
  }

  if (settings.debug) {
    settings.ostream << "Parsing " << csettings.entry_point
                     << "\n";
  }

  try {
    p.do_file(csettings.entry_point, csettings.entry_point);
  } catch (OutOfPPPLError &e) {
    throw OutOfPPPLError(
        "Error occurred while loading entry point " +
        csettings.entry_point.string() + ":\n" + e.what());
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
    if (settings.debug) {
      settings.ostream << "Translating "
                       << csettings.entry_point << " to "
                       << translated_file << "\n";
    }

    // Translate Oak token stream to C directly to file
    std::ofstream target_file(translated_file);
    translate(target_file);
  }

  // If requested, call compiler
  if (csettings.mode >=
      Settings::CompileSettings::TRANSLATE_AND_COMPILE) {
    if (settings.debug) {
      settings.ostream << "Compiling " << translated_file
                       << " to " << compiled_file << "\n";
    }

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
    if (settings.debug) {
      settings.ostream << "Linking " << compiled_file << " to "
                       << linked_file << "\n";
    }

    bool saw_main = false;
    const auto res = p.scope_manager.get("main");
    if (res.has_value() &&
        std::holds_alternative<std::list<std::variant<
            FnInfo, std::shared_ptr<TemplateInfo>>>>(
            res.value())) {
      for (const auto &def : std::get<std::list<std::variant<
               FnInfo, std::shared_ptr<TemplateInfo>>>>(
               res.value())) {
        if (std::holds_alternative<FnInfo>(def)) {
          const auto candidate = std::get<FnInfo>(def);
          if (!candidate.tags.contains("casual") &&
              candidate.tags.at("file") ==
                  csettings.entry_point) {
            saw_main = true;
            break;
          }
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
    if (settings.debug) {
      settings.ostream << "Executing " << compiled_file << "\n";
    }

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
