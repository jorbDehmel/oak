/*
Handles generics and templates in Oak source code.

Jordan Dehmel, 2023 - present
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

class generic_error : public std::runtime_error
{
  public:
    generic_error(const std::string &what) : runtime_error(what)
    {
    }
};

// Internal struct for info
struct genericInfo
{
    std::vector<std::string> typeVec;
    std::string originFile;

    std::vector<token> symbols;
    std::vector<token> preBlock, postBlock;

    std::vector<std::string> genericNames;

    std::vector<std::vector<std::vector<std::string>>> instances;
};

// Avoid using this unless you absolutely must
extern std::map<std::string, std::vector<genericInfo>> generics;

// Can throw generic_error's if no viable options exist.
// Ensure all items in genericSubs have been pre-mangled.
// Returns the mangled version.
std::string instantiateGeneric(const std::string &what, const std::vector<std::vector<std::string>> &genericSubs,
                               const std::vector<std::string> &typeVec);

// Also holds the skeleton of the inst block system, although gathering of these happens elsewhere.
void addGeneric(const std::vector<token> &what, const std::string &name, const std::vector<std::string> &genericsList,
                const std::vector<std::string> &typeVec, const std::vector<token> &preBlock,
                const std::vector<token> &postBlock);

// Print the info of all existing generics to a file stream
void printGenericDumpInfo(std::ostream &to);

#endif
