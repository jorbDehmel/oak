/**
 * @file
 * @brief The $apling2 rule engine
 */

#pragma once

#include "lexer.hpp"
#include "rule.hpp"
#include <sys/types.h>

namespace sapling {

/// The entry state of the Sapling FST
const static uint default_state = 0;

/**
 * @brief Transition function for Sapling FSTs
 * @param _rule_to_use The specs of the rule being used
 * @param _current_state The current FST state
 * @param _current_input The input token
 * @returns A 2-tuple containing the next state and the exit
 * status bool (true means to apply the transform function)
 */
std::pair<uint, bool>
state_transition(const Rule &_rule_to_use,
                 const uint &_current_state,
                 const Lexer::Token &_current_input);

/**
 * @brief Transforms a given text match according to a rule
 * @param _rule Which rule we are looking at
 * @param _match_state The last state returned by the transition
 * function
 * @param _match_text The matched text
 * @returns The text to replace the matched text with
 */
std::list<Lexer::Token>
on_match(const Rule &_rule, const uint &_match_state,
         const std::list<Lexer::Token> &_match_text);

} // namespace sapling
