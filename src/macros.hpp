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

#include "options.hpp"
#include "reconstruct.hpp"
#include "symbol_table.hpp"
#include "tags.hpp"

// USES SYSTEM CALLS. Ensures a given macro exists, then calls
// it with the given arguments. If debug is true, specifies such
// in the call.
std::string callMacro(const std::string &Name, const std::vector<std::string> &Args, AcornSettings &settings);

// USES SYSTEM CALLS. If source code exists for `name` but it is
// not yet compiled, calls another instance of the compiler on
// its contents. Note: The sub-instance of the compiler runs in
// a special macro-compilation mode which inhibits its outputs
// and syntax constraints.
void compileMacro(const std::string &Name, AcornSettings &settings);

// Returns the given file's last modification time, in seconds
// after the epoch.
long long getFileLastModification(const std::string &filepath);

// Returns true if the source file is newer than the destination
// one OR if either file is nonexistant.
bool isSourceNewer(const std::string &source, const std::string &dest);

// Returns true if and only if the given macro already exists.
bool hasMacro(const std::string &name, AcornSettings &settings) noexcept;

// Adds the given macro definition into the internal macro lookup table.
void addMacro(const std::string &name, const std::string &contents, AcornSettings &settings);

#endif
