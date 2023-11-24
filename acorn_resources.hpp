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
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

// #include "compiler_cache.hpp"
#include "document.hpp"
#include "lexer.hpp"
#include "macros.hpp"
#include "op-sub.hpp"
#include "packages.hpp"
#include "reconstruct.hpp"
#include "rules.hpp"
#include "sequence.hpp"
#include "symbol-table.hpp"
#include "type-builder.hpp"

#include "sizer.hpp"
#include "tags.hpp"

using namespace std;

#define VERSION "0.2.6"
#define LICENSE "GPLv3"
#define INFO "jdehmel@outlook.com"

#define OAK_DIR_PATH "/usr/include/oak/"

#define C_COMPILER "clang" // This should be a C compiler- like clang or gcc
#define LINKER "clang++"   // This should be a C++ compiler- like clang++ or g++

#define MAX_CACHE_KB 5000

/*
Remaining options:
-bfgjkyz
-BCEFGHIJKLNOPQVWXYZ
*/

const string helpText = "Acorn - Oak Standard Translator\n"
                        "For the Oak programming language\n"
                        "Translates .oak files to .cpp.\n\n"
                        "Option | Verbose     | Purpose\n"
                        "-------|-------------|-------------------------------\n"
                        " -a    |             | Update acorn\n"
                        " -A    |             | Uninstall acorn\n"
                        " -c    | --compile   | Produce object files\n"
                        " -d    | --debug     | Toggle debug mode\n"
                        " -D    | --dialect   | Uses a dialect file\n"
                        " -e    | --clean     | Toggle erasure (default on)\n"
                        " -g    | --exe_debug | Use LLVM debug flag\n"
                        " -h    | --help      | Show this\n"
                        " -i    | --install   | Install a package\n"
                        " -l    | --link      | Produce executables\n"
                        " -m    | --manual    | Produce a .md doc\n"
                        " -M    |             | Used for macros\n"
                        " -n    | --no_save   | Produce nothing\n"
                        " -o    | --output    | Set the output file\n"
                        " -O    | --optimize  | Use LLVM optimization O3\n"
                        " -q    | --quit      | Quit immediately\n"
                        " -r    | --reinstall | Reinstall a package\n"
                        " -R    | --remove    | Uninstalls a package\n"
                        " -s    | --size      | Show Oak disk usage\n"
                        " -S    | --install   | Install a package\n"
                        " -t    | --translate | Produce C++ files\n"
                        " -T    | --time      | Output time usage\n"
                        " -u    | --dump      | Save dump files\n"
                        " -U    |             | Save rule log files\n"
                        " -v    | --version   | Show version\n"
                        " -w    | --new       | Create a new package\n"
                        " -x    | --syntax    | Ignore syntax errors\n";

extern bool debug, compile, doLink, alwaysDump, manual, ignoreSyntaxErrors, isMacroCall, timeAnalysis;
extern set<string> visitedFiles, cppSources, objects, cflags;
extern map<string, string> preprocDefines;
extern vector<unsigned long long> phaseTimes;

// Prints the cumulative disk usage of Oak (in human-readable)
void getDiskUsage();

void doFile(const string &From);

void makePackage(const string &Name);
void ensureSyntax(const string &text, const bool &fatal = true);

#endif
