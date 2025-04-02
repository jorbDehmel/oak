/**
 * @file
 */

#pragma once

#include "lexer.hpp"
#include <cstdint>
#include <functional>
#include <list>
#include <map>
#include <string>

/**
 * @struct Rule
 * @brief An abstract rule, independent of engine
 */
struct Rule {
  /// Initialize some rule from basics
  Rule(const std::string &_i, const std::string &_o,
       const std::list<std::string> &_r, const std::string &_e);

  /// Copy constructor
  Rule(const Rule &_o)
      : uid(_o.uid), input_pattern(_o.input_pattern),
        output_pattern(_o.output_pattern), prereqs(_o.prereqs),
        engine(_o.engine) {
  }

  /// A unique ID assigned by Oak. Assume that IDs and rules
  /// are 1-to-1.
  const uintmax_t uid;

  /// The un-lexed input pattern
  const std::string input_pattern;

  /// The un-lexed output pattern
  const std::string output_pattern;

  /// Rules that must be done first
  const std::list<std::string> prereqs;

  /// The engine that should be used: Default is sapling
  const std::string engine;
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

  /// Collapse some list of entry points from a dependency
  /// graph to a runnable list. This is for internal use!
  std::list<Rule> resolve(const std::list<std::string> &_rules);

  /// All the things needed to run an arbitrary rule engine
  struct Engine {
    /// Alias to int for finite-state machine states
    using State = uint16_t;

    /// Begin a rule
    std::function<State(const Rule &)> start_rule;

    /// Transition from one state to some new state
    std::function<State(const Rule &, const State &,
                        const Lexer::Token &)>
        state_transition;

    /// Determine if a given state is a match
    std::function<bool(const Rule &, const State &)> is_match;

    /// Transformation function
    std::function<std::list<Lexer::Token>(
        const Rule &, const std::list<Lexer::Token> &)>
        on_match;
  };

protected:
  /// All known engines: You must compile with these!
  const std::map<std::string, Engine> engines;

  /// All known rules
  std::map<std::string, Rule> rules;

  /// All known bundles
  std::map<std::string, std::list<std::string>> bundles;

  /// All currently registered entry points
  std::list<std::string> entry_points;
};
