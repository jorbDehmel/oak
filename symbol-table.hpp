/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <map>
#include <string>
#include <vector>
#include <cassert>
#include <stdexcept>
#include <iostream>

#include "type-builder.hpp"
#include "lexer.hpp"
using namespace std;

class parse_error : public runtime_error
{
public:
    parse_error(const string &What) : runtime_error(What) {}
};

#define parse_assert(what) ((bool)(what)) ? true : (throw parse_error("Assertion '" #what "' failed (file " __FILE__ "line " + to_string(__LINE__) + ")"))

enum sequenceInfo
{
    function_call, // hi(1, 2, 3)
    parenthesis,   // (1 == 2) <=> bool
    code_scope,    // {1; 2; 3}
    code_line,     // 1;
    for_triple,    // (i: i16 = 0; i < 10; i++) <=> (NULL, bool, ANY)
    access,        // a[5];
    atom,          // Anything else; Uses "raw" to specify
    comma_sep,
    keyword,
};

struct sequence
{
    Type type;
    vector<sequence> items;
    sequenceInfo info = code_line;
    string raw; // If needed
};

// For internal use
struct __multiTableSymbol
{
    sequence seq;
    Type type;
    unsigned long long uses;
};

// For later instantiation
struct __template_info
{
    vector<string> generics;
    vector<string> guts;
    vector<string> returnType;
};

typedef map<string, vector<__multiTableSymbol>> multiSymbolTable;
typedef map<string, vector<__template_info>> multiTemplTable;

extern multiSymbolTable table;

// Returns the C version of a sequence
string toC(const sequence &What);

// Converts lexed symbols into a type
Type toType(const vector<string> &What, const set<string> &Local = set<string>());
Type toType(const string &What);

// Can throw errors (IE malformed definitions)
// Takes in the whole definition, starting at let
// and ending after }. (Oak has no trailing semicolon)
// Can also handle templating
void addStruct(const vector<string> &From);
void addStruct(const string &From);

// Takes full definition, possibly including "let"
// Could also be a template!
void addSymb(const string &Name, const vector<string> &From, const sequence &Seq);

#endif
