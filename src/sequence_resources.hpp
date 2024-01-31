/*
Lightens the compiler load for ASTNode.cpp by breaking 1 4000
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

// External non-constant globals
extern const std::set<std::string> specials;
extern unsigned long long int curLine;
extern std::string curFile;
extern std::vector<Token> curLineSymbols;

// Extension of runtime error for Oak sequencing
class sequencing_error : public std::runtime_error
{
  public:
    sequencing_error(const std::string &What) : runtime_error(What)
    {
    }
};

// ASTNode message assert
void sm_assert(const bool &expression, const std::string &message);

// Turn a .oak AST into a .cpp one.
std::string toC(const ASTNode &what);

// Clean the input to a compiler macro.
std::string cleanMacroArgument(const std::string &from);

// Destroy all unit, temp, or autogen definitions matching a
// given type. Can throw errors if doThrow is true.
void destroyUnits(const std::string &name, const Type &type, const bool &doThrow);

// Get the return type from a Type (of a function signature).
Type getReturnType(const Type &what);

// Gets the arguments from a Type, given that it is a function
std::vector<std::pair<std::string, Type>> getArgs(Type &type);

// Print the AST in a pretty-ish way.
void debugPrint(const ASTNode &what, int spaces = 0, std::ostream &to = std::cout);

// Restore the symbol table to a previous state. Only erases
// instance variables.
std::vector<std::pair<std::string, std::string>> restoreSymbolTable(MultiSymbolTable &backup);

// Adds an enumeration into the type symbol table.
void addEnum(const std::vector<Token> &fromIn);

// Dump data to file. Mostly used for debugging purposes.
void dump(const std::vector<Token> &lexed, const std::string &where, const std::string &fileName, const int &line,
          const ASTNode &fileSeq, const std::vector<Token> lexedBackup, const std::string &errorMsg = "");

// Get a valid constructor call for a given struct member var.
std::string getMemberNew(const std::string &selfName, const std::string &varName, const Type &varType);

// Get a valid destructor call for a given struct member var.
std::string getMemberDel(const std::string &selfName, const std::string &varName, const Type &varType);

// Inserts destructors at all appropriate places in a ASTNode.
void insertDestructors(ASTNode &what, const std::vector<std::pair<std::string, std::string>> &destructors);

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
void printCandidateErrors(const std::vector<MultiTableSymbol> &candidates, const std::vector<Type> &argTypes,
                          const std::string &name);

////////////////////////////////////////////////////////////////

#endif
