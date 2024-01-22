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

enum TypeInfo
{
    pointer,  // A pointer to the type specified by next
    atomic,   // An atomic type (or struct/class definition)
    join,     // A comma; Syntactic fluff, more or less
    function, // Function start (mostly fluff)
    maps,     // Syntactic fluff for functions
    var_name, // A variable name for functions
    arr,      // Unsized array. Equivalent to pointer.
    sarr,     // Sized array. like var_name, has size in `name`
};

struct typeNode
{
    typeNode &operator=(const typeNode &other);
    bool operator==(const typeNode &other) const;

    TypeInfo info;
    std::string name;
};

class Type
{
  public:
    Type(const TypeInfo &Info, const std::string &Name = "");
    Type(const Type &What);
    Type(const Type &What, const int &startingAt);
    Type();

    void prepend(const TypeInfo &Info, const std::string &Name = "");
    void append(const TypeInfo &Info, const std::string &Name = "");
    void append(const Type &Other);

    void pop_front();
    void pop_back();

    Type &operator=(const Type &Other);
    Type &operator=(const typeNode &Other);

    bool operator==(const Type &Other) const;
    bool operator!=(const Type &Other) const;

    const size_t size() const;

    // Not const, but does not modify ID
    void reserve(const unsigned long long &To);

    typeNode &operator[](const int &Index) const;

    unsigned long long ID;

    std::vector<typeNode> internal;
};

////////////////////////////////////////////////////////////////

extern std::set<std::string> deps;

extern const Type nullType;

struct __structLookupData
{
    std::map<std::string, Type> members;
    std::vector<std::string> order;

    bool erased = false;
};

extern std::map<std::string, __structLookupData> structData;
extern std::vector<std::string> structOrder;

// Return the standard C / C++ representation of this type
std::string toStr(const Type *const What, const unsigned int &pos = 0);

// Ignores all var_names, plus auto-ref/deref
bool typesAreSame(const Type *const A, const Type *const B, int &changes);

// Like the above, but does not do auto-referencing or dereferencing
bool typesAreSameExact(const Type *const A, const Type *const B);

/*
Compares two types. Returns true if they match exactly, if they
match using auto-reference/auto-dereference, or internal literal
casting. The number of changes is recorded in `changes`.
*/
bool typesAreSameCast(const Type *const A, const Type *const B, int &changes);

/*
Check if a given single token is a literal. Returns the type of
the literal if it is, otherwise nullType.
*/
Type checkLiteral(const std::string &From);

#endif
