/*
Jordan Dehmel, 2023 - present
jdehmel@outlook.com

Mangles symbol names and types into unambiguous C names.
*/

#ifndef MANGLER_HPP
#define MANGLER_HPP

#include "type_builder.hpp"
#include <string>
#include <vector>

// Used for mangling and resolving generics pre-sequencing
std::string mangle(const std::vector<std::string> &what);

// Mangle a struct, given its generic substitutions
std::string mangleStruct(const std::string &name, const std::vector<std::vector<std::string>> &generics =
                                                      std::vector<std::vector<std::string>>());

// Mangle an enumeration, givne its generic substitutions
std::string mangleEnum(const std::string &name,
                       const std::vector<std::vector<std::string>> &generics = std::vector<std::vector<std::string>>());

// Used for translation-time variable uniqueness in the face
// of overloaded names. Not immediately useful, but necessary
// for future lower-level (C or LLVM IR) translations.
std::string mangleType(const Type &type);
std::string mangleSymb(const std::string &name, Type &type);
std::string mangleSymb(const std::string &name, const std::string &typeStr);

#endif
