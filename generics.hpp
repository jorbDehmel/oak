/*
The UNIVERSAL file for dealing with generics in Oak
To be pushed in Oak ~0.0.13 after temporary generic
removal in 0.0.12

Jordan Dehmel, 2023

*/

#ifndef GENERICS_HPP
#define GENERICS_HPP

#include <map>
#include <vector>
#include <stdexcept>
#include <string>

#include "mangler.hpp"
#include "symbol-table.hpp"
#include "enums.hpp"

using namespace std;

// External definition of createSequence, defined in sequence.cpp
// This avoids circular dependencies
sequence createSequence(const vector<string> &From);

class generic_error : public runtime_error
{
public:
    generic_error(const string &what) : runtime_error(what) {}
};

/*
There are two types of generics present in code: Templates
and template shortcuts.

Templates represent the scope of code to be duplicated
upon instantiation. Template shortcuts prompt for
instantiation and are replaced by the mangled name of
the instantiation.

All angle brackets are calls to the instantiator / mangler
unless prefaced by "let".

Sub-mangles only occur during template shortcuts, and thus
should be handled at sequence-time. They will no be
addressed herein.
*/

struct genericInfo
{
    vector<string> symbols;
    vector<string> instBlock;

    vector<string> genericNames;
    bool hasBeenInstantiated = false;
};

// A pair of <name, number_of_generics> maps to a vector of symbols within
extern map<pair<string, int>, genericInfo> generics;

// Can throw generic_error's if no viable options exist.
// Ensure all items in genericSubs have been pre-mangled.
// Returns the mangled version.
string instantiateGeneric(const string &what, const vector<string> &genericSubs);

// Also holds the skeleton of the inst block system, although gathering of these happens elsewhere.
void addGeneric(const vector<string> &what, const string &name, const vector<string> &genericsList, const vector<string> &instBlock = vector<string>());

#endif
