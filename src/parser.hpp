#pragma once

#include "lexer.hpp"
#include <list>
#include <map>
#include <optional>
#include <variant>

struct ASTNode {
  Token cur;
  std::list<ASTNode> children;
};

struct StatementNode {
  ASTNode ast;
  std::map<std::string, StatementNode> next;
};

class Parser {};
