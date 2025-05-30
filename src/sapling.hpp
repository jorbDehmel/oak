/**
 * @file
 * @brief The $apling2 rule engine
 */

#pragma once

#include "lexer.hpp"
#include "rule.hpp"
#include <sys/types.h>

namespace sapling {

/// Alias for unsigned short. A single rule in a state
/// transition function.
using State = RuleRunner::Engine::State;

/**
 * @brief Return the start state for a rule
 * @param _rule The rule in question
 * @returns The start state
 */
State start_rule(const Rule &_rule);

/**
 * @brief Returns whether some state is a match for a rule
 * @param _rule The rule to check
 * @param _state The state in question
 * @returns Whether the state is a match
 */
bool is_match(const Rule &_rule, const State &_state);

/**
 * @brief Transition function for Sapling
 * @param _rule_to_use The specs of the rule being used
 * @param _current_state The current FST state
 * @param _current_input The input token
 * @returns The next state
 */
State state_transition(const Rule &_rule_to_use,
                       const State &_current_state,
                       const Lexer::Token &_current_input);

/**
 * @brief Transforms a given text match according to a rule
 * @param _rule Which rule we are looking at
 * @param _match_text The matched text
 * @returns The text to replace the matched text with
 */
TokenStream
on_match(const Rule &_rule,
         const std::list<Lexer::Token> &_match_text);

} // namespace sapling
