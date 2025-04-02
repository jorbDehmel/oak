/**
 * @file
 * @brief Outlines the Parser class used by the Oak compiler.
 * This operates on ALREADY preprocessed symbols. It is
 * responsible for loading the symbol table and collating that
 * which is used for target reconstruction.
 */

#pragma once

#include "lexer.hpp"
#include "settings.hpp"
#include "type.hpp"
#include <cstdint>
#include <list>
#include <optional>
#include <set>
#include <variant>
#include <vector>

/**
 * @struct Node
 * @brief A single node in an AST
 */
struct Node {
  /// The AST node type: Used for reconstruction
  enum NodeType {
    IF,
    WHILE,
    MATCH,
    OBJECT,
    CALL,
    NONE,
    DECL,
    RAW_C_FMT, // Format strings w/ `%`
    ARR,
    STMT
  };

  /// The type of this node
  NodeType node_type;

  /// Initialize from some node type
  Node(const NodeType &_node_type) : node_type(_node_type) {
  }

  /// If this node has one, the token
  std::optional<Lexer::Token> token;

  /// Some C literality
  std::optional<std::string> c_name;

  /// The return type of this node, if it has one
  std::optional<Type> type;

  /// The children of this AST node
  std::vector<Node> children;

  /// Given that this is a match statement, is it mutable?
  bool is_mutable_match = false;
};

/**
 * @brief Parses the text once it has been brought to Oak normal
 * form by the rules.
 */
class Parser {
public:
  /// Information about a single function. The type should be
  /// unique.
  struct FnInfo {
    /// Pragma-like tags
    std::map<std::string, std::string> tags;

    /// The fn's FULL type (not just return type)
    Type t;

    /// An AST node defining the behaviour
    Node n = Node(Node::NONE);
  };

  /// Information about a single struct definition.
  struct StructInfo {
    /// Pragma-like tags
    std::map<std::string, std::string> tags;

    /// The order of the members, since that matters
    std::list<std::string> member_order;

    /// The types of the members
    std::map<std::string, Type> members;
  };

  /// Information about a single enum definition
  struct EnumInfo {
    /// Pragma-like tags (EG "casual")
    std::map<std::string, std::string> tags;

    /// The order of the options, since this matters
    std::list<std::string> option_order;

    /// The types of the options
    std::map<std::string, Type> options;
  };

  /// Information about a single template block
  struct TemplateInfo {
    /// Initialize with the info needed to reconstruct (since
    /// instantiated templates take the region of their source,
    /// not their instantiator)
    TemplateInfo(const std::filesystem::path &_p,
                 const uint64_t &_l, const uint64_t &_c)
        : path(_p), line(_l), col(_c) {
    }

    /// The filepath it came from
    const std::filesystem::path path;

    /// The line it came from
    const uint64_t line;

    /// The column it came from
    const uint64_t col;

    /// The things to replace
    std::list<std::string> generics;

    /// "Sample" of body used for auto-instantiation.
    /// "enum"
    /// "struct"
    /// "( whatever : T )"
    std::list<std::string> provides;

    /// Run beforehand: If fail, no error
    std::list<std::string> validate;

    /// Run if valid
    std::list<std::string> instantiate;

    /// Instances which already exist
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
    static std::list<std::string> replace(
        const std::list<std::string> &_to_augment,
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
        Parser &_p,
        const std::list<std::list<std::string>> &_substitutions,
        Settings &_settings);
  };

  /// Parse a global scope. NOTE: All includes should have been
  /// handled already!
  void
  parse_global(const std::list<Lexer::Token> &_file_contents,
               Settings &_settings);

  /// Resolve the given variable
  Type resolve_variable(const Lexer::Token &_name);

  /// Constructs the equivalent C program in the given
  /// stringstream
  void reconstruct(std::ostream &_where,
                   const Settings::CompileSettings &_csettings)
      const noexcept;

