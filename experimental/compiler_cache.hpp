/*
Methods to save and load the entire state of the
compiler, including all symbols, structs, data,
used files, etc. all in one file.

Jordan Dehmel, 2023
jdehmel@outlook.com
*/

#ifndef COMPILER_CACHE_HPP
#define COMPILER_CACHE_HPP

#include "acorn_resources.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cassert>
using namespace std;

class cache_error : public runtime_error
{
public:
    cache_error(const string &what) : runtime_error(what) {}
};

// Save the entire state of the compiler to a file
void saveCompilerCache(const string &where);

// Load the entire state of the compiler from a file
// WARNING: Completely overwrites all runtime data in
// this translation unit.
void loadCompilerCache(const string &where);

#endif
