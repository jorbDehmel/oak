/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#ifndef RULES_HPP
#define RULES_HPP

#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>

#include "lexer.hpp"

using namespace std;

/*
Defines preprocessor rules for Oak

uses simple finite state machine pattern matching,
similar to regex

$v - variable name
$$ - wildcard
*/

class rule_error : public runtime_error
{
public:
    rule_error(const string &What) : runtime_error(What) {}
};

// Rule message assert
#define rm_assert(expression, message)                                     \
    ((bool)(expression)                                                    \
         ? true                                                            \
         : throw rule_error(message                                        \
                            " (Failed assertion: '" #expression "') " +    \
                            string(strrchr("/" __FILE__, '/') + 1) + " " + \
                            to_string(__LINE__)))

struct rule
{
    vector<string> inputPattern;
    vector<string> outputPattern;
};

extern map<string, rule> rules;

extern vector<string> activeRules;
extern vector<string> dialectRules;

extern map<string, vector<string>> bundles;

// I is the point in Lexed at which a macro name was found
// CONSUMPTIVE!
vector<string> getMacroArgs(vector<string> &lexed, const int &i);

// Active rules should already be present in their vector
void doRules(vector<string> &From);

//
void loadDialectFile(const string &File);

#endif
