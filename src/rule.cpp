/**
 * @file rule.cpp
 * @brief
 */

#include "rule.hpp"
#include "debug.hpp"
#include "sapling.hpp"
#include <cstdint>
#include <set>
#include <stdexcept>
#include <variant>

RuleRunner::RuleRunner()
    : engines({{"sapling",
                {sapling::state_transition, sapling::on_match,
                 sapling::default_state}}}) {
}

void RuleRunner::register_rule(const std::string &_name,
                               const Rule &_data) {
  debug_print();
  if (registered_rules.contains(_name)) {
    throw std::runtime_error(
        "Cannot reregister rule or bundle '" + _name + "'.");
  }
  registered_rules[_name] = _data;
}

void RuleRunner::register_bundle(
    const std::string &_name,
    const std::list<std::string> &_entails) {
  debug_print();
  if (registered_rules.contains(_name)) {
    throw std::runtime_error(
        "Cannot reregister rule or bundle '" + _name + "'.");
  }
  registered_rules[_name] = _entails;
}

void RuleRunner::remove_entry_point(const std::string &_name) {
  debug_print();
  if (!registered_rules.contains(_name)) {
    throw std::runtime_error(
        "Cannot deregister nonexistant rule or bundle '" +
        _name + "'.");
  }
  registered_rules.erase(_name);
}

void RuleRunner::add_entry_point(const std::string &_name) {
  debug_print();
  entry_points.push_back(_name);
}

void RuleRunner::purge_entry_points() {
  debug_print();
  entry_points.clear();
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
    } else if (!registered_rules.contains(cur)) {
      throw std::runtime_error(
          "Rule or bundle '" + cur +
          "' is required, but does not exist!");
    }

    const auto rule_or_bundle = registered_rules.at(cur);

    if (std::holds_alternative<Rule>(rule_or_bundle)) {
      // Rule
      visited.insert(cur);

      const auto rule = std::get<Rule>(rule_or_bundle);

      if (!engines.contains(rule.engine)) {
        throw std::runtime_error(
            "Rule '" + cur + "' required nonexistant engine '" +
            rule.engine + "'.");
      }

      out.push_front(rule);

      for (const auto &prereq : rule.prereqs) {
        to_visit.push_back(prereq);
      }
    } else {
      // Bundle
      const auto bundle =
          std::get<std::list<std::string>>(rule_or_bundle);
      for (const auto &item : bundle) {
        to_visit.push_front(item);
      }
    }
  }

  return out;
}

uint RuleRunner::process_text(std::list<Lexer::Token> &_what,
                              const uint &_max_passes) {
  debug_print();
  const auto rules = resolve(entry_points);
  uintmax_t pass = 0, rule_pass = 0;
  bool did_change = false;

  do {
    if (pass >= _max_passes) {
      throw std::runtime_error(
          "Ruleset failed to converge within " +
          std::to_string(_max_passes) +
          " meta-passes! A loop of length >1 is likely!");
    }

    // Do rules here
    for (const auto &rule_spec : rules) {
      // Fetch engine details
      const auto engine = engines.at(rule_spec.engine);

      auto pos = _what.begin();
      auto state = engine.default_state;
      auto most_recent_reset = pos;

      while (pos != _what.end()) {
        auto res =
            engine.state_transition(rule_spec, state, *pos);

        if (res.second) {
          // Do replacement
          std::list<Lexer::Token> matched_text;
          matched_text.assign(most_recent_reset,
                              std::next(pos));

          const auto replacement = engine.on_match(
              rule_spec, res.first, matched_text);

          // Pop most recent reset
        } else if (res.first == engine.default_state) {
          // Log as most recent reset
          state = res.first;
        } else {
          // Normal transition
          state = res.first;
        }

        ++pos;
      }

      ++rule_pass;
    }

    ++pass;
  } while (did_change);

  return pass;
}
