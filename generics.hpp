/*
Handles generics and templates in Oak source code.

Jordan Dehmel, 2023
jdehmel@outlook.com
*/

#ifndef GENERICS_HPP
#define GENERICS_HPP

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "enums.hpp"
#include "mangler.hpp"
#include "rules.hpp"
#include "symbol_table.hpp"

using namespace std;

extern unsigned long long int curLine;

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

class generic_error : public runtime_error
{
  public:
    generic_error(const string &what) : runtime_error(what)
    {
    }
};

// Internal struct for info
struct genericInfo
{
    vector<string> typeVec;
    string originFile;

    vector<string> symbols;
    vector<string> needsBlock;

    vector<string> genericNames;

    vector<vector<vector<string>>> instances;
};

// Avoid using this unless you absolutely must
extern map<string, vector<genericInfo>> generics;

// Can throw generic_error's if no viable options exist.
// Ensure all items in genericSubs have been pre-mangled.
// Returns the mangled version.
string instantiateGeneric(const string &what, const vector<vector<string>> &genericSubs, const vector<string> &typeVec);

// Also holds the skeleton of the inst block system, although gathering of these happens elsewhere.
void addGeneric(const vector<string> &what, const string &name, const vector<string> &genericsList,
                const vector<string> &instBlock, const vector<string> &typeVec);

// Print the info of all existing generics to a file stream
void printGenericDumpInfo(ostream &to);

#endif
