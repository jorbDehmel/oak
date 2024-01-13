/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author

Resources for getting file sizes. Used by the acorn
compiler frontend.
*/

#ifndef SIZER_HPP
#define SIZER_HPP

#include "symbol_table.hpp"

#include <fstream>
#include <string>

// Use system calls to get the size in kilobytes of a given filepath
unsigned long long int getSize(const std::string &FilePath);

// Turn the output of getSize into a human-readable string
std::string humanReadable(const unsigned long long int &Size);

#endif
