/*
Jordan Dehmel
2023 - present
jdehmel@outlook.com

Underlying class for representing types in Oak source code.
*/

#ifndef TYPE_BUILDER_HPP
#define TYPE_BUILDER_HPP

#include <array>
#include <iostream>
#include <limits.h>
#include <map>
#include <set>
#include <string>
#include <vector>

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

    void prepend(const TypeInfo &info, const std::string &name = "");
    void append(const TypeInfo &info, const std::string &name = "");
    void append(const Type &other);

    void pop_front();
    void pop_back();

    Type &operator=(const Type &other);
    Type &operator=(const TypeNode &other);

    bool operator==(const Type &other) const;
    bool operator!=(const Type &other) const;

    const size_t size() const;

    // Not const, but does not modify ID
    void reserve(const unsigned long long &to);

    TypeNode &operator[](const int &index) const;

    unsigned long long ID;

    std::vector<TypeNode> internal;
};

////////////////////////////////////////////////////////////////

// An entry in the struct symbol table.
struct StructLookupData
{
    std::map<std::string, Type> members;
    std::vector<std::string> order;

    bool erased = false;
};

// Used in the `enumData` map (part of the type symbol table).
struct EnumLookupData
{
    std::map<std::string, Type> options;
    std::vector<std::string> order;

    bool erased = false;
};

// The null type, used for comparisons.
const static Type nullType = {atomic, "NULL"};

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
