/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#ifndef ACORN_RESOURCES_HPP
#define ACORN_RESOURCES_HPP

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "document.hpp"
#include "lexer.hpp"
#include "macros.hpp"
#include "op_sub.hpp"
#include "options.hpp"
#include "packages.hpp"
#include "reconstruct.hpp"
#include "rules.hpp"
#include "sequence.hpp"
#include "sizer.hpp"
#include "symbol_table.hpp"
#include "tags.hpp"
#include "type_builder.hpp"

// Prints the cumulative disk usage of Oak (human-readable).
void getDiskUsage();

// The most important function in the compiler; Actually loads,
// lexes, does rules, does AST-analysis and reconstruction. This
// is the core of the compiler.
void doFile(const std::string &filepath, AcornSettings &settings);

// Creates a factory-settings package with the given name in the
// current directory. Sets up everything you need.
void makePackage(const std::string &name);

// Ensures that the text passed adheres to basic style-based
// syntactical requirements (IE line width limits, matching
// parenthesis). If fatal, does not allow recovery. Otherwise,
// does.
void ensureSyntax(const std::string &text, const bool &fatal, const std::string &curFile);

#endif
