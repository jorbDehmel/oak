/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author

Generates a markdown document from the commenting
of an Oak source file
*/

#ifndef DOCUMENT_HPP
#define DOCUMENT_HPP

#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

void generate(const std::vector<std::string> &Files, const std::string &Output);

#endif
