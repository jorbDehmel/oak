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
extern map<string, vector<string>> bundles;

// Active rules should already be present in their vector
void doRules(vector<string> &From);

// To be called by doRules
vector<string> replace(const vector<string> Subset, const rule &Rule);

#endif
