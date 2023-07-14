/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#ifndef AUTO_OAK_HPP
#define AUTO_OAK_HPP

// Populate the Oak symbol table automatically by scanning a
// .cpp, .c, .h, or .hpp file. Only worries about function
// definitions, not their contents, because Oak is a translated
// language. Actual function linking will be done during C++
// compilation.

#include <string>
#include <iostream>
#include <fstream>

#include "lexer.hpp"
#include "macros.hpp"
#include "reconstruct.hpp"
#include "symbol-table.hpp"
#include "type-builder.hpp"

using namespace std;

int autoOak(const string &Filename);

#endif
