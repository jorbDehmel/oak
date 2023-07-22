/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#ifndef RECONSTRUCT_HPP
#define RECONSTRUCT_HPP

/*
Reconstruct an abstract symbol tree and symbol
table into compilable C++. This should be language
agnostic with respect to input (as much as possible).
*/

#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include "lexer.hpp"
#include "type-builder.hpp"
#include "sequence.hpp"
#include "symbol-table.hpp"

#define OAK_HEADER_PATH "/usr/include/std_oak_header.hpp"

using namespace std;

// Removes illegal characters
string purifyStr(const string &What);

// Reconstruct the existing symbol table into C++
void reconstruct(const string &Name,
                 stringstream &header,
                 stringstream &body);

// Save reconstructed files and return compilation command
// Return pair<sstream, sstream>{header, body};
pair<string, string> save(const stringstream &header, const stringstream &body, const string &Name);

// Call reconstruct and save, without fiddling with stringstreams
// returns headerName, bodyName
pair<string, string> reconstructAndSave(const string &Name);

// Return the C++ format-version of a type, to be followed by symbol name
string toStrC(Type *What);

// Return the C++ format-version of a function, INCLUDING symbol name
string toStrCFunction(Type *What, const string &Name);

// Other type of C++ function; IE bool (*what)(const bool &What);
string toStrCFunctionRef(Type *What, const string &Name);

// Dump data to file
void dump(const vector<string> &Lexed, const string &Where, const string &FileName, const int &Line);

#endif
