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

// Compiler name: Oakc -> "Oak Seed" -> acorn

extern set<string> compiled;
extern map<string, string> macros;

// USES SYSTEM CALLS
string callMacro(const string &Name, const vector<string> &Args, bool debug);
void compileMacro(const string &Name, bool debug);

#endif
