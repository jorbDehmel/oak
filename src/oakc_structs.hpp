/*
All struct definitions (except those involved in lexing) used
by the oak compiler.

File structure:
```
lexer.hpp
   |
   v
oakc_structs.hpp   <---
   |
   v
options.hpp
   |
   v
oakc_fns.hpp
   |
   v
<compiler frontend>
```

Jordan Dehmel, 2024
jdehmel@outlook.com
*/

#ifndef ACORN_STRUCTS_HPP
#define ACORN_STRUCTS_HPP

#include "lexer.hpp"
#include <cstring>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

// If the given item is false, throw a parse_error.
#define parse_assert(b)                                        \
    ((bool)(b))                                                \
        ? true                                                 \
        : throw parse_error(__FILE__ ":" +                     \
                            std::to_string(__LINE__) +         \
                            " Failed assertion '" #b "'\n")

// An error which may arise during the processing of rules.
class rule_error : public std::runtime_error
{
  public:
    rule_error(const std::string &What)
        : std::runtime_error(What)
    {
    }
};

// Error representing a fault with generic processing in Oak.
// Usually thrown at instantiation-time for a given template.
class generic_error : public std::runtime_error
{
  public:
    generic_error(const std::string &what) : runtime_error(what)
    {
    }
};

// An error which may occur during package loading.
class package_error : public std::runtime_error
{
  public:
    package_error(const std::string &what)
        : std::runtime_error(what)
    {
    }
};

// Extension of runtime error for Oak sequencing
class sequencing_error : public std::runtime_error
{
  public:
    sequencing_error(const std::string &What)
        : runtime_error(What)
    {
    }
};

// An error which may arise during parsing / lexing.
class parse_error : public std::runtime_error
{
  public:
    parse_error(const std::string &what)
        : std::runtime_error(what)
    {
    }
};

// Contains information about a given rule.
struct Rule
{
    std::vector<Token> inputPattern;
    std::vector<Token> outputPattern;
    std::string engineName;
};

// Info about a single generic template. This can be used to
// instantiate.
struct GenericInfo
{
    std::list<std::string> typeVec;
    std::string originFile;
    std::list<Token> symbols;
    std::list<Token> preBlock, postBlock;
    std::list<std::string> genericNames;
    std::list<std::list<std::list<std::string>>> instances;
};

// Holds information about an installed package.
struct PackageInfo
{
    std::string name;      // Package name
    std::string version;   // Package version
    std::string license;   // Package license
    std::string date;      // Date the current version was
                           // released
    std::string author;    // The author(s) of the package
    std::string email;     // The email(s) of the author(s)
    std::string source;    // URL package was downloaded from
    std::string path;      // Path from root to get to the install
                           // point
    std::string about;     // Package description
    std::string toInclude; // File within
                           // /usr/include/oak/$(PACKAGE_NAME)
                           // to include;
    std::string sysDeps;   // System dependencies
    std::string oakDeps;   // Oak dependencies
    std::string validationScript; // Oak file which will
                                  // ensure the package is
                                  // viable
};

// Enumeration representing the type of a single AST node.
enum SequenceInfo
{
    code_scope, // {1; 2; 3}
    code_line,  // 1;
    atom,       // Anything else; Uses "raw" to specify
    keyword,
    enum_keyword,
};

// Information classifying a single type node.
enum TypeInfo
{
    pointer,  // A pointer to the type specified by next.
    atomic,   // An atomic type (or struct/class definition).
    join,     // A comma; Syntactic fluff, more or less.
    function, // Function start (mostly fluff).
    maps,     // Syntactic fluff for functions.
    var_name, // A variable name for functions.
    arr,      // Unsized array. Equivalent to pointer.
    sarr,     // Sized array. like var_name, has size in `name`.
};

// A single node in a type; For instance, in `[]i8`, both
// `arr` and `i8` are nodes.
struct TypeNode
{
    TypeNode &operator=(const TypeNode &other);
    bool operator==(const TypeNode &other) const;

    TypeInfo info;
    std::string name;
};

// A list of type nodes which, taken all together, comprise a
// valid Oak type.
class Type
{
  public:
    Type(const TypeInfo &info, const std::string &name = "");
    Type(const Type &what);
    Type(const Type &what, const int &startingAt);
    Type();

    void prepend(const TypeInfo &info,
                 const std::string &name = "");
    void append(const TypeInfo &info,
                const std::string &name = "");
    void append(const Type &other);

    void pop_front();
    void pop_back();

    Type &operator=(const Type &other);
    Type &operator=(const TypeNode &other);

    bool operator==(const Type &other) const;
    bool operator!=(const Type &other) const;

    const size_t size() const;

    TypeNode &operator[](const int &index) const;

    unsigned long long ID;

    std::list<TypeNode> internal;
};

// An entry in the struct symbol table.
struct StructLookupData
{
    std::map<std::string, Type> members;
    std::list<std::string> order;

    bool erased = false;
};

// Used in the `enumData` map (part of the type symbol table).
struct EnumLookupData
{
    std::map<std::string, Type> options;
    std::list<std::string> order;

    bool erased = false;
};

// Abstract Syntax Tree node. This represents a complete AST in
// itself. This is where the majority of reconstructive and
// transformative work will be done.
struct ASTNode
{
    Type type;
    std::list<ASTNode> items;
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
    unsigned long long line = 0;

    std::set<std::string> tags;
};

// Alias for the symbol table.
typedef std::map<std::string, std::list<MultiTableSymbol>>
    MultiSymbolTable;

// The null type, used for comparisons.
const static Type nullType = {atomic, "NULL"};

#endif
