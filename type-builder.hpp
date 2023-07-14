#ifndef TYPE_BUILDER_HPP
#define TYPE_BUILDER_HPP

#include <string>
#include <set>
#include <map>
#include <iostream>
#include <vector>
using namespace std;

class Type;

enum TypeInfo
{
    pointer,   // A pointer to the type specified by next
    templ,     // The previous type is templated with next
    templ_end, // Ends a template
    atomic,    // An atomic type (or struct/class definition)
    join,      // A comma; Syntactic fluff, more or less
    modifier,  // IE const, unsigned, signed, etc...
    function,  // Function start (mostly fluff)
    maps,      // Syntactic fluff for functions
    var_name,  // A variable name for functions
    generic,   // A generic; Akin to atomic
};

// To be expanded with struct definitions
extern set<string> atomics, modifiers;
extern set<string> deps;

class Type
{
public:
    Type(const TypeInfo &Info, const string &Name = "");
    Type(const Type &What);
    Type();

    ~Type();

    void prepend(const TypeInfo &Info, const string &Name = "");
    void append(const TypeInfo &Info, const string &Name = "");
    void append(const Type &Other);

    Type &operator=(const Type &Other);

    bool operator==(const Type &Other) const;
    bool operator!=(const Type &Other) const;

    TypeInfo info;
    string name;
    Type *next = nullptr;
};

extern const Type nullType;

struct __structLookupData
{
    set<string> templatedTypes;
    Type type;
    map<string, Type> members;
};

extern map<string, __structLookupData> structData;

// Returns whether or not a type requires a
// template<whatever> line before it
// NOT if a line takes template arguments!!!!
// IE map<T> does but map<string> does not.
bool isTemplated(Type *T);

// Return the standard C / C++ representation of this type
string toStr(const Type *const What);

vector<string> getTemplate(Type *T);

#endif
