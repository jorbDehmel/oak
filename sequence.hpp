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

#include "mem.hpp"
#include "type-builder.hpp"
#include "symbol-table.hpp"
#include "reconstruct.hpp"

#include "tags.hpp"

using namespace std;

// Globals

extern const set<string> specials;
extern unsigned long long int curLine;
extern string curFile;

class sequencing_error : public runtime_error
{
public:
    sequencing_error(const string &What) : runtime_error(What) {}
};

// Macros

#define seq_assert(what) \
    ((bool)(what) ? true : throw sequencing_error("Assertion '" #what "' failed."))

// Sequence message assert
#define sm_assert(expression, message)                                           \
    ((bool)(expression)                                                          \
         ? true                                                                  \
         : throw sequencing_error(message                                        \
                                  " (Failed assertion: '" #expression "') " +    \
                                  string(strrchr("/" __FILE__, '/') + 1) + " " + \
                                  to_string(__LINE__)))

#define pop_front(Of) \
    Of.empty() ? throw runtime_error("Cannot pop front from empty vector; line " + to_string(__LINE__)) : Of.erase(Of.begin())

#define printvec(What)                                                \
    {                                                                 \
        cout << #What << ' ' << __LINE__ << ' ' << __FILE__ << ":\n"; \
        for (auto s : What)                                           \
            cout << s << ' ';                                         \
        cout << '\n';                                                 \
    }

// Externally useful functions

// Creates a sequence from a lexed string.
// Return type is deduced naturally from the contents.
// Can throw sequencing errors.
sequence createSequence(const vector<string> &From);
sequence createSequence(const string &From);

// Internal consumptive version: Erases from vector, so not safe for frontend
sequence __createSequence(vector<string> &From);

// As above, but enforce type accuracy.
sequence createSequence(const vector<string> &From, const Type type);

// Turn a .oak sequence into a .cpp one
string toC(const sequence &What);

// Get the return type of a function which exists in the symbol table
Type getReturnType(const string &Name, const Type &ArgType, bool templated = false);

// Get the return type from a Type (of a function signature)
Type getReturnType(const Type &T);

// Enforce member existence
Type getMemberType(const Type &Base, const string &Name);

Type getType(const string &Name);

map<string, Type> getArgs(Type &type);

void debugPrint(const sequence &What, int spaces = 0, ostream &to = cout);

// a.b() -> b(a), a.b().c().d.e() -> e(c(b(a)).d), etc
vector<string> fixMethodCall(const vector<string> &What);
vector<string> fixMethodCall(const string &What);

#endif
