/**
 * @file rule.hpp
 * @brief Non-length-preserving Iterated uniform finite-state
 * transducers operating with the set of all valid Tokens as the
 * alphabet. These are FSTs such that replacement text is
 * yielded only upon completion.
 */

#pragma once

#include "lexer.hpp"
#include <functional>
#include <list>
#include <map>
#include <string>
#include <variant>

static_assert(__cplusplus >= 2020'00ULL);

struct Rule {
  std::string input_pattern;
  std::string output_pattern;
  std::list<std::string> prereqs;
  std::string engine;
};

class RuleRunner {
public:
  RuleRunner();

  void register_rule(const std::string &_name,
                     const Rule &_data);
  void register_bundle(const std::string &_name,
                       const std::list<std::string> &_entails);
  void deregister_rule(const std::string &_name);
  void add_entry_point(const std::string &_name);
  void purge_entry_points();

  uint process_text(std::list<Lexer::Token>::iterator &_begin,
                    std::list<Lexer::Token>::iterator &_end,
                    const uint &_max_passes);

protected:
  std::list<Rule> resolve(const std::list<std::string> &_rules);

  // All the things needed to run an arbitrary rule engine FST
  struct Engine {
    // Matching, but NOT transforming function
    std::function<std::pair<uint, bool>(
        const Rule &, const uint &, const Lexer::Token &)>
        state_transition;

    // Transformation function
    std::function<std::list<Lexer::Token>(
        const Rule &, const uint &,
        const std::list<Lexer::Token> &)>
        on_match;

    // The state to start in
    uint default_state = 0;
  };

  const std::map<std::string, Engine> engines;
  std::map<std::string,
           std::variant<Rule, std::list<std::string>>>
      registered_rules;
  std::list<std::string> entry_points;
};
