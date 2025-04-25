/**
 * @file
 */

#include "rule.hpp"
#include "debug.hpp"
#include "lexer.hpp"
#include "sapling.hpp"
#include <set>
#include <stack>
#include <stdexcept>

// If extra rule engines are provided at compile-time, include
// their headers here
#ifdef EXTRA_RULE_ENGINE_INCLUDES
EXTRA_RULE_ENGINE_INCLUDES
#endif

static uintmax_t next_uid = 0;
Rule::Rule(const std::string &_i, const std::string &_o,
           const std::list<std::string> &_r,
           const std::string &_e)
    : uid(++next_uid), input_pattern(_i), output_pattern(_o),
      prereqs(_r), engine(_e) {
}

RuleRunner::RuleRunner()
    : engines({{"sapling",
                Engine{sapling::start_rule,
                       sapling::state_transition,
                       sapling::is_match, sapling::on_match}},
// If extra rule engines are provided at compile-time, include
// their definitions here
#ifdef EXTRA_RULE_ENGINE_PAIRS
               EXTRA_RULE_ENGINE_PAIRS
#endif
      }) {
}

void RuleRunner::register_rule(const std::string &_name,
                               const Rule &_data) {
  debug_print();
  if (rules.contains(_name) || bundles.contains(_name)) {
    throw std::runtime_error(
        "Cannot reregister rule or bundle '" + _name + "'.");
  }
  rules.emplace(_name, _data);
}

void RuleRunner::register_bundle(
    const std::string &_name,
    const std::list<std::string> &_entails) {
  debug_print();
  if (rules.contains(_name) || bundles.contains(_name)) {
    throw std::runtime_error(
        "Cannot reregister rule or bundle '" + _name + "'.");
  }
  bundles[_name] = _entails;
}

void RuleRunner::remove_entry_point(const std::string &_name) {
  debug_print();
  std::erase(entry_points, _name);
}

void RuleRunner::add_entry_point(const std::string &_name) {
  debug_print();
  entry_points.push_back(_name);
}

std::list<std::string> RuleRunner::purge_entry_points() {
  debug_print();
  const auto to_return = entry_points;
  entry_points.clear();
  return to_return;
}

std::list<Rule>
RuleRunner::resolve(const std::list<std::string> &_rules) {
  debug_print();
  // Build graph where edges mean "needs target before source"
  // Maps source to targets
  std::set<std::string> visited;
  std::list<std::string> to_visit;
  std::map<std::string, std::list<std::string>> graph;

  std::list<Rule> out;

  for (const auto &item : _rules) {
    to_visit.push_back(item);
  }

  while (!to_visit.empty()) {
    const auto cur = to_visit.front();
    to_visit.pop_front();

    if (visited.contains(cur)) {
      continue;
    }

    if (rules.contains(cur)) {
      // Rule
      visited.insert(cur);

      const auto rule = rules.at(cur);

      if (!engines.contains(rule.engine)) {
        throw std::runtime_error(
            "Rule '" + cur + "' required nonexistant engine '" +
            rule.engine + "'.");
      }

      out.push_front(rule);

      for (const auto &prereq : rule.prereqs) {
        to_visit.push_back(prereq);
      }
    } else if (bundles.contains(cur)) {
      // Bundle
      const auto bundle = bundles.at(cur);
      for (const auto &item : bundle) {
        to_visit.push_front(item);
      }
    } else {
      throw std::runtime_error(
          "Rule or bundle '" + cur +
          "' is required, but does not exist!");
    }
  }

  return out;
}

bool RuleRunner::process_text(std::list<Lexer::Token> &_what) {
  debug_print();
  const auto rules = resolve(entry_points);
  bool has_changed = false;

  // Do rules here
  for (const auto &rule_spec : rules) {
    // Fetch engine details
    const auto engine = engines.at(rule_spec.engine);

    const Engine::State original_state =
        engine.start_rule(rule_spec);
    Engine::State state = original_state;
    std::stack<
        std::pair<std::list<Lexer::Token>::iterator, uint>>
        resets;

    for (auto pos = _what.begin(); pos != _what.end(); ++pos) {
      const auto new_state =
          engine.state_transition(rule_spec, state, *pos);
      // Emergency edge case handling: Shouldn't usually happen
      if (resets.empty()) {
        // Log as most recent reset
        state = original_state;
        resets.push({pos, state});
      } else if (engine.is_match(rule_spec, new_state)) {
        has_changed = true;

        // Do replacement
        std::list<Lexer::Token> matched_text;
        matched_text.assign(std::next(resets.top().first),
                            std::next(pos));

        const auto replacement =
            engine.on_match(rule_spec, matched_text);

        _what.erase(std::next(resets.top().first),
                    std::next(pos));
        _what.insert(std::next(resets.top().first),
                     replacement.begin(), replacement.end());

        // Pop most recent reset
        pos = resets.top().first;
        state = resets.top().second;
        resets.pop();
      } else if (new_state == original_state) {
        // Log as most recent reset
        state = new_state;
        resets.push({pos, state});
      } else {
        // Normal transition
        state = new_state;
      }
    }
  }

  return has_changed;
}
