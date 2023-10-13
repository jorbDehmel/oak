/*
Handles enumerations in Oak Enumerations
(unfortunately) must have templates. There must
be only one enumeration per name, like structs.
The insertion of enums will not be handled by
this file. Enums in Oak are very similar to
structs- The only difference in the signatures
is whether it says 'struct' or 'enum'.
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

extern map<string, __enumLookupData> enumData;
extern vector<string> enumOrder;

#endif
