/**
 * @file sapling.cpp
 * @brief The $apling2 rule engine definitions
 */

#include "sapling.hpp"
#include "debug.hpp"
#include <stdexcept>

struct CompiledRule {
  std::list<std::string> lexed_inp, lexed_out;
  std::map<uint, std::map<std::string, std::pair<uint, bool>>>
      delta;
};

/// Compiles a rule to store in the cache
CompiledRule compile(const Rule &_from) {
  debug_print();
  CompiledRule out;
  std::string cur;

  // Lex input rule
  for (const auto &c : _from.input_pattern) {
    switch (c) {
    case ' ':
      if (!cur.empty()) {
        out.lexed_inp.push_back(cur);
        cur = "";
      }
      break;
    default:
      cur.push_back(c);
      break;
    }
  }
  if (!cur.empty()) {
    out.lexed_inp.push_back(cur);
    cur = "";
  }

  // Lex output rule
  for (const auto &c : _from.output_pattern) {
    switch (c) {
    case ' ':
      if (!cur.empty()) {
        out.lexed_out.push_back(cur);
        cur = "";
      }
      break;
    default:
      cur.push_back(c);
      break;
    }
  }
  if (!cur.empty()) {
    out.lexed_out.push_back(cur);
  }

  // Create delta mapping

  // Note: The translation fn will be interpretted on match
  return out;
}

/// Fetches from cache
const CompiledRule &fetch(const Rule &_from) {
  debug_print();
  // Matches I/O rule pair to compiled version
  static std::map<std::pair<std::string, std::string>,
                  CompiledRule>
      cache;

  if (!cache.contains(
          {_from.input_pattern, _from.output_pattern})) {
    cache[{_from.input_pattern, _from.output_pattern}] =
        compile(_from);
  }
  return cache.at({_from.input_pattern, _from.output_pattern});
}

/**
 * @brief Transition function for Sapling FSTs
 * @param _rule_to_use The specs of the rule being used
 * @param _current_state The current FST state
 * @param _current_input The input token
 * @returns A 2-tuple containing the next state and the exit
 * status bool (true means to apply the transform function)
 */
std::pair<uint, bool>
sapling::state_transition(const Rule &_rule_to_use,
                          const uint &_current_state,
                          const Lexer::Token &_current_input) {
  debug_print();
  const auto &fst = fetch(_rule_to_use);

  if (fst.delta.contains(_current_state)) {
    const auto row = fst.delta.at(_current_state);
    if (row.contains(_current_input.text)) {
      return row.at(_current_input.text);
    } else if (row.contains("")) {
      return row.at("");
    }
  }

  return {sapling::default_state, false};
}

/**
 * @brief Transforms a given text match according to a rule
 * @param _rule Which rule we are looking at
 * @param _match_state The last state returned by the transition
 * function
 * @param _match_text The matched text
 * @returns The text to replace the matched text with
 */
std::list<Lexer::Token>
sapling::on_match(const Rule &_rule, const uint &_match_state,
                  const std::list<Lexer::Token> &_match_text) {
  debug_print();
  // Rerun the input rule in more detail, then interpret the
  // output rule
  throw std::runtime_error(__FUNCTION__);
}
