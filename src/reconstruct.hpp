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

#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "enums.hpp"
#include "lexer.hpp"
#include "mangler.hpp"
#include "rules.hpp"
#include "symbol_table.hpp"
#include "type_builder.hpp"

const static std::string OAK_HEADER_PATH = "/usr/include/oak/std_oak_header.h";

// Removes illegal characters
std::string purifyStr(const std::string &What);

// Reconstruct the existing symbol table into C++
void reconstruct(const std::string &Name, std::stringstream &header, std::stringstream &body);

// Contains all the atomic types (ints, floats, bools, etc)
extern std::map<std::string, unsigned long long> atomics;

// Save reconstructed files and return compilation command
// Return pair<sstream, sstream>{header, body};
std::pair<std::string, std::string> save(const std::stringstream &header, const std::stringstream &body,
                                         const std::string &Name);

// Call reconstruct and save, without fiddling with stringstreams
// returns headerName, bodyName
std::pair<std::string, std::string> reconstructAndSave(const std::string &Name);

// Return the C++ format-version of a type, to be followed by symbol name
std::string toStrC(const Type *What, const std::string &Name = "", const unsigned int &pos = 0);

// Return the C++ format-version of a function, INCLUDING symbol name
std::string toStrCFunction(const Type *What, const std::string &Name, const unsigned int &pos = 0);

// Other type of C++ function; IE bool (*what)(const bool &What);
std::string toStrCFunctionRef(const Type *What, const std::string &Name, const unsigned int &pos = 0);

std::string enumToC(const std::string &name);

#endif
