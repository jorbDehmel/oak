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
#include <set>
#include <stdexcept>

/**
 * @struct CompiledRule
 * @brief The processed version of a Sapling rule. This contains
 * the lexed versions of the input and output rules.
 */
struct CompiledRule {
  /// Starting node
  sapling::State q0;

  /// Ending nodes
  std::set<sapling::State> acceptance_states;

  /// The thing that is actually traversed in pass 1
  std::map<sapling::State,
           std::list<std::pair<std::string, sapling::State>>>
      delta;

  /// The broken-apart input rule for pass 2: Maps states to
  /// their instructions
  std::map<sapling::State, std::list<std::string>> scripting;

  /// The broken-apart output rule for pass 3
  std::vector<std::string> lexed_out;
};

/// Compiles a rule to store in the cache
CompiledRule compile(const Rule &_from) {
  debug_print();

  CompiledRule out;

  // Lex
  const static auto split_on_spaces =
      [](const std::string &_text) -> std::vector<std::string> {
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
  const auto original_lexed_input =
      split_on_spaces(_from.input_pattern);
  out.lexed_out = split_on_spaces(_from.output_pattern);
  out.q0 = 0;

  // Compile to DFA
  throw std::runtime_error("UNIMPLEMENTED");

  // Need to take care of these:
  // out.delta;
  // out.scripting;
  // out.acceptance_states;

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
  const CompiledRule &r = fetch(_rule);
  return r.q0;
}

bool sapling::is_match(const Rule &_rule, const State &_state) {
  debug_print();
  const CompiledRule &r = fetch(_rule);
  return r.acceptance_states.contains(_state);
}

sapling::State
sapling::state_transition(const Rule &_rule,
                          const sapling::State &_current_state,
                          const Lexer::Token &_current_input) {
  debug_print();
  const CompiledRule &r = fetch(_rule);

  if (r.delta.contains(_current_state)) {
    // Literal check
    for (const auto &p : r.delta.at(_current_state)) {
      if (_current_input == p.first) {
        return p.second;
      }
    }

    // Pattern checks
    for (const auto &p : r.delta.at(_current_state)) {
      if (p.first == "$/${ID}/" &&
          _current_input.type == "ID") {
        return p.second;
      } else if (p.first == "$/${OPERATOR}/" &&
                 _current_input.type == "OPERATOR") {
        return p.second;
      } else if (p.first == "$/${STRING}/" &&
                 _current_input.type == "STRING") {
        return p.second;
      } else if (p.first == "$/${NUMBER}/" &&
                 _current_input.type == "NUMBER") {
        return p.second;
      } else if (p.first.starts_with("$/")) {
        throw std::runtime_error(
            "RegEx Sapling cards are unimplemented");
      }
    }
  }

  // Nothing matched
  return r.q0;
}

std::list<Lexer::Token>
sapling::on_match(const Rule &_rule,
                  const std::list<Lexer::Token> &_match_text) {
  debug_print();
  const CompiledRule &r = fetch(_rule);

  // Pass 2: Rerun match, gather captured data
  std::map<std::string, std::list<std::string>> variables;
  std::list<std::string> memory;
  sapling::State state = r.q0;

  for (const auto &tok : _match_text) {
    // Process capture instructions
    memory.push_back(tok);
    for (const auto &instr : r.scripting.at(state)) {
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
      }
    }

    // Go to next state
    state = state_transition(_rule, state, tok);
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
