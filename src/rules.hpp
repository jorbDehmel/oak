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

// An error which may arise during the processing of rules.
class rule_error : public std::runtime_error
{
  public:
    rule_error(const std::string &What) : std::runtime_error(What)
    {
    }
};

// Contains information about a given rule.
struct Rule
{
    std::vector<Token> inputPattern;
    std::vector<Token> outputPattern;

    // The engine hook to call.
    void (*doRule)(std::vector<Token> &, int &, Rule &);
};

// Maps a string engine ID to its corresponding engine function.
extern std::map<std::string, void (*)(std::vector<Token> &, int &, Rule &)> engines;

extern bool doRuleLogFile;
extern std::ofstream ruleLogFile;
extern std::map<std::string, Rule> rules;

extern std::string curFile;

extern std::vector<std::string> activeRules;
extern std::vector<std::string> dialectRules;
extern bool dialectLock;

extern std::map<std::string, std::vector<std::string>> bundles;

// Add a new rule engine.
void addEngine(const std::string &name, void (*hook)(std::vector<std::string> &, int &, Rule &));

// `i` is the point in Lexed at which a macro name was found.
// CONSUMPTIVE on `lexed`.
std::vector<std::string> getMacroArgs(std::vector<Token> &lexed, const int &i);

// Does all active rules on a given token stream.
void doRules(std::vector<Token> &from);

// Load a dialect file.
void loadDialectFile(const std::string &file);

// Internal pass-through for Sapling rule engine.
void doRuleAcorn(std::vector<Token> &tokens, int &i, Rule &toDo);

#endif
