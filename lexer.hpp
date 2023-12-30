/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author

Lexes files into tokens to be handled by the compiler.
*/

#ifndef LEXER_HPP
#define LEXER_HPP

#include <cstring>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "tags.hpp"

using namespace std;

// An assertion which throws a runtime error instead of breaking everything
#define throw_assert(expression) ((bool)(expression) ? true : throw runtime_error("Assertion " #expression " failed."))

// If an empty symbol is printed, it is a newline literal
vector<string> lex(const string &What);

// Throws an error upon failure
void smartSystem(const string &What);

#endif
