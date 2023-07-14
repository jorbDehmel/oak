/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#ifndef SIZER_HPP
#define SIZER_HPP

#include <string>
#include <fstream>
#include <cassert>
using namespace std;

// Use system calls to get the size in kilobytes of a given filepath
unsigned long long int getSize(const string &FilePath);

// Turn the output of getSize into a human-readable string
string humanReadable(const unsigned long long int &Size);

#endif
