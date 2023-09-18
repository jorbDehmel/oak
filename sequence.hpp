/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#ifndef SEQUENCE_HPP
#define SEQUENCE_HPP

/*
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

#include <vector>
#include <string>
#include <set>
#include <cstring>
#include <cmath>
#include <map>

#include <list>

#include "mem.hpp"
#include "type-builder.hpp"
#include "symbol-table.hpp"
#include "reconstruct.hpp"
#include "enums.hpp"
#include "mangler.hpp"
#include "generics.hpp"

#include "tags.hpp"

using namespace std;

// Output language options
enum outputLanguageType
{
    cpp,
    // llvm, // Eventually, but not yet
};

// The specific output language selected
extern outputLanguageType outputLanguage;

// Globals
extern const set<string> specials;
extern unsigned long long int curLine;
extern string curFile;

// For error trace
extern vector<string> curLineSymbols;

// Extension of runtime error for Oak sequencing
class sequencing_error : public runtime_error
{
public:
    sequencing_error(const string &What) : runtime_error(What) {}
};

// Sequence message assert
#define sm_assert(expression, message)                                           \
    ((bool)(expression)                                                          \
         ? true                                                                  \
         : throw sequencing_error(message                                        \
                                  " (Failed assertion: '" #expression "') " +    \
                                  string(strrchr("/" __FILE__, '/') + 1) + " " + \
                                  to_string(__LINE__)))

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

#endif
