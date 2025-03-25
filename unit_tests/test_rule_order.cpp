/*
Tests the graph-based rule ordering/prerequisite system
*/

#include "../src/rule.hpp"
#include <cassert>
#include <iostream>

int main() {
  RuleRunner rr;

  rr.register_rule("exp", Rule{"e", "", {}, "sapling"});
  rr.register_rule("add",
                   Rule{"a", "", {"mult", "div"}, "sapling"});
  rr.register_rule("mult", Rule{"m", "", {"exp"}, "sapling"});
  rr.register_rule("sub",
                   Rule{"s", "", {"mult", "div"}, "sapling"});
  rr.register_rule("div", Rule{"d", "", {"exp"}, "sapling"});

  rr.register_bundle("ooo", {"add", "sub"});

  const auto order = rr.resolve({"ooo"});

  std::cout << "Order of operations according to rule graph:\n";
  for (const auto &item : order) {
    std::cout << item.input_pattern << ' ';
  }
  std::cout << '\n';

  return 0;
}
