/**
 * @file
 * @brief The $apling2 rule engine definitions
 */

#include "sapling.hpp"
#include "debug.hpp"
#include "lexer.hpp"
#include <set>
#include <stdexcept>

/**
 * @struct CompiledRule
 * @brief The processed version of a Sapling rule. This contains
 * the lexed versions of the input and output rules.
 */
struct CompiledRule {
  /// The broken-apart input rule
  std::vector<std::string> lexed_inp;

  /// The broken-apart output rule
  std::vector<std::string> lexed_out;
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
 * @brief Internal lambda-based transition function
 * @param _r The *compiled* version of the rule
 * @param _current_state The current state int
 * @param _current_input The token we are processing
 * @param _on_unknown_special_token When we see an unknown
 * special token (EG $~ $>a), we pass it to this lambda. If it
 * returns, we incr past the unknown token. If the token is
 * truly invalid, the lambda should throw an error.
 * @returns A 2-tuple of the new state and whether this was a
 * match
 */
std::pair<uint, bool>
__state_transition(const CompiledRule &_r,
                   const uint &_current_state,
                   const Lexer::Token &_current_input,
                   std::function<void(const std::string &)>
                       _on_unknown_special_token) {
  debug_print();
  auto state = _current_state;

  // Reach a valid state
  const static std::set<std::string> skip_toks = {"$(", "$)"};
  while (state < _r.lexed_inp.size() &&
         skip_toks.contains(_r.lexed_inp.at(state))) {
    ++state;
  }

  if (state >= _r.lexed_inp.size()) {
    throw std::runtime_error(
        "Invalid state reached in Sapling input rule!");
  }

  // Interpret one input step
  const auto cur = _r.lexed_inp.at(state);
  uint new_state = state;
  if (cur.front() == '$') {
    // Special cases
    if (cur.starts_with("$$")) {
      // Literal dollar sign prefix
      if (_current_input == cur.substr(1)) {
        ++new_state;
      } else {
        new_state = sapling::default_state;
      }
    } else if (cur == "$.") {
      // Wildcard
      ++new_state;
    } else if (cur == "$*") {
      // Dot-star

      // Get next literal
      uint i = new_state + 1;
      while (i < _r.lexed_inp.size() &&
             _r.lexed_inp.at(i).starts_with("$")) {
        ++i;
      }

      if (i >= _r.lexed_inp.size()) {
        throw std::runtime_error("Sapling '$*' must eventually "
                                 "be followed by a literal");
      }

      if (_current_input == _r.lexed_inp.at(i)) {
        ++new_state;
      }
    }

    // Invalid special token
    else {
      _on_unknown_special_token(cur);
      ++new_state;
    }
  } else {
    // Literal
    if (_current_input == cur) {
      ++new_state;
    } else {
      new_state = sapling::default_state;
    }
  }

  // Return the new determined state
  // NOTE: Attempting to continue matching on an
  // already-matched rule is intentionally undefined!
  return {new_state, new_state == _r.lexed_inp.size()};
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
  const CompiledRule &rule = fetch(_rule_to_use);
  return __state_transition(
      rule, _current_state, _current_input,
      [](const std::string &_tok) {
        if (_tok == "$~" || _tok.substr(0, 2) == "$>") {
          return;
        }
        throw std::runtime_error(
            "Invalid sapling special token '" + _tok + "'");
      });
}

/**
 * @brief Transforms a given text match according to a rule
 * @param _rule Which rule we are looking at
 * @param _match_state The last state returned by the
 * transition function
 * @param _match_text The matched text
 * @returns The text to replace the matched text with
 */
std::list<Lexer::Token>
sapling::on_match(const Rule &_rule, const uint &_match_state,
                  const std::list<Lexer::Token> &_match_text) {
  debug_print();
  const auto &fst = fetch(_rule);
  // Rerun the input rule in more detail, then interpret the
  // output rule
  // Variables are things like `$foo`
  std::map<std::string, std::list<std::string>> vars;
  std::list<std::string> memory;
  uint state = sapling::default_state;

  for (const auto &tok : _match_text) {
    state = __state_transition(
                fst, state, tok,
                [&](const std::string &_tok) {
                  if (_tok == "$~") {
                    memory.clear();
                    return;
                  } else if (_tok.substr(0, 2) == "$>") {
                    const std::string var_name =
                        "$" + _tok.substr(2);
                    for (const auto &item : memory) {
                      vars[item].push_back(item);
                    }
                    return;
                  }
                  throw std::runtime_error(
                      "Invalid sapling special token '" + _tok +
                      "'");
                })
                .first;
  }

  // Run replacement
  Lexer::Token template_token = _match_text.front();
  std::list<Lexer::Token> replacement;
  for (const auto &tok : fst.lexed_out) {
    // Variable
    if (vars.contains(tok)) {
      for (const auto &item : vars.at(tok)) {
        replacement.push_back(
            Lexer::Token(template_token, item));
      }
    } else {
      // Literal
      replacement.push_back(Lexer::Token(template_token, tok));
    }
  }

  // Output replacement
  return replacement;
}
