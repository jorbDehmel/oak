/*
Handles generics and templates in Oak source code.

Jordan Dehmel, 2023 - present
jdehmel@outlook.com
*/

#ifndef GENERICS_HPP
#define GENERICS_HPP

#include <list>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "mangler.hpp"
#include "options.hpp"
#include "rules.hpp"
#include "symbol_table.hpp"

// Error representing a fault with generic processing in Oak.
// Usually thrown at instantiation-time for a given template.
class generic_error : public std::runtime_error
{
  public:
    generic_error(const std::string &what) : runtime_error(what)
    {
    }
};

// Info about a single generic template. This can be used to
// instantiate.
struct GenericInfo
{
    std::vector<std::string> typeVec;
    std::string originFile;
    std::vector<Token> symbols;
    std::vector<Token> preBlock, postBlock;
    std::vector<std::string> genericNames;
    std::vector<std::vector<std::vector<std::string>>> instances;
};

// Can throw generic_error's if no viable options exist.
// Ensure all items in genericSubs have been pre-mangled.
// Returns the mangled version.
std::string instantiateGeneric(const std::string &what, const std::vector<std::vector<std::string>> &genericSubs,
                               const std::vector<std::string> &typeVec, AcornSettings &settings);

// Also holds the skeleton of the inst block system, although
// gathering of these happens elsewhere.
void addGeneric(const std::vector<Token> &what, const std::string &name, const std::vector<std::string> &genericsList,
                const std::vector<std::string> &typeVec, const std::vector<Token> &preBlock,
                const std::vector<Token> &postBlock, AcornSettings &settings);

// Print the info of all existing generics to a file stream
void printGenericDumpInfo(std::ostream &to);

#endif
