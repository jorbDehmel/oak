/*
Methods to save and load the entire state of the
compiler, including all symbols, structs, data,
used files, etc. all in one file.

Jordan Dehmel, 2023
jdehmel@outlook.com
*/

#ifndef COMPILER_CACHE_HPP
#define COMPILER_CACHE_HPP

// #include "acorn_resources.hpp"
#include "generics.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

class cache_error : public runtime_error
{
  public:
    cache_error(const string &what) : runtime_error(what)
    {
    }
};

// Save the entire state of the compiler to a file
void saveCompilerCache(const string &where);

// Load the entire state of the compiler from a file
// WARNING: Completely overwrites all runtime data in
// this translation unit.
void loadCompilerCache(const string &where);

// For saving and loading Types from caches
ostream &operator<<(ostream &strm, Type &type);
istream &operator>>(istream &strm, Type &type);

template <class T> ostream &operator<<(ostream &strm, vector<T> &vec)
{
    strm << vec.size() << '\t';
    for (T item : vec)
    {
        strm << item << '\t';
    }
    return strm;
}

template <class T> istream &operator>>(istream &strm, vector<T> &vec)
{
    size_t size = 0;
    strm >> size;
    vec.reserve(vec.size() + size);

    T temp;

    for (int i = 0; i < size; i++)
    {
        strm >> temp;
        vec.push_back(temp);
    }

    return strm;
}

template <class T> ostream &operator<<(ostream &strm, set<T> &vec)
{
    strm << vec.size() << '\t';
    for (T item : vec)
    {
        strm << item << '\t';
    }
    return strm;
}

template <class T> istream &operator>>(istream &strm, set<T> &vec)
{
    size_t size = 0;
    strm >> size;
    // vec.reserve(vec.size() + size);

    T temp;

    for (int i = 0; i < size; i++)
    {
        strm >> temp;
        vec.insert(temp);
    }

    return strm;
}

#endif
