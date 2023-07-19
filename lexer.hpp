/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>
#include <iostream>

#include "tags.hpp"

using namespace std;

// If an empty symbol is printed, it is a newline literal
vector<string> lex(const string &What);

// Throws an error upon failure
void smartSystem(const string &What);

#endif
