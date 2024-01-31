/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author

Handles macro extraction, compilation, and replacement for Oak
source code.
*/

#ifndef MACROS_HPP
#define MACROS_HPP

#include <chrono>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "reconstruct.hpp"
#include "symbol_table.hpp"
#include "tags.hpp"

// Extern non-constant globals
extern std::set<std::string> compiled;
extern std::map<std::string, std::string> macros;
extern std::map<std::string, std::string> macroSourceFiles;

const static std::string COMPILED_PATH = ".oak_build/";
const static std::string COMPILER_COMMAND = "acorn";

// USES SYSTEM CALLS. Ensures a given macro exists, then calls
// it with the given arguments. If debug is true, specifies such
// in the call.
std::string callMacro(const std::string &name, const std::vector<std::string> &args, bool debug);

// USES SYSTEM CALLS. If source code exists for `name` but it is
// not yet compiled, calls another instance of the compiler on
// its contents. Note: The sub-instance of the compiler runs in
// a special macro-compilation mode which inhibits its outputs
// and syntax constraints.
void compileMacro(const std::string &name, bool debug);

// Returns the given file's last modification time, in seconds
// after the epoch.
long long getFileLastModification(const std::string &filepath);

// Returns true if the source file is newer than the destination
// one OR if either file is nonexistant.
bool isSourceNewer(const std::string &source, const std::string &dest);

#endif
