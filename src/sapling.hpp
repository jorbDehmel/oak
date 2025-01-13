/**
 * @file sapling.hpp
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
 * @brief Transition function for Sapling FSTs as prescribed by
 * rule.hpp
 * @param _current_state The current FST state
 * @param _current_input The input token
 * @param _new_state Delta will save the new state into this
 * @param _is_complete Delta will save the completion state into
 * this: True means the cumulative output will replace
 * everything processed since the last instance of the default
 * state.
 * @param _rule_to_use The specs of the rule being used: It is
 * OK to cache this statically by name.
 */
std::list<Lexer::Token>
delta(const uint &_current_state,
      const Lexer::Token &_current_input, uint &_new_state,
      bool &_is_complete, const Rule &_rule_to_use);

} // namespace sapling
