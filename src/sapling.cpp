/**
 * @file sapling.cpp
 * @brief The $apling2 rule engine definitions
 */

#include "sapling.hpp"
#include <stdexcept>

std::list<Lexer::Token>
sapling::delta(const uint &_current_state,
               const Lexer::Token &_current_input,
               uint &_new_state, bool &_is_complete,
               const Rule &_rule_to_use) {
  throw std::runtime_error("Sapling delta is unimplemented.");
}
