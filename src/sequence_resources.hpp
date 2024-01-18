/*
Lightens the compiler load for sequence.cpp by breaking 1 4000
line file into 2 2000 line files (this is marginally better for
makefiles).

Jordan Dehmel
2023 - present
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

// Globals
extern const std::set<std::string> specials;
extern unsigned long long int curLine;
extern std::string curFile;

// For error trace
extern std::vector<std::string> curLineSymbols;

// Extension of runtime error for Oak sequencing
class sequencing_error : public std::runtime_error
{
  public:
    sequencing_error(const std::string &What) : runtime_error(What)
    {
    }
};

// Sequence message assert
void sm_assert(const bool &expression, const std::string &message);

// Turn a .oak sequence into a .cpp one
std::string toC(const sequence &What);

std::string cleanMacroArgument(const std::string &from);

// Destroy all unit, temp, or autogen definitions matching a given type.
// Can throw errors if doThrow is true.
// Mostly used for New and Del, Oak ~0.0.14
void destroyUnits(const std::string &name, const Type &type, const bool &doThrow);

// Get the return type from a Type (of a function signature)
Type getReturnType(const Type &T);

// Gets the arguments from a Type, given that it is a function
std::vector<std::pair<std::string, Type>> getArgs(Type &type);

void debugPrint(const sequence &What, int spaces = 0, std::ostream &to = std::cout);

Type checkLiteral(const std::string &From);
std::vector<std::pair<std::string, std::string>> restoreSymbolTable(multiSymbolTable &backup);

void addEnum(const std::vector<token> &FromIn);

// Dump data to file
void dump(const std::vector<token> &Lexed, const std::string &Where, const std::string &FileName, const int &Line,
          const sequence &FileSeq, const std::vector<token> LexedBackup, const std::string &ErrorMsg = "");

std::string getMemberNew(const std::string &selfName, const std::string &varName, const Type &varType);
std::string getMemberDel(const std::string &selfName, const std::string &varName, const Type &varType);

// Inserts destructors at all appropriate places in a sequence
void insertDestructors(sequence &what, const std::vector<std::pair<std::string, std::string>> &destructors);

////////////////////////////////////////////////////////////////

// Internal function definition candidate matching functions.
// Used for identifying candidates and possibly printing errors
// for them.

// Get all candidates which match exactly. Returns vector of all
// indices which match exactly. Takes in candArgs, which is a
// list of lists of argument Types. the 0th item in candArgs is
// a vector of the types of the arguments of the 0th candidate.
// argTypes is a vector of the types provided
std::vector<int> getExactCandidates(const std::vector<std::vector<Type>> &candArgs, const std::vector<Type> &argTypes);

// Get all candidates which match with implicit casting. Works
// as above.
std::vector<int> getCastingCandidates(const std::vector<std::vector<Type>> &candArgs,
                                      const std::vector<Type> &argTypes);

// Get all candidates which match with auto referencing and/or
// dereferencing. Works as above.
std::vector<int> getReferenceCandidates(const std::vector<std::vector<Type>> &candArgs,
                                        const std::vector<Type> &argTypes);

// Prints the reason why each candidate was rejected
void printCandidateErrors(const std::vector<__multiTableSymbol> &candidates, const std::vector<Type> &argTypes,
                          const std::string &name);

////////////////////////////////////////////////////////////////

#endif
