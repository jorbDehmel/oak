/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#ifndef OP_SUB_HPP
#define OP_SUB_HPP

// Pre-translation operator substitution for Oak
// Should be done after macros

#include "symbol-table.hpp"

#include <string>
#include <vector>
#include <iostream>

using namespace std;

// Replaces all operator calls with their corresponding operator
// CHANGES THE VECTOR!!
void operatorSub(vector<string> &From);

// You should call this one.
void parenSub(vector<string> &From);

#endif
