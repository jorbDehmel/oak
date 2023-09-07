#ifndef TYPE_BUILDER_HPP
#define TYPE_BUILDER_HPP

#include <string>
#include <set>
#include <map>
#include <iostream>
#include <vector>
using namespace std;

#define DB_INFO __FILE__ << ':' << __LINE__ << ' '

class Type;

enum TypeInfo
{
    pointer,  // A pointer to the type specified by next
    atomic,   // An atomic type (or struct/class definition)
    join,     // A comma; Syntactic fluff, more or less
    function, // Function start (mostly fluff)
    maps,     // Syntactic fluff for functions
    var_name, // A variable name for functions
};

// To be expanded with struct definitions
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

    void pop_front();
    void pop_back();

    Type &operator=(const Type &Other);

    bool operator==(const Type &Other) const;
    bool operator!=(const Type &Other) const;

    TypeInfo info;
    string name;
    Type *next = nullptr;
};

// More efficient than Type::pop_front()
void popTypeFront(Type &What);

extern const Type nullType;

struct __structLookupData
{
    map<string, Type> members;
    vector<string> order;

    bool erased = false;
};

/*
struct __templStructLookupData
{
    vector<string> generics;
    vector<string> guts;
};
*/

extern map<string, __structLookupData> structData;
extern vector<string> structOrder;

// Return the standard C / C++ representation of this type
string toStr(const Type *const What);

#endif
