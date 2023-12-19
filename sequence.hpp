/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author

If a sequence's type is the NULL atomic, that means
that it has no hanging type. It does NOT mean it is
an empty sequence; Consider semicolon-terminated
lines.

Handles sequencing in Oak:
Basically everything occurring within a function

A sequence can contain other sequences, as long as
they combine to create exactly one mega-sequence.
This is the only part of the language in which
AST structures might be useful.

A sequence takes a number of other sequences,
each of which have a return type. These sequence
types are compile-time enforced.

All functions have exactly one attached sequence.
Function signatures will be handles by the
non-sequential parser.
*/

#ifndef SEQUENCE_HPP
#define SEQUENCE_HPP

#include <algorithm>
#include <cmath>
#include <cstring>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "enums.hpp"
#include "generics.hpp"
#include "mangler.hpp"
#include "mem.hpp"
#include "reconstruct.hpp"
#include "symbol_table.hpp"

#include "tags.hpp"

using namespace std;

// Globals
extern const set<string> specials;
extern unsigned long long int curLine;
extern string curFile;
// extern bool skipCodeScopes;

// For error trace
extern vector<string> curLineSymbols;

// Extension of runtime error for Oak sequencing
class sequencing_error : public runtime_error
{
  public:
    sequencing_error(const string &What) : runtime_error(What)
    {
    }
};

// Sequence message assert
#define sm_assert(expression, message)                                                                                 \
    ((bool)(expression) ? true                                                                                         \
                        : throw sequencing_error(string(message) + " (Failed assertion: '" #expression "') " +         \
                                                 string(strrchr("/" __FILE__, '/') + 1) + " " + to_string(__LINE__)))

// Externally useful functions

// Creates a sequence from a lexed string.
// Return type is deduced naturally from the contents.
// Can throw sequencing errors.
sequence createSequence(const vector<string> &From);

// Internal consumptive version: Erases from vector, so not safe for frontend
sequence __createSequence(list<string> &From);

// Turn a .oak sequence into a .cpp one
string toC(const sequence &What);

// Get the return type from a Type (of a function signature)
Type getReturnType(const Type &T);

// Gets the arguments from a Type, given that it is a function
vector<pair<string, Type>> getArgs(Type &type);

// Ignores all var_names
bool typesAreSame(Type *A, Type *B);

void debugPrint(const sequence &What, int spaces = 0, ostream &to = cout);

Type resolveFunction(const vector<string> &What, int &start, string &c);
Type checkLiteral(const string &From);
string restoreSymbolTable(multiSymbolTable &backup);

void addEnum(const vector<string> &FromIn);

// Dump data to file
void dump(const vector<string> &Lexed, const string &Where, const string &FileName, const int &Line,
          const sequence &FileSeq, const vector<string> LexedBackup);

#endif
