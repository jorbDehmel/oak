#ifndef RULE_STRUCTS_HPP
#define RULE_STRUCTS_HPP

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
    std::string engineName;
};

#endif
