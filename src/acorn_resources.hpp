/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#ifndef ACORN_RESOURCES_HPP
#define ACORN_RESOURCES_HPP

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "document.hpp"
#include "lexer.hpp"
#include "macros.hpp"
#include "op_sub.hpp"
#include "packages.hpp"
#include "reconstruct.hpp"
#include "rules.hpp"
#include "sequence.hpp"
#include "sizer.hpp"
#include "symbol_table.hpp"
#include "type_builder.hpp"

#include "tags.hpp"

// External settings (non-constant)
extern bool debug, doLink, alwaysDump, ignoreSyntaxErrors, isMacroCall;
extern std::set<std::string> objects, cflags;
extern std::map<std::string, std::string> preprocDefines;
extern std::vector<unsigned long long> phaseTimes;

// Maps used filepaths to their post-compiler macro compile
// times, if debug is on.
// Otherwise, maps them to zero.
extern std::map<std::filesystem::path, unsigned long long> visitedFiles;

// Info: global constants
const static std::string VERSION = "0.4.20";
const static std::string LICENSE = "GPLv3";
const static std::string INFO = "jdehmel@outlook.com";

// The main include directory for oak
const static std::string OAK_DIR_PATH = "/usr/include/oak/";

// Underlying compilers and linkers allowing Acorn to function
const static std::string C_COMPILER = "clang"; // This should be a C compiler- like clang or gcc
const static std::string LINKER = "clang++";   // This should be a C++ compiler- like clang++ or g++
const static std::string PRETTIFIER =
    "clang-format --style=Microsoft -i "; // This should be a formatter (like clang-format)

// Max allowable size of .acorn_build in kilobytes
const static unsigned long long MAX_CACHE_KB = 2000;

const static std::string helpText = "Acorn - Oak Standard Translator\n"
                                    "For the Oak programming language\n"
                                    "Translates and compiles .oak files\n\n"
                                    "Option | Verbose     | Purpose\n"
                                    "-------|-------------|-------------------------------\n"
                                    " -a    |             | Update acorn\n"
                                    " -A    |             | Uninstall acorn\n"
                                    " -c    | --compile   | Produce object files\n"
                                    " -d    | --debug     | Toggle debug mode\n"
                                    " -D    | --dialect   | Uses a dialect file\n"
                                    " -e    | --clean     | Toggle erasure (default off)\n"
                                    " -E    | --execute   | Run executable when done\n"
                                    " -g    | --exe_debug | Use LLVM debug flag\n"
                                    " -h    | --help      | Show this\n"
                                    " -i    | --install   | Install a package\n"
                                    " -l    | --link      | Produce executables\n"
                                    " -m    | --manual    | Produce a .md doc\n"
                                    " -M    |             | Used for macros\n"
                                    " -n    | --no_save   | Produce nothing\n"
                                    " -o    | --output    | Set the output file\n"
                                    " -O    | --optimize  | Use LLVM optimization O3\n"
                                    " -p    | --prettify  | Use clang-format on output C\n"
                                    " -q    | --quit      | Quit immediately\n"
                                    " -Q    | --query     | Query an installed package\n"
                                    " -r    | --reinstall | Reinstall a package\n"
                                    " -R    | --remove    | Uninstalls a package\n"
                                    " -s    | --size      | Show Oak disk usage\n"
                                    " -S    | --install   | Install a package\n"
                                    " -t    | --translate | Produce C++ files\n"
                                    " -T    | --test      | Compile and run tests/*.oak\n"
                                    " -u    | --dump      | Save dump files\n"
                                    " -U    |             | Save rule log files\n"
                                    " -v    | --version   | Show version\n"
                                    " -w    | --new       | Create a new package\n"
                                    " -x    | --syntax    | Ignore syntax errors\n";

// Prints the cumulative disk usage of Oak (human-readable).
void getDiskUsage();

// The most important function in the compiler; Actually loads,
// lexes, does rules, does AST-analysis and reconstruction. This
// is the core of the compiler.
void doFile(const std::string &filepath);

// Creates a factory-settings package with the given name in the
// current directory. Sets up everything you need.
void makePackage(const std::string &name);

// Ensures that the text passed adheres to basic style-based
// syntactical requirements (IE line width limits, matching
// parenthesis). If fatal, does not allow recovery. Otherwise,
// does.
void ensureSyntax(const std::string &text, const bool &fatal = true);

#endif
