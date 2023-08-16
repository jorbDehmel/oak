/*
Handles enumerations in Oak Enumerations
(unfortunately) must have templates. There must
be only one enumeration per name, like structs.
The insertion of enums will not be handled by
this file. Enums in Oak are very similar to
structs- The only difference in the signatures
is whether it says 'struct' or 'enum'.

Match statements have not been implemented
yet.
*/

/*
Oak:

let test: struct
{
    a, b, c, d: i32,
}

let main_struct: enum
{
    a: test, // Takes a pre-existing struct
    b: unit, // This is a unit struct
    c: unit,
    d: unit,
}

Cpp:

struct main_struct
{
    enum
    {
        a,
        b,
        c,
        d,
    } __info;

    union
    {
        test a_data;
        unit b_data;
        unit c_data;
        unit d_data;
    } __data;
};
*/

#ifndef ENUMS_HPP
#define ENUMS_HPP

#include <vector>
#include <map>
#include <string>

#include "type-builder.hpp"

using namespace std;

struct __enumLookupData
{
    map<string, Type> options;
    vector<string> order;

    bool erased = false;
};

struct __templEnumLookupData
{
    vector<string> generics;
    vector<string> guts;
};

extern map<string, __enumLookupData> enumData;

// Instantiating a templated enum should function EXACTLY like a struct.
extern map<string, __templEnumLookupData> templEnumData;

#endif
