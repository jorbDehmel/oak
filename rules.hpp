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

using namespace std;

class rule_error : public runtime_error
{
  public:
    rule_error(const string &What) : runtime_error(What)
    {
    }
};

// Rule message assert
#define rm_assert(expression, message)                                                                                 \
    ((bool)(expression) ? true                                                                                         \
                        : throw rule_error(message " (Failed assertion: '" #expression "') " +                         \
                                           string(strrchr("/" __FILE__, '/') + 1) + " " + to_string(__LINE__)))

struct rule
{
    vector<string> inputPattern;
    vector<string> outputPattern;

    // The function to call
    void (*doRule)(vector<string> &, int &, rule &);
};

// Maps a string engine ID to its corresponding engine function.
extern map<string, void (*)(vector<string> &, int &, rule &)> engines;

// Add a new engine
void addEngine(const string &name, void (*hook)(vector<string> &, int &, rule &));

extern bool doRuleLogFile;
extern map<string, rule> rules;

extern string curFile;

extern vector<string> activeRules;
extern vector<string> dialectRules;
extern bool dialectLock;

extern map<string, vector<string>> bundles;

// I is the point in Lexed at which a macro name was found
// CONSUMPTIVE!
vector<string> getMacroArgs(vector<string> &lexed, const int &i);

// Active rules should already be present in their vector
void doRules(vector<string> &From);

// Load a dialect file
void loadDialectFile(const string &File);

// Internal pass-through for Sapling rule engine
void doRuleAcorn(vector<string> &tokens, int &i, rule &toDo);

#endif
