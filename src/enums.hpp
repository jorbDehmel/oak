/*
Jordan Dehmel, 2023 - present
jdehmel@outlook.com

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

#include <map>
#include <string>
#include <vector>

#include "type_builder.hpp"

// Used in type sizing later
const static char enumTypeStr[] = "i32";

struct __enumLookupData
{
    std::map<std::string, Type> options;
    std::vector<std::string> order;

    bool erased = false;
};

extern std::map<std::string, __enumLookupData> enumData;

#endif
