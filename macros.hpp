/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

// NOTE: This file is extremely dependant on the Acorn
// translator!

#ifndef MACROS_HPP
#define MACROS_HPP

#include <map>
#include <string>
#include <fstream>
#include <set>
#include <chrono>
#include <vector>

#include "symbol-table.hpp"
#include "reconstruct.hpp"

using namespace std;

#define COMPILED_PATH ".oak_build/"
#define COMPILER_COMMAND "acorn"

extern set<string> compiled;
extern map<string, string> macros;
extern map<string, string> macroSourceFiles;

// USES SYSTEM CALLS
string callMacro(const string &Name, const vector<string> &Args, bool debug);
void compileMacro(const string &Name, bool debug);

// Misnomer: This returns the create time in seconds after epoch
// Is costly the first time, but uses caching
long long getAgeOfFile(const string &filepath);

// Returns true if the source file is newer than the destination one
// OR if either file is nonexistant
// Is costly the first time, but uses caching
bool isSourceNewer(const string &source, const string &dest);

#endif
