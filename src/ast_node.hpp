/**
 * @file
 * @brief Abstract Syntax Tree nodes used in program
 * reconstruction
 */

#pragma once

#include <ostream>
#include <string>
#include <vector>

/// An AST node
struct ASTNode {
  std::string text;
  std::vector<ASTNode> children;
};

inline std::ostream &operator<<(std::ostream &_into,
                                const ASTNode &_what) {
  _into << "(" << _what.text;
  for (const auto &child : _what.children) {
    _into << ' ' << child;
  }
  _into << ")";
  return _into;
}

/// Recursively reconstruct AST in C
void reconstruct(const ASTNode &_what, std::ostream &_where);

/// Extracts the type AST from any node, throwing if it is
/// untyped. Use the result in the `Type` class constructor.
ASTNode type(const ASTNode &_what);
