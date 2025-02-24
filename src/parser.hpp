/**
 * @file parser.hpp
 * @brief Outlines the Parser class used by the Oak compiler.
 * This operates on ALREADY preprocessed symbols. It is
 * responsible for loading the symbol table and collating that
 * which is used for target reconstruction.
 */

#pragma once

#include "lexer.hpp"
#include "type.hpp"
#include <cstdint>
#include <filesystem>
#include <list>
#include <optional>
#include <set>
#include <stdexcept>
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
    DECL,
    MODIFIER, // A raw C prefix modifier (EG &, *)
    OTHER,    // Special cases determinable by c_name
    STMT
  } node_type;

  std::optional<Lexer::Token> token;
  std::optional<std::string> c_name; // If different than token
  std::optional<Type> type;
  std::vector<Node> children;
};

/**
 * @brief When thrown, the owner OakC object should print a
 * corresponding error or rethrow.
 */
struct PrintableFileError : std::runtime_error {
  PrintableFileError(const std::string &_msg,
                     const Lexer::Token &_tok)
      : std::runtime_error(_msg), path(_tok.file),
        line(_tok.line), col(_tok.col) {
  }

  const std::filesystem::path path;
  const uint64_t line, col;
};

/**
 * @brief Parses the text once it has been brought to Oak normal
 * form by the rules. Oak normal form is functional with if and
 * while statements.
 */
class Parser {
public:
  /// Information about a single function. The type should be
  /// unique.
  struct FnInfo {
    std::map<std::string, std::string> tags;
    Type t;
    Node n;
  };

  /// Information about a single struct definition.
  struct StructInfo {
    std::map<std::string, std::string> tags;
    std::list<std::string> member_order;
    std::map<std::string, Type> members;
  };

  /// Information about a single enum definition
  struct EnumInfo {
    std::map<std::string, std::string> tags;
    std::list<std::string> option_order;
    std::map<std::string, Type> options;
  };

  /// Information about a single template block
  struct TemplateInfo {
    std::list<std::string> generics;  // The things to replace
    std::list<Lexer::Token> provides; // For auto-instantiation

    std::list<Lexer::Token> validate;    // Run beforehand
    std::list<Lexer::Token> instantiate; // Run if valid

    // Instances which have already existed
    std::set<std::list<std::list<std::string>>>
        existing_instances;

    /// Returns whether the given substitutions would cause the
    /// `provides` list to match the given list
    bool does_provide(
        const std::list<std::list<std::string>> &_substitutions,
        const std::list<std::string> &_desired) const;

    /// Returns a list of tokens based on _to_augment wherein
    /// all occurrences of generics are replaced with their
    /// corresponding replacements
    static std::list<Lexer::Token> replace(
        const std::list<Lexer::Token> &_to_augment,
        const std::list<std::string> &_generics,
        const std::list<std::list<std::string>> &_replacements);

    /// Run the given parser as necessary on this template.
    /// This first checks for existing instances. If none
    /// exist, it replaces and parses the validate block. If
    /// that works, it replaces and parses the instantiate
    /// block. If the instantiate block fails, it raises an
    /// error. If not, the instance is logged and we return
    /// without error. Returns true on full success, false
    /// on failure w/o error
    bool attempt_instantiation(
        Parser &_p, const std::list<std::list<std::string>>
                        &_substitutions);
  };

  /// Parse a global scope. NOTE: All includes should have been
  /// handled already!
  void
  parse_global(const std::list<Lexer::Token> &_file_contents);

  /// Resets the state of the translation unit
  void reset();

  /// Resolve the given variable
  Type resolve_variable(const Lexer::Token &_name);

  /// Constructs the equivalent C program in the given
  /// stringstream
  void reconstruct(std::ostream &_where) const noexcept;

  /// Dump to the given stream
  void dump(std::ostream &_where,
            const std::list<Lexer::Token> &_file_contents)
      const noexcept;

