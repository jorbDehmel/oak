/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author

Reconstruct an abstract symbol tree and symbol
table into compilable C. This should be language
agnostic with respect to input (as much as possible).
*/

#ifndef RECONSTRUCT_HPP
#define RECONSTRUCT_HPP

#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "lexer.hpp"
#include "mangler.hpp"
#include "options.hpp"
#include "rules.hpp"
#include "symbol_table.hpp"
#include "type_builder.hpp"

// Removes illegal characters.
std::string purifyStr(const std::string &what);

// Reconstruct the existing symbol table into C.
void reconstruct(const std::string &Name, AcornSettings &settings, std::stringstream &header, std::stringstream &body);

// Save reconstructed files and return compilation command
// Return pair<sstream, sstream>{header, body};
std::pair<std::string, std::string> save(const std::stringstream &header, const std::stringstream &body,
                                         const std::string &name);

// Call reconstruct and save, without fiddling with
// stringstreams. Returns headerName, bodyName.
std::pair<std::string, std::string> reconstructAndSave(const std::string &Name, AcornSettings &settings);

// Return the C format-version of a type, to be followed by
// symbol name.
std::string toStrC(const Type *what, const std::string &name = "", const unsigned int &pos = 0);

// Return the C format-version of a function, INCLUDING symbol
// name.
std::string toStrCFunction(const Type *what, const std::string &name, const unsigned int &pos = 0);

// Other type of C function; IE bool (*what)(const bool &What);.
std::string toStrCFunctionRef(const Type *what, const std::string &name, const unsigned int &pos = 0);

// Return the C version of an Oak `enum`. These are heavily
// involved, more so than structs or variables.
std::string enumToC(const std::string &name, AcornSettings &settings);

#endif
