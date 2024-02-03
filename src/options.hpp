/*
Defines the AcornSettings struct, which contains all non-const
globals needed by the compiler. This file also contains all
constant globals needed by the compiler.

IMPORTANT: Some subsystems use local caches!

Jordan Dehmel, 2024
jdehmel@outlook.com
*/

#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "lexer.hpp"
#include "rule_structs.hpp"
#include "symbol_table.hpp"
#include "type_builder.hpp"

namespace fs = std::filesystem;

// Info: global constants
const static std::string VERSION = "0.4.21";
const static std::string LICENSE = "GPLv3";
const static std::string INFO = "jdehmel@outlook.com";

// The main include directory for oak
const static std::string OAK_DIR_PATH = "/usr/include/oak/";

const static std::string COMPILED_PATH = ".oak_build/";
const static std::string COMPILER_COMMAND = "acorn";

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

const static std::string PACKAGE_TEMP_LOCATION = "/tmp/oak_packages/";
const static std::string PACKAGE_INCLUDE_PATH = "/usr/include/oak/";
const static std::string INFO_FILE = "oak_package_info.txt";
const static std::string PACKAGES_LIST_PATH = "/usr/include/oak/packages_list.txt";
const static std::string CLONE_COMMAND = "git clone ";

// Contains all the atomic types (ints, floats, bools, etc)
const static std::map<std::string, unsigned long long> atomics = {
    {"u8", 1},   {"i8", 1},     {"u16", 2},   {"i16", 2},   {"u32", 4},
    {"i32", 4},  {"u64", 8},    {"i64", 8},   {"u128", 16}, {"i128", 16},
    {"f32", 4},  {"f64", 8},    {"f128", 16}, {"bool", 1},  {"str", sizeof(void *)},
    {"void", 1}, {"struct", 1}, {"enum", 1}};

const static std::string OAK_HEADER_PATH = "/usr/include/oak/std_oak_header.h";

// Settings struct
struct AcornSettings
{
    std::set<std::string> objects, cflags;
    std::map<std::string, std::string> preprocDefines;
    std::vector<unsigned long long> phaseTimes;

    // Maps used filepaths to their post-compiler macro compile
    // times, if debug is on.
    // Otherwise, maps them to zero.
    std::map<fs::path, unsigned long long> visitedFiles;

    bool doLink = true;
    bool compile = true;
    bool debug = false;
    bool alwaysDump = false;
    bool ignoreSyntaxErrors = false;
    bool isMacroCall = false;
    bool noSave = false;
    bool eraseTemp = false;
    bool prettify = false;
    bool execute = false;
    bool test = false;
    bool manual = false;
    bool testFail = false;

    std::map<std::string, StructLookupData> structData;
    std::map<std::string, EnumLookupData> enumData;
    std::vector<std::string> structOrder;
    MultiSymbolTable table;

    unsigned long long int curLine = 1;
    std::string curFile = "NULL";
    std::vector<Token> curLineSymbols;

    std::vector<std::string> activeRules;
    std::vector<std::string> dialectRules;
    bool dialectLock = false;
    std::map<std::string, std::vector<std::string>> bundles;
    bool doRuleLogFile = false;
    std::ofstream ruleLogFile;

    std::map<std::string, Rule> rules;
    std::map<std::string, void (*)(std::vector<Token> &, int &, Rule &, AcornSettings &)> engines;

    // The pre-inserted ones are used by the compiler- Not literal macros
    std::set<std::string> compiled = {"include!",  "link!",     "package!",  "alloc!",    "free!",
                                      "free_arr!", "new_rule!", "use_rule!", "rem_rule!", "bundle_rule!",
                                      "erase!",    "c_print!",  "c_warn!",   "c_panic!",  "type!",
                                      "size!",     "ptrcpy!",   "ptrarr!",   "raw_c!"};
    std::map<std::string, std::string> macros;
    std::map<std::string, std::string> macroSourceFiles;
};

#endif
