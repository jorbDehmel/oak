/**
 * @file
 * @brief The $apling2 rule engine definitions. Pass 1 is the
 * match, pass 2 gathers match groups, and pass 3 constructs
 * replacement text.
 */

#include "sapling.hpp"
#include "debug.hpp"
#include "lexer.hpp"
#include <cstdint>
#include <stdexcept>

/**
 * @struct CompiledRule
 * @brief The processed version of a Sapling rule. This
 * contains the lexed versions of the input and output
 * rules.
 */
struct CompiledRule {
  /// The broken-apart input rule for pass 1/2. The first entry
  /// is the rule token, while the second item is a list of
  /// pass 2 instructions to parse when ENTERING this node.
  std::vector<std::pair<std::string, std::list<std::string>>>
      lexed_in;

  /// The broken-apart output rule for pass 3
  std::vector<std::string> lexed_out;
};

/// Compiles a rule to store in the cache
CompiledRule compile(const Rule &_from) {
  debug_print();

  CompiledRule out;

  // Break a string along spaces
  const static auto split_on_spaces =
      [](const std::string &_text) {
        std::string cur;
        std::vector<std::string> out;
        for (const char &c : _text) {
          if (c == ' ') {
            if (!cur.empty()) {
              out.push_back(cur);
              cur.clear();
            }
          } else {
            cur.push_back(c);
          }
        }
        if (!cur.empty()) {
          out.push_back(cur);
        }
        return out;
      };

  // Parse a space-broken stream into input pattern
  const static auto break_input_pattern =
      [](const std::vector<std::string> &_in) {
        std::vector<
            std::pair<std::string, std::list<std::string>>>
            out;

        for (auto it = _in.begin(); it != _in.end(); ++it) {
          std::list<std::string> instrs;
          while (it->starts_with("$~") ||
                 it->starts_with("$>")) {
            instrs.push_back(*it);
            ++it;
          }

          out.push_back({*it, instrs});
        }

        return out;
      };

  out.lexed_in =
      break_input_pattern(split_on_spaces(_from.input_pattern));

  out.lexed_out = split_on_spaces(_from.output_pattern);
  return out;
}

/// Fetches from cache, compiling as needed
const CompiledRule &fetch(const Rule &_from) {
  /// Maps rule UIDs to their compiled counterparts
  static std::map<uintmax_t, CompiledRule> cache;

  debug_print();
  if (!cache.contains(_from.uid)) {
    cache.insert_or_assign(_from.uid, compile(_from));
  }
  return cache.at(_from.uid);
}

sapling::State sapling::start_rule(const Rule &_rule) {
  debug_print();
  return 0;
}

bool sapling::is_match(const Rule &_rule, const State &_state) {
  debug_print();
  const CompiledRule &r = fetch(_rule);
  return _state >= r.lexed_in.size();
}

sapling::State
sapling::state_transition(const Rule &_rule,
                          const sapling::State &_current_state,
                          const Lexer::Token &_current_input) {
  debug_print();
  const CompiledRule &r = fetch(_rule);
  const auto thing_to_match =
      r.lexed_in.at(_current_state).first;

  if (thing_to_match.starts_with('$')) {
    if (thing_to_match == "$.") {
      // Single wildcard: Unconditionally advance
      return _current_state + 1;
    }

    throw std::runtime_error(__FUNCTION__ + thing_to_match);
  } else if (_current_input == thing_to_match) {
    // Literal
    return _current_state + 1;
  }

  // Nothing matched
  return 0;
}

std::list<Lexer::Token>
sapling::on_match(const Rule &_rule,
                  const std::list<Lexer::Token> &_match_text) {
  debug_print();
  const CompiledRule &r = fetch(_rule);

  // Pass 2: Rerun match, gather captured data
  std::map<std::string, std::list<std::string>> variables;
  std::list<std::string> memory;
  sapling::State state = 0;

  for (auto it = _match_text.begin(); it != _match_text.end();
       ++it) {
    // Process capture instructions
    for (const auto &instr : r.lexed_in.at(state).second) {
      if (instr == "$~") {
        memory.clear();
      } else if (instr.starts_with("$~")) {
        const std::string var = "$" + instr.substr(2);
        variables[var].clear();
      } else if (instr.starts_with("$>")) {
        const std::string var = "$" + instr.substr(2);
        for (const auto &m : memory) {
          variables[var].push_back(m);
        }
      } else {
        throw std::runtime_error(
            "Unknown input rule instruction '" + instr + "'");
      }
    }

    // Go to next state
    memory.push_back(*it);
    state = state_transition(_rule, state, *it);
  }

  // Pass 3: Reconstruct
  std::list<std::string> to_lex;
  bool merge = false;

  for (const auto &t : r.lexed_out) {
    if (variables.contains(t)) {
      // Variable access
      if (merge) {
        if (to_lex.empty()) {
          to_lex.push_back("");
        }
        for (const auto &v : variables.at(t)) {
          to_lex.back().append(v);
        }
        merge = false;
      } else {
        for (const auto &v : variables.at(t)) {
          to_lex.push_back(v);
        }
      }
    } else if (t == "$<") {
      // Merge tokens
      merge = true;
    } else if (t.front() == '$') {
      throw std::runtime_error("Output rule variable '" + t +
                               "' does not exist.");
    } else {
      if (merge) {
        if (to_lex.empty()) {
          to_lex.push_back(t);
        } else {
          to_lex.back().append(t);
        }
        merge = false;
      } else {
        to_lex.push_back(t);
      }
    }
  }

  // Lex
  Lexer l;
  uint64_t line = _match_text.front().line,
           col = _match_text.front().col;
  std::string text;
  for (const auto &t : to_lex) {
    if (!text.empty()) {
      text.push_back(' ');
    }
    text.append(t);
  }
  return l.lex(text, _match_text.front().file, line, col);
}
