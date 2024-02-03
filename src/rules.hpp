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

#include "options.hpp"
#include "rule_structs.hpp"

// Add a new rule engine.
void addEngine(const std::string &name, void (*hook)(std::vector<Token> &, int &, Rule &, AcornSettings &),
               AcornSettings &settings);

// `i` is the point in Lexed at which a macro name was found.
// CONSUMPTIVE on `lexed`.
std::vector<std::string> getMacroArgs(std::vector<Token> &lexed, const int &i);

// Does all active rules on a given token stream.
void doRules(std::vector<Token> &From, AcornSettings &settings);

// Load a dialect file.
void loadDialectFile(const std::string &File, AcornSettings &settings);

// Internal pass-through for Sapling rule engine.
void doRuleAcorn(std::vector<Token> &From, int &i, Rule &curRule, AcornSettings &settings);

#endif
