/**
 * @file parser.hpp
 * @brief Outlines the Parser class used by the Oak compiler.
 * This operates on ALREADY preprocessed symbols. It is
 * responsible for loading the symbol table and collating that
 * which is used for target recconstruction.
 */

#pragma once

#include "lexer.hpp"
#include "type.hpp"
#include <filesystem>
#include <list>
#include <optional>
#include <set>
#include <variant>
#include <vector>

struct Node {
  // NONE is the unit statement: Invalid in non-statement
  // contexts. It will not have any data. IF, WHILE, STMT, CASE,
  // and MATCH will have children, but no type or token.
  // Function calls and variables will be of type OBJECT and
  // have all their data.
  // IF.children    = {condition, body, else?}
  // WHILE.children = {condition, body}
  // MATCH.children = {else, cases...}
  // CASE.children  = {capture, body}
  // STMT.children  = {stmts...}
  // CALL
  enum {
    IF,
    WHILE,
    MATCH,
    CASE,
    OBJECT,
    CALL,
    NONE,
    STMT
  } node_type;

  std::optional<Lexer::Token> token;
  std::optional<Type> type;
  std::vector<Node> children;
};

/**
 * @brief Parses the text once it has been brought to Oak normal
 * form by the rules. Oak normal form is functional with if and
 * while statements.
 */
class Parser {
public:
  // Parse a global scope. NOTE: All includes should have been
  // handled already!
  void
  parse_global(const std::list<Lexer::Token> &_file_contents);

  // Resets the state of the translation unit
  void reset();

  // Constructs the equivalent C program at the given path
  void reconstruct(const std::filesystem::path &_where);

protected:
  // All parse methods leave the iterator pointing to the first
  // token OF the thing parsed, NOT AFTER

  // Parse a single function declaration
  // Assumes we have just seen "let NAME (" and are pointing to
  // the next token.
  void parse_function(
      const std::set<std::string> &_names,
      std::list<Lexer::Token>::const_iterator &_cur_pos,
      const std::list<Lexer::Token>::const_iterator &_end);

  // Parse a single struct declaration
  // Assumes we have just seen "let NAME : struct" and are
  // pointing to the next token.
  void parse_struct(
      const std::set<std::string> &_names,
      std::list<Lexer::Token>::const_iterator &_cur_pos,
      const std::list<Lexer::Token>::const_iterator &_end);

  // Parse a single enum declaration
  // Assumes we have just seen "let NAME : enum" and are
  // pointing to the next token.
  void parse_enum(
      const std::set<std::string> &_names,
      std::list<Lexer::Token>::const_iterator &_cur_pos,
      const std::list<Lexer::Token>::const_iterator &_end);

  // Assumes we are pointing to the first token in the statement
  // Non-global (inside functions)
  Node parse_statement(
      std::list<Lexer::Token>::const_iterator &_cur_pos,
      const std::list<Lexer::Token>::const_iterator &_end);

  // Assumes we are pointing to "case" or "else"
  // Non-global (inside match statement)
  Node parse_case(
      const std::set<std::string> &_names,
      const Type &_enum_type,
      std::list<Lexer::Token>::const_iterator &_cur_pos,
      const std::list<Lexer::Token>::const_iterator &_end);

  // Parses a single object (resolvable variable or function
  // call return value). Assumes we are pointing ot the first
  // token of the object. Non-global (inside statements)
  Node parse_object(
      std::list<Lexer::Token>::const_iterator &_cur_pos,
      const std::list<Lexer::Token>::const_iterator &_end);

  // Resolves a function call through any means necessary. If
  // it cannot be resolved, an error is thrown.
  Type resolve_fn_call(const std::string &_name,
                       const std::vector<Type> &_args);

  // Throws an error on invalid type (EG undefined struct name)
  void validate_type(const Type &_t) const;

  // Symbol table stuff
  struct FnInfo {};
  struct StructInfo {};
  struct EnumInfo {};
  struct ObjInfo {};
  struct TemplateInfo {};

  // Global compiler definitions (EG structs, enums, fns)
  std::map<std::string, std::variant<StructInfo, EnumInfo,
                                     std::list<FnInfo>>>
      definitions;

  // Local scope stack: Empty means global
  std::stack<std::map<std::string, ObjInfo>> locals;

  // All known templates
  std::list<TemplateInfo> templates;
};
