/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author

Handles the Oak symbol table.
*/

#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "lexer.hpp"
#include "type_builder.hpp"

class parse_error : public std::runtime_error
{
  public:
    parse_error(const std::string &What) : std::runtime_error(What)
    {
    }
};

#define parse_assert(what) ((bool)(what)) ? true : (throw parse_error("Assertion '" #what "' failed."))

// External definition
std::string mangle(const std::vector<std::string> &what);

enum sequenceInfo
{
    code_scope, // {1; 2; 3}
    code_line,  // 1;
    atom,       // Anything else; Uses "raw" to specify
    keyword,
    enum_keyword,
};

// Abstract Syntax Tree node: I don't know why I called it this
struct sequence
{
    Type type;
    std::vector<sequence> items;
    sequenceInfo info = code_line;
    std::string raw; // If needed
};

// For internal use
struct __multiTableSymbol
{
    sequence seq;
    Type type;

    bool erased = false;
    std::string sourceFilePath = "";
};

typedef std::map<std::string, std::vector<__multiTableSymbol>> multiSymbolTable;

// typedef map<string, vector<__template_info>> multiTemplTable;

extern multiSymbolTable table;

// Returns the C version of a sequence
std::string toC(const sequence &What);

// Converts lexed symbols into a type
Type toType(const std::vector<token> &What);
Type toType(const std::vector<std::string> &What);

// Can throw errors (IE malformed definitions)
// Takes in the whole definition, starting at let
// and ending after }. (Oak has no trailing semicolon)
// Can also handle templating
void addStruct(const std::vector<token> &From);

// Extern defs
std::string mangleStruct(const std::string &name, const std::vector<std::vector<std::string>> &generics);
std::string instantiateGeneric(const std::string &what, const std::vector<std::vector<std::string>> &genericSubs,
                               const std::vector<std::string> &typeVec);

/*
Erases any non-function symbols which were not present
in the original table. However, skips all functions.
If not contradicted by the above rules, bases off of
the current table (not backup).
*/
std::vector<std::pair<std::string, std::string>> restoreSymbolTable(multiSymbolTable &backup);

#endif