  /// Fetch a symbol
  std::optional<
      std::variant<StructInfo, EnumInfo, std::list<FnInfo>>>
  fetch_symbol(const std::string &_name) const noexcept;

protected:
  // All parse methods leave the iterator pointing to the
  // first token OF the thing parsed, NOT AFTER

  /// Parse a single function declaration
  /// Assumes we have just seen "let NAME (" and are pointing
  /// to the next token.
  void parse_function(
      const std::set<std::string> &_names,
      std::list<Lexer::Token>::const_iterator &_cur_pos,
      const std::list<Lexer::Token>::const_iterator &_end);

  /// Parse a single struct declaration
  /// Assumes we have just seen "let NAME : struct" and are
  /// pointing to the next token.
  void parse_struct(
      const std::set<std::string> &_names,
      std::list<Lexer::Token>::const_iterator &_cur_pos,
      const std::list<Lexer::Token>::const_iterator &_end);

  /// Parse a single enum declaration
  /// Assumes we have just seen "let NAME : enum" and are
  /// pointing to the next token.
  void parse_enum(
      const std::set<std::string> &_names,
      std::list<Lexer::Token>::const_iterator &_cur_pos,
      const std::list<Lexer::Token>::const_iterator &_end);

  /// Parses a struct/enum's guts
  std::list<std::pair<std::string, Type>> parse_members(
      std::list<Lexer::Token>::const_iterator &_cur_pos,
      const std::list<Lexer::Token>::const_iterator &_end);

  /// Assumes we are pointing to the first token in the
  /// statement Non-global (inside functions)
  Node parse_statement(
      std::list<Lexer::Token>::const_iterator &_cur_pos,
      const std::list<Lexer::Token>::const_iterator &_end);

  /// Assumes we are pointing to "case" or "else"
  /// Non-global (inside match statement)
  Node parse_case(
      const std::set<std::string> &_names,
      const Type &_enum_type,
      std::list<Lexer::Token>::const_iterator &_cur_pos,
      const std::list<Lexer::Token>::const_iterator &_end);

  /// Return the type spec at the specified location
  Type parse_type(
      std::list<Lexer::Token>::const_iterator &_cur_pos,
      const std::list<Lexer::Token>::const_iterator &_end);

  /// Parses a single function call
  Node parse_function_call(
      std::list<Lexer::Token>::const_iterator &_cur_pos,
      const std::list<Lexer::Token>::const_iterator &_end);

  /// Parses a single object (resolvable variable or function
  /// call return value). Assumes we are pointing ot the first
  /// token of the object. Non-global (inside statements)
  Node parse_object(
      std::list<Lexer::Token>::const_iterator &_cur_pos,
      const std::list<Lexer::Token>::const_iterator &_end);

  /// Resolves a function call through any means necessary. If
  /// it cannot be resolved, an error is thrown. Returns the
  /// ENTIRE FN TYPE, not just the return type!
  Type resolve_fn_call(const std::string &_name,
                       const std::vector<Type> &_args,
                       FnInfo &_into,
                       std::vector<int> &_derefs);

  /// Throws an error on invalid type (EG undefined struct name)
  void validate_type(const Type &_t) const;

  /// Finds all possible template instantiations to match the
  /// given function call information
  void find_substitutions(
      const std::string &_name,
      const std::vector<Type> &_arg_types,
      std::list<std::pair<std::list<std::list<std::string>>,
                          std::list<TemplateInfo>::iterator>>
          &_candidates) const;

  /// Global compiler definitions (EG structs, enums, fns)
  std::map<std::string, std::variant<StructInfo, EnumInfo>>
      globals;

  /// Functions
  std::map<std::string, std::list<FnInfo>> functions;

  /// Local scope stack
  std::list<std::map<std::string, Type>> locals;

  /// All known templates
  std::list<TemplateInfo> templates;
};
