/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author

Pre-translation operator substitution for Oak
Should be done after macros
*/

#ifndef OP_SUB_HPP
#define OP_SUB_HPP

#include "symbol_table.hpp"
#include <iostream>
#include <string>
#include <vector>

/*
Takes entire lexed token stream. After call, no operators
should remain.
*/
void operatorSub(std::vector<std::string> &From);

#endif
