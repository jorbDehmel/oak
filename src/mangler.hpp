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

// Mangle an enumeration, give its generic substitutions
std::string mangleEnum(const std::string &name,
                       const std::vector<std::vector<std::string>> &generics = std::vector<std::vector<std::string>>());

// Fully qualify a type in a canonical way.
std::string mangleType(const Type &type);

// Fully qualify a symbol in a canonical way.
std::string mangleSymb(const std::string &name, Type &type);

// Fully qualify a symbol in a canonical way.
std::string mangleSymb(const std::string &name, const std::string &typeStr);

#endif
