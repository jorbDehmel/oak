/*
Lightens the compiler load for sequence.cpp by breaking 1 4000
line file into 2 2000 line files (this is marginally better for
makefiles).

Jordan Dehmel
jdehmel@outlook.com
*/

#ifndef SEQUENCE_RESOURCES_HPP
#define SEQUENCE_RESOURCES_HPP

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iomanip>
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
#include "type_builder.hpp"

#include "tags.hpp"
using namespace std;

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
    sequencing_error(const string &What) : runtime_error(What)
    {
    }
};

// Sequence message assert
#define sm_assert(expression, message)                                                                                 \
    ((bool)(expression) ? true                                                                                         \
                        : throw sequencing_error(string(message) + " (Failed assertion: '" #expression "') " +         \
                                                 string(strrchr("/" __FILE__, '/') + 1) + " " + to_string(__LINE__)))

// Turn a .oak sequence into a .cpp one
string toC(const sequence &What);

string cleanMacroArgument(const string &from);

// Destroy all unit, temp, or autogen definitions matching a given type.
// Can throw errors if doThrow is true.
// Mostly used for New and Del, Oak ~0.0.14
void destroyUnits(const string &name, const Type &type, const bool &doThrow);

// Get the return type from a Type (of a function signature)
Type getReturnType(const Type &T);

// Gets the arguments from a Type, given that it is a function
vector<pair<string, Type>> getArgs(Type &type);

void debugPrint(const sequence &What, int spaces = 0, ostream &to = cout);

Type checkLiteral(const string &From);
string restoreSymbolTable(multiSymbolTable &backup);

void addEnum(const vector<string> &FromIn);

// Dump data to file
void dump(const vector<string> &Lexed, const string &Where, const string &FileName, const int &Line,
          const sequence &FileSeq, const vector<string> LexedBackup);

string getMemberNew(const string &selfName, const string &varName, const Type &varType);
string getMemberDel(const string &selfName, const string &varName, const Type &varType);

////////////////////////////////////////////////////////////////

// Internal function definition candidate matching functions.
// Used for identifying candidates and possibly printing errors
// for them.

// Get all candidates which match exactly. Returns vector of all
// indices which match exactly. Takes in candArgs, which is a
// list of lists of argument Types. the 0th item in candArgs is
// a vector of the types of the arguments of the 0th candidate.
// argTypes is a vector of the types provided
vector<int> getStageOneCandidates(const vector<vector<Type>> &candArgs, const vector<Type> &argTypes);

// Get all candidates which match with implicit casting. Works
// as above.
vector<int> getStageTwoCandidates(const vector<vector<Type>> &candArgs, const vector<Type> &argTypes);

// Get all candidates which match with auto referencing and/or
// dereferencing. Works as above.
vector<int> getStageThreeCandidates(const vector<vector<Type>> &candArgs, const vector<Type> &argTypes);

// Prints the reason why each candidate was rejected
void printCandidateErrors(const vector<__multiTableSymbol> &candidates, const vector<Type> &argTypes,
                          const string &name);

////////////////////////////////////////////////////////////////

#endif
