/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#ifndef DOCUMENT_HPP
#define DOCUMENT_HPP

// Generates a markdown document from the commenting
// of an Oak source file

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <chrono>

using namespace std;

void generate(const vector<string> &Files, const string &Output);

#endif
