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
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "reconstruct.hpp"
#include "symbol_table.hpp"

const static std::string COMPILED_PATH = ".oak_build/";
const static std::string COMPILER_COMMAND = "acorn";

extern std::set<std::string> compiled;
extern std::map<std::string, std::string> macros;
extern std::map<std::string, std::string> macroSourceFiles;

// USES SYSTEM CALLS
std::string callMacro(const std::string &Name, const std::vector<std::string> &Args, bool debug);
void compileMacro(const std::string &Name, bool debug);

// Misnomer: This returns the create time in seconds after epoch
// Is costly the first time, but uses caching
long long getAgeOfFile(const std::string &filepath);

// Returns true if the source file is newer than the destination one
// OR if either file is nonexistant
// Is costly the first time, but uses caching
bool isSourceNewer(const std::string &source, const std::string &dest);

#endif
