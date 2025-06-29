/**
 * @brief Tests the ASTNodes::Box and OptBox classes
 */

#include "../src/ast_node.hpp"
#include <iostream>
#include <map>
#include <string>

using namespace ASTNodes;
using IntBox = OptBox<int>;

/// Returns (moves) a box instances
IntBox box_returner() {
  return IntBox(123);
}

/// Copies and moves the passed box
IntBox box_stealer(IntBox &_arg) {
  auto copy = _arg;
  return _arg;

  // Copy is destroyed here, but that shouldn't effect _arg
}

// A linked list via opt boxes
struct LLNode {
  int data;
  OptBox<LLNode> next;
};

/// Copies and moves the passed list
LLNode list_stealer(LLNode &_arg) {
  auto copy = _arg;
  return copy;
}

int main() {
  IntBox a(321);
  IntBox b = box_returner();
  IntBox c = box_stealer(a);
  c = box_stealer(c);

  // Construct a linked list
  LLNode head;
  head.data = -1;

  auto ptr = &head;
  for (uint i = 0; i < 128; ++i) {
    ptr->next = LLNode();
    ptr = &ptr->next.get();
    ptr->data = i;
  }

  head = list_stealer(head);

  // Print it
  for (auto ptr = &head; ptr->next.has_value();
       ptr = &ptr->next.get()) {
    std::cout << ptr->data << ' ';
  }
  std::cout << '\n';

  std::map<std::string, LLNode> map_test;

  return 0;
}
