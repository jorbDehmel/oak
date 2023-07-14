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

#define parse_assert(what) ((bool)(what)) ? true : (throw parse_error("Assertion '" #what "' failed."))

class tableSymbol
{
public:
    tableSymbol() : type(atomic, "NULL"), uses(0), line(0) {}

    tableSymbol(const TypeInfo &Info,
                const unsigned int &Line = 0,
                const string &Name = "",
                const unsigned int &Uses = 0)
        : type(Info, Name), uses(Uses), line(Line) {}

    tableSymbol(const Type &Type,
                const unsigned int &Line = 0,
                const unsigned int &Uses = 0)
        : type(Type), uses(Uses), line(Line) {}

    tableSymbol(const tableSymbol &What)
        : type(What.type), uses(What.uses), line(What.line) {}

    tableSymbol &operator=(const tableSymbol &What)
    {
        type = What.type;
        uses = What.uses;
        line = What.line;

        return *this;
    }

    Type type;
    unsigned int uses, line;
};

enum sequenceInfo
{
    list,          // a = [1, 2, 3,]
    function_call, // hi(1, 2, 3)
    parenthesis,   // (1 == 2) <=> bool
    code_scope,    // {1; 2; 3}
    code_line,     // 1;
    for_triple,    // (i: i16 = 0; i < 10; i++) <=> (NULL, bool, ANY)
    access,        // a[5];
    atom,          // Anything else; Uses "raw" to specify
    keyword,       // Const, mut, let, etc...
};

struct sequence
{
    Type type;
    vector<sequence> items;
    sequenceInfo info = code_line;
    string raw; // If needed
};

/*
Stage 1 (raw):
let a: c = hi(1, 2, a.b());

Stage 2 (after method replacement):
let a: c = hi(1, 2, b(&a));

Stage 3 (after toSequence):
code_line -> atom "let" -> atom "a" -> atom ":" -> atom "c" -> atom "=" -> atom "hi" -> function_call ->
    {atom "1", atom "2", atom "b"
        -> function_call -> {atom "&" -> atom "a" }
    }

Stage 4 (C):


(Then can be converted into C)
*/

string toC(const sequence &What);

// For internal use
struct __multiTableSymbol
{
    tableSymbol ts;
    sequence seq;
};

typedef map<string, vector<__multiTableSymbol>> multiSymbolTable;
extern multiSymbolTable table;

/*
// From type-builder.hpp:
extern set<string> atomics, modifiers;
extern map<string, map<string, Type>> structData;
*/

// Converts lexed symbols into a type
Type toType(const vector<string> &What, const set<string> &Local = set<string>());
Type toType(const string &What);

// Can throw errors (IE malformed definitions)
// Takes in the whole definition, starting at let
// and ending after }. (Oak has no trailing semicolon)
void addStruct(const vector<string> &From);
void addStruct(const string &From);

// Takes full definition, possibly including "let"
__multiTableSymbol *addSymb(const string &Name, const string &String);
__multiTableSymbol *addSymb(const string &Name, const vector<string> &From);

#endif
