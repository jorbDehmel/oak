/**
 * @file
 */

#pragma once

#include "lexer.hpp"
#include <functional>
#include <list>
#include <map>
#include <string>
#include <variant>

/**
 * @struct Rule
 * @brief An abstract rule, independent of engine
 */
struct Rule {
  /// The un-lexed input pattern
  std::string input_pattern;

  /// The un-lexed output pattern
  std::string output_pattern;

  /// Rules that must be done first
  std::list<std::string> prereqs;

  /// The engine that should be used: Default is sapling
  std::string engine;
};

/**
 * @class RuleRunner
 * @brief Manages and runs rules
 */
class RuleRunner {
public:
  /// Initialize w/ default sapling engine
  RuleRunner();

  /// Register a rule so that it can be run later
  void register_rule(const std::string &_name,
                     const Rule &_data);

  /// Create a new bundle aliasing one name to zero or more
  void register_bundle(const std::string &_name,
                       const std::list<std::string> &_entails);

  /// Remove the given rule from the list of rules to RUN
  void remove_entry_point(const std::string &_name);

  /// Add the given rule to the list of rules to run
  void add_entry_point(const std::string &_name);

  /// Remove all entry points
  std::list<std::string> purge_entry_points();

  /// Iteratively run our rules. Returns whether or not the
  /// token stream has changed
  bool process_text(std::list<Lexer::Token> &_what);

protected:
  /// Collapse some list of entry points from a dependency graph
  /// to a runnable list
  std::list<Rule> resolve(const std::list<std::string> &_rules);

  /// All the things needed to run an arbitrary rule engine FST
  struct Engine {
    /// Matching, but NOT transforming function
    std::function<std::pair<uint, bool>(
        const Rule &, const uint &, const Lexer::Token &)>
        state_transition;

    /// Transformation function
    std::function<std::list<Lexer::Token>(
        const Rule &, const uint &,
        const std::list<Lexer::Token> &)>
        on_match;

    /// The state to start in
    uint default_state = 0;
  };

  /// All known engines: You must compile with these!
  const std::map<std::string, Engine> engines;

  /// All known rules
  std::map<std::string,
           std::variant<Rule, std::list<std::string>>>
      registered_rules;

  /// All currently registered entry points
  std::list<std::string> entry_points;
};
