/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author

If a sequence's type is the NULL atomic, that means
that it has no hanging type. It does NOT mean it is
an empty sequence; Consider semicolon-terminated
lines.

Handles sequencing in Oak:
Basically everything occurring within a function

A sequence can contain other sequences, as long as
they combine to create exactly one mega-sequence.
This is the only part of the language in which
AST structures might be useful.

A sequence takes a number of other sequences,
each of which have a return type. These sequence
types are compile-time enforced.

All functions have exactly one attached sequence.
Function signatures will be handles by the
non-sequential parser.
*/

#ifndef SEQUENCE_HPP
#define SEQUENCE_HPP

#include "sequence_resources.hpp"

// Externally useful functions

// Creates a sequence from a lexed string.
// Return type is deduced naturally from the contents.
// Can throw sequencing errors.
ASTNode createSequence(const std::vector<Token> &from);

// Get the return type; Set as a global
Type resolveFunction(const std::vector<Token> &what, int &start, std::string &c);

#endif
