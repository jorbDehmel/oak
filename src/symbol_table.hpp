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

////////////////////////////////////////////////////////////////

// An error which may arise during parsing / lexing.
class parse_error : public std::runtime_error
{
  public:
    parse_error(const std::string &what) : std::runtime_error(what)
    {
    }
};

// If the given item is false, throw a parse_error.
#define parse_assert(b)                                                                                                \
    ((bool)(b)) ? true : throw parse_error(__FILE__ ":" + std::to_string(__LINE__) + " Failed assertion '" #b "'\n")

// Enumeration representing the type of a single AST node.
enum SequenceInfo
{
    code_scope, // {1; 2; 3}
    code_line,  // 1;
    atom,       // Anything else; Uses "raw" to specify
    keyword,
    enum_keyword,
};

// Abstract Syntax Tree node. This represents a complete AST in
// itself. This is where the majority of reconstructive and
// transformative work will be done.
struct ASTNode
{
    Type type;
    std::vector<ASTNode> items;
    SequenceInfo info = code_line;
    std::string raw; // If needed
};

// Holds multiple possible entrees for a given name in the
// symbol table.
struct MultiTableSymbol
{
    ASTNode seq;
    Type type;

    bool erased = false;
    std::string sourceFilePath = "";
};

// Alias for the symbol table.
typedef std::map<std::string, std::vector<MultiTableSymbol>> MultiSymbolTable;

/*
Erases any non-function symbols which were not present
in the original table. However, skips all functions.
If not contradicted by the above rules, bases off of
the current table (not backup).
*/
std::vector<std::pair<std::string, std::string>> restoreSymbolTable(MultiSymbolTable &backup,
                                                                    MultiSymbolTable &realTable);

#endif
