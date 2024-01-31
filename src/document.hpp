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

// Generate a single `.md` file from the given file(s). Saves at
// the provided filepath.
void generate(const std::vector<std::string> &files, const std::string &output);

#endif
