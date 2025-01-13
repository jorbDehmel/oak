/**
 * @file rule.cpp
 * @brief
 */

#include "rule.hpp"
#include "sapling.hpp"
#include <stdexcept>

RuleRunner::RuleRunner()
    : engines({{"sapling",
                {sapling::default_state, sapling::delta}}}) {}

void RuleRunner::register_rule(const std::string &_name,
                               const Rule &_data) {
  if (registered_rules.contains(_name)) {
    throw std::runtime_error(
        "Cannot reregister rule or bundle '" + _name + "'.");
  }
  registered_rules[_name] = _data;
}

void RuleRunner::register_bundle(
    const std::string &_name,
    const std::list<std::string> &_entails) {
  if (registered_rules.contains(_name)) {
    throw std::runtime_error(
        "Cannot reregister rule or bundle '" + _name + "'.");
  }
  registered_rules[_name] = _entails;
}

void RuleRunner::deregister_rule(const std::string &_name) {
  if (!registered_rules.contains(_name)) {
    throw std::runtime_error(
        "Cannot deregister nonexistant rule '" + _name + "'.");
  }
  registered_rules.erase(_name);
}

void RuleRunner::add_entry_point(const std::string &_name) {
  entry_points.push_back(_name);
}

void RuleRunner::purge_entry_points() { entry_points.clear(); }

uint RuleRunner::process_text(
    std::list<Lexer::Token>::iterator &_begin,
    std::list<Lexer::Token>::iterator &_end,
    const uint &_max_passes) {
  throw std::runtime_error(__FUNCTION__);
}

std::list<Rule> RuleRunner::resolve(const std::string &_rules) {
  throw std::runtime_error(__FUNCTION__);
}
