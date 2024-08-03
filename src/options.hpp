/*
Defines the AcornSettings struct, which contains all non-const
globals needed by the compiler. This file also contains all
constant globals needed by the compiler.

File structure:
```
    <lexer dependencies>
    |
    lexer.hpp
    |
    oakc_structs.hpp
    |
    options.hpp   <---
    |
    oakc_fns.hpp
    |
    <compiler frontend>
```

Jordan Dehmel, 2024
jdehmel@outlook.com
*/

#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#include "lexer.hpp"
#include "oakc_structs.hpp"
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// The current version of Oak.
const static std::string VERSION = "0.7.1";

// The license that this version of Oak uses.
const static std::string LICENSE = "GPLv3";

// Email(s) to contact with any questions.
const static std::string INFO = "jdehmel@outlook.com";

// The main include directory for oak.
// This is where Oak will install and look for for packages.
const static std::string OAK_DIR_PATH = "/usr/include/oak/";

// The name of the temporary folder to use for building.
const static std::string COMPILED_PATH = ".oak_build/";

// The command to use to compile Oak files (for macros).
const static std::string COMPILER_COMMAND = "acorn";

// The `C` (not `C++`) compiler which will turn the translated
// `C` files into object files.
const static std::string C_COMPILER = "clang";

// The `C++` (not `C`) compiler/linker which will turn the
// collected object files into an executable.
const static std::string LINKER = "clang++";

// A command-line `C` formatting tool which will make the
// target `C` prettier when required.
const static std::string PRETTIFIER =
    "clang-format --style=Microsoft -i ";

// Max allowable size of .acorn_build in kilobytes
const static unsigned long long MAX_CACHE_KB = 2000;

// The text to be shown upon `-h` or `--help`
const static std::string HELP_TEXT =
    "Acorn - Oak Standard Translator\n"
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
    " -M    |             | Used for macro compilation\n"
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
    " -t    | --translate | Produce C files\n"
    " -T    | --test      | Compile and run tests/*.oak\n"
    " -u    | --dump      | Save dump files\n"
    " -U    |             | Save rule log files\n"
    " -v    | --version   | Show version\n"
    " -w    | --new       | Create a new package\n"
    " -x    | --syntax    | Ignore syntax errors\n";

// A temporary place to store packages during installation.
// These will be deleted after install.
const static std::string PACKAGE_TEMP_LOCATION =
    "/tmp/oak_packages/";

// Where to look for packages.
const static std::string PACKAGE_INCLUDE_PATH = OAK_DIR_PATH;

// The file name to use as the package information file.
// Will attempt to use .json, and .txt if fails.
const static std::string INFO_FILE = "oak_package_info";

// A locally stored list of all official packages.
const static std::string PACKAGES_LIST_PATH =
    "/usr/include/oak/packages_list.txt";

// A command to clone a `git` repository locally.
const static std::string CLONE_COMMAND = "git clone ";

// Contains all the atomic types (ints, floats, bools, etc)
const static std::set<std::string> ATOMICS = {
    "u8",   "i8",   "u16",  "i16",  "u32",    "i32",
    "u64",  "i64",  "u128", "i128", "f32",    "f64",
    "f128", "bool", "str",  "void", "struct", "enum"};

// Where to look for the standard `oak` header file. This is
// included at the top of all target `C` files.
const static std::string OAK_HEADER_PATH =
    "/usr/include/oak/std_oak_header.h";

// Used for keyword collision avoidance. All `Oak` keywords.
const static std::set<std::string> OAK_KEYWORDS = {
    "if",    "else",  "let",    "case", "default",
    "match", "while", "return", "pre",  "post"};

// Used for keyword collision avoidance. "All" `C` keywords.
const static std::set<std::string> C_KEYWORDS = {
    "alignas",
    "alignof",
    "auto",
    "break",
    "case",
    "const",
    "constexpr",
    "continue",
    "default",
    "do",
    "else",
    "extern",
    "for",
    "goto",
    "if",
    "inline",
    "nullptr",
    "register",
    "restrict",
    "return",
    "signed",
    "sizeof",
    "static",
    "static_assert",
    "switch",
    "thread_local",
    "typedef",
    "typeof",
    "typeof_unqual",
    "union",
    "unsigned",
    "volatile",
    "while",
    "_Alignas",
    "_Alignof",
    "_Atomic",
    "_BitInt",
    "_Bool",
    "_Complex",
    "_Decimal128",
    "_Decimal32",
    "_Decimal64",
    "_Generic",
    "_Imaginary",
    "_Noreturn",
    "_Static_assert",
    "_Thread_local",
    "_Pragma",
    "asm",
    "fortran",
    "int",
    "char",
    "float",
    "double",
    "long"};

// The set of all default compiler macros.
const static std::set<std::string> COMPILER_MACROS = {
    "include!", "package!", "link!", "flag!"};

// Constants used for markdown conversion.
const std::string H_LINE = "---";
const std::string HEADER_START = "# ";
const std::string FILE_START = "## ";
const std::string FN_START = "### ";

// Operations set for operator substitution.
const static std::set<std::string> OPERATORS = {
    "+",  "-",  "/",  "*",  "%",  "=",  "-=", "+=", "/=", "*=",
    "%=", "&=", "|=", "<<", ">>", "&&", "||", "<",  ">",  "==",
    "!=", "<=", ">=", "&",  "|",  ",",  "(",  ")",  ";",  "++",
    "--", "!",  "{",  "}",  "[",  "]",  "[]", "^",  "@"};

