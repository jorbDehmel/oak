/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

// Stack operations for Oak
// To be called by sequencing when in dirty mode

#ifndef MEM_HPP
#define MEM_HPP

#include "symbol-table.hpp"
#include "reconstruct.hpp"
using namespace std;

// Returns the sequence which allocates an array of size num and type type.
sequence getAllocSequence(Type &type, const string &name, const int &num = 1);

// Returns the sequence which frees the referenced memory
sequence getFreeSequence(const string &name, const bool &isArr = false);

#endif
