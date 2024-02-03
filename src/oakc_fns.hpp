/*
Functions signatures for the Acorn compiler. The definitions for
these functions are scattered around in various `.cpp` files.
These can all basically be considered methods of the
`AcornSettings` type.

File structure:
```
    lexer.hpp
    |
    oakc_structs.hpp
    |
    options.hpp
    |
    oakc_fns.hpp   <---
    |
    <compiler frontend>
```

Jordan Dehmel, 2024
jdehmel@outlook.com
*/

#ifndef ACORN_FNS
#define ACORN_FNS

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "options.hpp"

// Prints the cumulative disk usage of Oak (human-readable).
void getDiskUsage();

// The most important function in the compiler; Actually loads,
// lexes, does rules, does AST-analysis and reconstruction. This
// is the core of the compiler.
void doFile(const std::string &filepath, AcornSettings &settings);

// Creates a factory-settings package with the given name in the
// current directory. Sets up everything you need.
void makePackage(const std::string &name);

// Ensures that the text passed adheres to basic style-based
// syntactical requirements (IE line width limits, matching
// parenthesis). If fatal, does not allow recovery. Otherwise,
// does.
void ensureSyntax(const std::string &text, const bool &fatal, const std::string &curFile);

// Generate a single `.md` file from the given file(s). Saves at
// the provided filepath.
void generate(const std::vector<std::string> &files, const std::string &output);

// Can throw generic_error's if no viable options exist.
// Ensure all items in genericSubs have been pre-mangled.
// Returns the mangled version.
std::string instantiateGeneric(const std::string &what, const std::vector<std::vector<std::string>> &genericSubs,
                               const std::vector<std::string> &typeVec, AcornSettings &settings);

// Also holds the skeleton of the inst block system, although
// gathering of these happens elsewhere.
void addGeneric(const std::vector<Token> &what, const std::string &name, const std::vector<std::string> &genericsList,
                const std::vector<std::string> &typeVec, const std::vector<Token> &preBlock,
                const std::vector<Token> &postBlock, AcornSettings &settings);

// Print the info of all existing generics to a file stream
void printGenericDumpInfo(std::ostream &file, AcornSettings &settings);

// USES SYSTEM CALLS. Ensures a given macro exists, then calls
// it with the given arguments. If debug is true, specifies such
// in the call.
std::string callMacro(const std::string &Name, const std::vector<std::string> &Args, AcornSettings &settings);

// USES SYSTEM CALLS. If source code exists for `name` but it is
// not yet compiled, calls another instance of the compiler on
// its contents. Note: The sub-instance of the compiler runs in
// a special macro-compilation mode which inhibits its outputs
// and syntax constraints.
void compileMacro(const std::string &Name, AcornSettings &settings);

// Returns the given file's last modification time, in seconds
// after the epoch.
long long getFileLastModification(const std::string &filepath, AcornSettings &settings);

// Returns true if the source file is newer than the destination
// one OR if either file is nonexistant.
bool isSourceNewer(const std::string &source, const std::string &dest, AcornSettings &settings);

// Returns true if and only if the given macro already exists.
bool hasMacro(const std::string &name, AcornSettings &settings) noexcept;

// Adds the given macro definition into the internal macro lookup table.
void addMacro(const std::string &name, const std::string &contents, AcornSettings &settings);

// Used for mangling and resolving generics pre-sequencing
std::string mangle(const std::vector<std::string> &what);

// Mangle a struct, given its generic substitutions
std::string mangleStruct(const std::string &name, const std::vector<std::vector<std::string>> &generics =
                                                      std::vector<std::vector<std::string>>());

// Mangle an enumeration, give its generic substitutions
std::string mangleEnum(const std::string &name,
                       const std::vector<std::vector<std::string>> &generics = std::vector<std::vector<std::string>>());

// Fully qualify a type in a canonical way.
std::string mangleType(const Type &type);

// Fully qualify a symbol in a canonical way.
std::string mangleSymb(const std::string &name, Type &type);

// Fully qualify a symbol in a canonical way.
std::string mangleSymb(const std::string &name, const std::string &typeStr);

// Returns the ASTNode which allocates an array of size num and type type.
ASTNode getAllocSequence(Type &type, const std::string &name, AcornSettings &settings, const std::string &num = "1");

// Returns the ASTNode which frees the referenced memory
ASTNode getFreeSequence(const std::string &name, AcornSettings &settings);

/*
Takes entire lexed token stream. After call, no operators
should remain.
*/
void operatorSub(std::vector<Token> &from);

// Installs a given SYSTEM package; NOT an Oak one.
void install(const std::string &what, AcornSettings &settings);

// Output package information to an output stream in a pretty
// way.
std::ostream &operator<<(std::ostream &strm, const PackageInfo &info);

// Loads a package info file.
PackageInfo loadPackageInfo(const std::string &Filepath, AcornSettings &settings);

// Loads all packages which are currently known about.
void loadAllPackages();

// Saves a package info file.
void savePackageInfo(const PackageInfo &info, const std::string &filepath);

// Downloads a package from a URL via git.
void downloadPackage(const std::string &url, AcornSettings &settings, const bool &reinstall = false,
                     const std::string &path = "");

// Get the include!() -ed files of a package given name and
// possibly URL.
std::vector<std::string> getPackageFiles(const std::string &Name, AcornSettings &settings);

// Removes illegal characters.
std::string purifyStr(const std::string &what);

// Reconstruct the existing symbol table into C.
void reconstruct(const std::string &Name, AcornSettings &settings, std::stringstream &header, std::stringstream &body);

// Save reconstructed files and return compilation command
// Return pair<sstream, sstream>{header, body};
std::pair<std::string, std::string> save(const std::stringstream &header, const std::stringstream &body,
                                         const std::string &name);

