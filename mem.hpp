/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author

Heap operations for Oak
To be called by sequencing
*/

#ifndef MEM_HPP
#define MEM_HPP

#include "symbol_table.hpp"

// Returns the sequence which allocates an array of size num and type type.
sequence getAllocSequence(Type &type, const std::string &name, const std::string &num = "1");

// Returns the sequence which frees the referenced memory
sequence getFreeSequence(const std::string &name, const bool &isArr = false);

#endif
