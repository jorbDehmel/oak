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

// If an empty symbol is printed, it is a newline literal
std::vector<std::string> lex(const std::string &What);

// Throws an error upon failure
void smartSystem(const std::string &What);

#endif
