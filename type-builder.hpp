#ifndef TYPE_BUILDER_HPP
#define TYPE_BUILDER_HPP

#include <string>
#include <set>
#include <map>
#include <iostream>
#include <vector>
using namespace std;

#define DB_INFO __FILE__ << ':' << __LINE__ << ' '

enum TypeInfo
{
    pointer,  // A pointer to the type specified by next
    atomic,   // An atomic type (or struct/class definition)
    join,     // A comma; Syntactic fluff, more or less
    function, // Function start (mostly fluff)
    maps,     // Syntactic fluff for functions
    var_name, // A variable name for functions
};

struct typeNode
{
    typeNode &operator=(const typeNode &other);

    TypeInfo info;
    string name;
};

class Type
{
public:
    Type(const TypeInfo &Info, const string &Name = "");
    Type(const Type &What);
    Type(const Type &What, const int &startingAt);
    Type();

    void prepend(const TypeInfo &Info, const string &Name = "");
    void append(const TypeInfo &Info, const string &Name = "");
    void append(const Type &Other);

    void pop_front();
    void pop_back();

    Type &operator=(const Type &Other);
    Type &operator=(const typeNode &Other);

    bool operator==(const Type &Other) const;
    bool operator!=(const Type &Other) const;

    const size_t size() const;

    typeNode &operator[](const int &Index);

    unsigned long long ID;

protected:
    vector<typeNode> internal;

    friend istream &operator>>(istream &strm, Type &type);
    friend string toStr(const Type *const, const unsigned int &pos);
    friend bool typesAreSame(Type *A, Type *B);
    friend bool typesAreSameExact(Type *A, Type *B);
};

////////////////////////////////////////////////////////////////

extern set<string> deps;

extern const Type nullType;

struct __structLookupData
{
    map<string, Type> members;
    vector<string> order;

    bool erased = false;
};

extern map<string, __structLookupData> structData;
extern vector<string> structOrder;

// Return the standard C / C++ representation of this type
string toStr(const Type *const What, const unsigned int &pos = 0);

// Ignores all var_names
// As of 0.0.21, can also do automatic referencing
// of arguments
bool typesAreSame(Type *A, Type *B);

// Like the above, but does not do auto-referencing or dereferencing
bool typesAreSameExact(Type *A, Type *B);

#endif