// Call reconstruct and save, without fiddling with
// stringstreams. Returns headerName, bodyName.
std::pair<std::string, std::string> reconstructAndSave(const std::string &Name, AcornSettings &settings);

// Return the C format-version of a type, to be followed by
// symbol name.
std::string toStrC(const Type *What, AcornSettings &settings, const std::string &Name = "",
                   const unsigned int &pos = 0);

// Return the C format-version of a function, INCLUDING symbol
// name.
std::string toStrCFunction(const Type *what, AcornSettings &settings, const std::string &name,
                           const unsigned int &pos = 0);

// Other type of C function; IE bool (*what)(const bool &What);.
std::string toStrCFunctionRef(const Type *what, AcornSettings &settings, const std::string &name,
                              const unsigned int &pos = 0);

// Return the C version of an Oak `enum`. These are heavily
// involved, more so than structs or variables.
std::string enumToC(const std::string &name, AcornSettings &settings);

// Add a new rule engine.
void addEngine(const std::string &name, void (*hook)(std::vector<Token> &, int &, Rule &, AcornSettings &),
               AcornSettings &settings);

// `i` is the point in Lexed at which a macro name was found.
// CONSUMPTIVE on `lexed`.
std::vector<std::string> getMacroArgs(std::vector<Token> &lexed, const int &i);

// Does all active rules on a given token stream.
void doRules(std::vector<Token> &From, AcornSettings &settings);

// Load a dialect file.
void loadDialectFile(const std::string &File, AcornSettings &settings);

// Internal pass-through for Sapling rule engine.
void doRuleAcorn(std::vector<Token> &From, int &i, Rule &curRule, AcornSettings &settings);

// Converts lexed symbols into a type.
Type toType(const std::vector<Token> &whatIn, AcornSettings &settings);

// Converts lexed symbols into a type.
Type toType(const std::vector<std::string> &what, AcornSettings &settings);

// ASTNode message assert
void sm_assert(const bool &expression, const std::string &message);

// Turn a .oak AST into a .c one.
std::string toC(const ASTNode &What, AcornSettings &settings);

// Clean the input to a compiler macro.
std::string cleanMacroArgument(const std::string &from);

// Destroy all unit, temp, or autogen definitions matching a
// given type. Can throw errors if doThrow is true.
void destroyUnits(const std::string &name, const Type &type, const bool &doThrow, AcornSettings &settings);

// Get the return type from a Type (of a function signature).
Type getReturnType(const Type &T, AcornSettings &settings);

// Gets the arguments from a Type, given that it is a function
std::vector<std::pair<std::string, Type>> getArgs(Type &type, AcornSettings &settings);

// Print the AST in a pretty-ish way.
void debugPrint(const ASTNode &what, int spaces = 0, std::ostream &to = std::cout);

// Adds an enumeration into the type symbol table.
void addEnum(const std::vector<Token> &From, AcornSettings &settings);

// Can throw errors (IE malformed definitions)
// Takes in the whole definition, starting at let
// and ending after }. (Oak has no trailing semicolon)
// Can also handle templating
void addStruct(const std::vector<Token> &From, AcornSettings &settings);

// Dump data to file. Mostly used for debugging purposes.
void dump(const std::vector<Token> &Lexed, const std::string &Where, const std::string &FileName, const int &Line,
          const ASTNode &FileSeq, const std::vector<Token> LexedBackup, const std::string &ErrorMsg,
          AcornSettings &settings);

// Get a valid constructor call for a given struct member var.
std::string getMemberNew(const std::string &selfName, const std::string &varName, const Type &varType,
                         AcornSettings &settings);

// Get a valid destructor call for a given struct member var.
std::string getMemberDel(const std::string &selfName, const std::string &varName, const Type &varType,
                         AcornSettings &settings);

// Inserts destructors at all appropriate places in a ASTNode.
void insertDestructors(ASTNode &what, const std::vector<std::pair<std::string, std::string>> &destructors,
                       AcornSettings &settings);

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
                          const std::string &name, AcornSettings &settings);

// Creates a sequence from a lexed string.
// Return type is deduced naturally from the contents.
// Can throw sequencing errors.
ASTNode createSequence(const std::vector<Token> &from, AcornSettings &settings);

// Get the return type; Set as a global
Type resolveFunction(const std::vector<Token> &What, int &start, std::string &c, AcornSettings &settings);

// Use filesystem calls to get the size in kilobytes of a given
// filepath.
unsigned long long int getSize(const std::string &filePath);

// Turn the output of getSize into a human-readable string.
std::string humanReadable(const unsigned long long int &size);

/*
Erases any non-function symbols which were not present
in the original table. However, skips all functions.
If not contradicted by the above rules, bases off of
the current table (not backup).
*/
std::vector<std::pair<std::string, std::string>> restoreSymbolTable(MultiSymbolTable &backup,
                                                                    MultiSymbolTable &realTable);

// Return the standard C representation of this type.
std::string toStr(const Type *const what, const unsigned int &pos = 0);

// Ignores all var_names, plus auto-ref/deref.
bool typesAreSame(const Type *const a, const Type *const b, int &changes);

// Like the above, but does not do auto-referencing or
// dereferencing.
bool typesAreSameExact(const Type *const a, const Type *const b);

/*
Compares two types. Returns true if they match exactly, if they
match using auto-reference/auto-dereference, or internal literal
casting. The number of changes is recorded in `changes`.
*/
bool typesAreSameCast(const Type *const a, const Type *const b, int &changes);

/*
Check if a given single token is a literal. Returns the type of
the literal if it is, otherwise nullType.
*/
Type checkLiteral(const std::string &from);

#endif