  /// Dump to the given stream
  void dump(std::ostream &_where,
            const std::list<Lexer::Token> &_file_contents,
            const Settings::CompileSettings &_csettings)
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
      const std::list<std::string> &_names,
      std::list<Lexer::Token>::const_iterator &_cur_pos,
      const std::list<Lexer::Token>::const_iterator &_end,
      Settings &_settings);

  /// Parse a single struct declaration
  /// Assumes we have just seen "let NAME : struct" and are
  /// pointing to the next token.
  void parse_struct(
      const std::list<std::string> &_names,
      std::list<Lexer::Token>::const_iterator &_cur_pos,
      const std::list<Lexer::Token>::const_iterator &_end,
      Settings &_settings);

  /// Parse a single enum declaration
  /// Assumes we have just seen "let NAME : enum" and are
  /// pointing to the next token.
  void parse_enum(
      const std::list<std::string> &_names,
      std::list<Lexer::Token>::const_iterator &_cur_pos,
      const std::list<Lexer::Token>::const_iterator &_end,
      Settings &_settings);

  /// Parses a struct/enum's guts
  std::list<std::pair<std::string, Type>> parse_members(
      std::list<Lexer::Token>::const_iterator &_cur_pos,
      const std::list<Lexer::Token>::const_iterator &_end,
      Settings &_settings);

  /// Parses the (pre, post) regions of a template if they
  /// exist. This should be called after any generic body
  std::pair<std::list<std::string>, std::list<std::string>>
  parse_template_pre_post(
      std::list<Lexer::Token>::const_iterator &_cur_pos,
      const std::list<Lexer::Token>::const_iterator &_end);

  /// Assumes we are pointing to the first token in the
  /// statement Non-global (inside functions)
  Node parse_statement(
      std::list<Lexer::Token>::const_iterator &_cur_pos,
      const std::list<Lexer::Token>::const_iterator &_end,
      Settings &_settings);

  /// Assumes we are pointing to "case" or "else"
  /// Non-global (inside match statement)
  Node parse_case(
      const EnumInfo &_enum_type,
      std::list<Lexer::Token>::const_iterator &_cur_pos,
      const std::list<Lexer::Token>::const_iterator &_end,
      Settings &_settings, const bool &_is_mutable);

  /// Return the type spec at the specified location
  Type parse_type(
      std::list<Lexer::Token>::const_iterator &_cur_pos,
      const std::list<Lexer::Token>::const_iterator &_end,
      Settings &_settings);

  /// Parses a single function call
  Node parse_function_call(
      std::list<Lexer::Token>::const_iterator &_cur_pos,
      const std::list<Lexer::Token>::const_iterator &_end,
      Settings &_settings);

  /// This is what you should call: The other one is called by
  /// this
  Node parse_object(
      std::list<Lexer::Token>::const_iterator &_cur_pos,
      const std::list<Lexer::Token>::const_iterator &_end,
      Settings &_settings);

  /// Parses a single object (resolvable variable or function
  /// call return value). Assumes we are pointing ot the first
  /// token of the object. Non-global (inside statements), does
  /// not deal with parenthesis.
  Node parse_object_without_paren(
      std::list<Lexer::Token>::const_iterator &_cur_pos,
      const std::list<Lexer::Token>::const_iterator &_end,
      Settings &_settings);

  /// Resolves a function call through any means necessary.
  /// If it cannot be resolved, an error is thrown. Returns
  /// the ENTIRE FN TYPE, not just the return type!
  Type resolve_fn_call(const std::string &_name,
                       const std::vector<Type> &_args,
                       FnInfo &_into, std::vector<int> &_derefs,
                       Settings &_settings,
                       const bool &_allow_template = true);

  /// Throws an error on invalid type (EG undefined struct name)
  void validate_type(const Type &_t) const;

  /// Finds all possible template instantiations to match the
  /// given FUNCTION signature
  std::list<std::pair<std::list<std::list<std::string>>, uint>>
  find_substitutions(
      const std::string &_name,
      const std::list<std::string> &_signature) const;

  /// Pops a `locals` frame and calls destructors. Returns a new
  /// scope node based on the old node followed by all
  /// destructor calls.
  Node pop_frame(const Node &_old_node, Settings &_settings);

  /// Global compiler definitions (EG structs, enums, fns)
  std::map<std::string, std::variant<StructInfo, EnumInfo>>
      globals;

  /// Functions
  std::map<std::string, std::list<FnInfo>> functions;

  /// Local scope stack
  std::list<std::map<std::string, Type>> locals;

  /// All known templates
  std::map<std::string, std::vector<TemplateInfo>> templates;
};