// Used for implicit casting.
const std::map<std::string, unsigned char> INT_LITERALS = {
    {"u8", 0},   {"i8", 0},  {"u16", 1}, {"i16", 1},
    {"u32", 2},  {"i32", 2}, {"u64", 3}, {"i64", 3},
    {"u128", 4}, {"i128", 4}};
const std::map<std::string, unsigned char> FLOAT_LITERALS = {
    {"f32", 0}, {"f64", 1}, {"f128", 2}};

// Settings structure. Holds all information which used to be
// non-const globals.
struct AcornSettings
{
    // Holds information about the file where useful. Reset each
    // file transition.
    std::map<std::string, std::map<std::string, std::string>>
        file_tags;

    // The current depth of createSequence
    unsigned long long int depth = 0;

    // The file whose analysis began this translation unit.
    fs::path entryPoint;

    // Object files to include when linking.
    std::set<std::string> objects;

    // Flags to add onto the compilation and linking commands.
    std::set<std::string> cflags;

    // A map of all preprocessor definitions- similar to C's
    // `#define`.
    std::map<std::string, std::string> preprocDefines;

    // A vector for keeping track of the time spent in each
    // phase of compilation. Only used if debug is on.
    std::vector<unsigned long long> phaseTimes;

    // Maps used filepaths to their post-compiler macro compile
    // times, if debug is on. Otherwise, maps them to zero.
    std::map<fs::path, unsigned long long> visitedFiles;

    // Whether or not to do linkage.
    bool doLink = true;

    // Whether or not to compile target `C` files.
    bool compile = true;

    // Whether or not to show additional debugging stats.
    bool debug = false;

    // Whether or not to save dump files when there is no error.
    bool alwaysDump = false;

    // Whether or not to ignore syntax errors.
    bool ignoreSyntaxErrors = false;

    // Whether or not this is a macro compilation call. Should
    // not be used as a flag by users.
    bool isMacroCall = false;

    // If true, does not produce any output files.
    bool noSave = false;

    // Whether or not to erase the temporary build folder.
    bool eraseTemp = false;

    // Whether or not to call a formatter on the produces `C`
    // code.
    bool prettify = false;

    // If true, executes the produces executable after
    // compilation.
    bool execute = false;

    // If true, runs a test suite. See documentation for more
    // details on test suite.
    bool test = false;

    // If true, generates `.md` files from the commenting of
    // the source code.
    bool manual = false;

    // If true, halts after the first failure in a test suite.
    bool testFail = false;

    // Maps the name of an `Oak` struct to its data.
    std::map<std::string, StructLookupData> structData;

    // Maps the name of an `Oak` enum to its data.
    std::map<std::string, EnumLookupData> enumData;

    // Lists the order in which structs and enums were created.
    std::list<std::string> structOrder;

    // The symbol table. Holds every instance of a type within
    // the program.
    MultiSymbolTable table;

    // The current line.
    uint64_t curLine = 1, curCol = 0;

    // The current file.
    fs::path curFile = "NULL";

    // The tokens in the current line. Used for debugging.
    std::list<Token> curLineSymbols;

    std::vector<std::string> activeRules;
    std::vector<std::string> dialectRules;
    bool dialectLock = false;
    std::map<std::string, std::list<std::string>> bundles;
    bool doRuleLogFile = false;
    std::ofstream ruleLogFile;

    std::map<std::string, Rule> rules;
    std::map<std::string, void (*)(std::list<Token> &,
                                   std::list<Token>::iterator &,
                                   Rule &, AcornSettings &)>
        engines;

    // The set of all macros for which an executable version
    // already exists. If a macro is not present here, it will
    // need to be compiled before it is used.
    // The pre-inserted macros are compiler macros.
    std::set<std::string> compiled = {
        "include!",  "link!",        "package!",  "alloc!",
        "free!",     "free_arr!",    "new_rule!", "use_rule!",
        "rem_rule!", "bundle_rule!", "erase!",    "c_print!",
        "c_warn!",   "c_panic!",     "type!",     "size!",
        "ptrcpy!",   "ptrarr!",      "raw_c!"};

    // The set of all macros. Maps name to source code.
    std::map<std::string, std::string> macros;

    // Maps the name of a macro to the file it came from.
    std::map<std::string, std::string> macroSourceFiles;

    // The set of all existing templates for generics.
    std::map<std::string, std::list<GenericInfo>> generics;

    // The current return type if inside a function. If not
    // currently processing a function, is nullType.
    Type currentReturnType = nullType;

    // Used for debugging.
    std::string debugTreePrefix = "";

    // Used for match statements.
    std::string prevMatchTypeStr = "NULL";

    // Caches.
    std::map<std::string, long long> ageCache;
    std::map<std::pair<unsigned long long, std::string>,
             std::string>
        toStrCTypeCache;
    std::map<std::string, std::string> toStrCEnumCache;
    std::map<unsigned long long, Type> getReturnTypeCache;
    std::map<unsigned long long,
             std::list<std::pair<std::string, Type>>>
        cache;

    // The local command to install a system package.
    std::string installCommand = "";

    std::map<std::string, PackageInfo> packages;

}; // struct AcornSettings

#endif // OPTIONS_HPP
