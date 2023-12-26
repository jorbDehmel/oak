/*
Jordan Dehmel, 2023
jdehmel@outlook.com

Mangles symbol names and types into unambiguous C names.
*/

#ifndef MANGLER_HPP
#define MANGLER_HPP

#include "type_builder.hpp"
#include <string>
#include <vector>
using namespace std;

// Used for mangling and resolving generics pre-sequencing
string mangle(const vector<string> &what);

// Mangle a struct, given its generic substitutions
string mangleStruct(const string &name, const vector<vector<string>> &generics = vector<vector<string>>());

// Mangle an enumeration, givne its generic substitutions
string mangleEnum(const string &name, const vector<vector<string>> &generics = vector<vector<string>>());

// Used for translation-time variable uniqueness in the face
// of overloaded names. Not immediately useful, but necessary
// for future lower-level (C or LLVM IR) translations.
string mangleType(const Type &type);
string mangleSymb(const string &name, Type &type);
string mangleSymb(const string &name, const string &typeStr);

#endif
