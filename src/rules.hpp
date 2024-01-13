/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author

Defines preprocessor rules for Oak
*/

#ifndef RULES_HPP
#define RULES_HPP

#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "lexer.hpp"

class rule_error : public std::runtime_error
{
  public:
    rule_error(const std::string &What) : std::runtime_error(What)
    {
    }
};

struct rule
{
    std::vector<token> inputPattern;
    std::vector<token> outputPattern;

    // The function to call
    void (*doRule)(std::vector<token> &, int &, rule &);
};

// Maps a string engine ID to its corresponding engine function.
extern std::map<std::string, void (*)(std::vector<token> &, int &, rule &)> engines;

// Add a new engine
void addEngine(const std::string &name, void (*hook)(std::vector<std::string> &, int &, rule &));

extern bool doRuleLogFile;
extern std::ofstream ruleLogFile;
extern std::map<std::string, rule> rules;

extern std::string curFile;

extern std::vector<std::string> activeRules;
extern std::vector<std::string> dialectRules;
extern bool dialectLock;

extern std::map<std::string, std::vector<std::string>> bundles;

// I is the point in Lexed at which a macro name was found
// CONSUMPTIVE!
std::vector<std::string> getMacroArgs(std::vector<token> &lexed, const int &i);

// Active rules should already be present in their vector
void doRules(std::vector<token> &From);

// Load a dialect file
void loadDialectFile(const std::string &File);

// Internal pass-through for Sapling rule engine
void doRuleAcorn(std::vector<token> &tokens, int &i, rule &toDo);

#endif
