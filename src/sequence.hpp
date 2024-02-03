/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author

Handles sequencing in Oak:
Basically everything occurring within a function
*/

#ifndef SEQUENCE_HPP
#define SEQUENCE_HPP

#include "options.hpp"
#include "sequence_resources.hpp"
#include "symbol_table.hpp"
#include "tags.hpp"
#include "type_builder.hpp"

// Creates a sequence from a lexed string.
// Return type is deduced naturally from the contents.
// Can throw sequencing errors.
ASTNode createSequence(const std::vector<Token> &from, AcornSettings &settings);

// Get the return type; Set as a global
Type resolveFunction(const std::vector<Token> &What, int &start, std::string &c, AcornSettings &settings);

#endif
