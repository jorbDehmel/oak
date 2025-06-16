/**
 * @file
 * @brief Outlines the Parser class used by the Oak compiler.
 * This operates on ALREADY preprocessed symbols. It is
 * responsible for loading the symbol table and collating that
 * which is used for target reconstruction.
 */

#pragma once

#include "ast_node.hpp"
#include "lexer.hpp"
#include "settings.hpp"
#include "symbols.hpp"
#include "type.hpp"
#include <list>
#include <vector>

/**
 * @brief Parses the text once it has been brought to Oak normal
 * form by the rules.
 */
class Parser {
public:
  ///
  using ScopeFrame = std::map<std::string, Type>;

  /// Parse a global scope. NOTE: All includes should have been
  /// handled already!
  void parse_global(TokenStream &_file_contents,
                    Settings &_settings);

  /// Resolve the given variable. If a name change is needed,
  /// saves it in _new_name. Otherwise, it will contain a
  /// duplicate of _name.text.
  Type resolve_variable(const Lexer::Token &_name,
                        std::string &_new_name);

  /// Constructs the equivalent C program in the given
  /// string stream
  void reconstruct(std::ostream &_where,
                   const Settings::CompileSettings &_csettings)
      const noexcept;

  /// Dump to the given stream
  void dump(std::ostream &_where, TokenStream &_file_contents,
            const Settings::CompileSettings &_csettings)
      const noexcept;

protected:
  // All parse methods leave the iterator pointing to the
  // first token OF the thing parsed, NOT AFTER

  /// Parse a single function declaration
  /// Assumes we have just seen "let NAME (" and are pointing
  /// to "("
  void parse_function(const std::list<std::string> &_names,
                      TokenStream &_pos, Settings &_settings);

  /// Parse a single struct declaration
  /// Assumes we have just seen "let NAME : struct" and are
  /// pointing to the next token.
  void parse_struct(const std::list<std::string> &_names,
                    TokenStream &_pos, Settings &_settings);

  /// Parse a single enum declaration
  /// Assumes we have just seen "let NAME : enum" and are
  /// pointing to the next token.
  void parse_enum(const std::list<std::string> &_names,
                  TokenStream &_pos, Settings &_settings);

  /// Parses a struct/enum's guts
  std::list<std::pair<std::string, Type>>
  parse_members(TokenStream &_pos, Settings &_settings);

  /// Parses the (pre, post) regions of a template if they
  /// exist. This should be called after any generic body
  std::pair<std::list<std::string>, std::list<std::string>>
  parse_template_pre_post(TokenStream &_pos);

  /// Assumes we are pointing to the first token in the
  /// statement Non-global (inside functions)
  ASTNodes::Statement
  parse_statement(TokenStream &_pos, Settings &_settings,
                  const Type &_return_type = {});

  /// Assumes we are pointing to "case" or "else"
  /// Non-global (inside match statement)
  ASTNodes::Case parse_case(const EnumInfo &_enum_type,
                            TokenStream &_pos,
                            Settings &_settings,
                            const bool &_is_mutable);

  /// Return the type spec at the specified location
  Type parse_type(TokenStream &_pos, Settings &_settings);

  /// Parses a single function call
  ASTNodes::Call parse_function_call(TokenStream &_pos,
                                     Settings &_settings);

  /// This is what you should call: The other one is called by
  /// this
  ASTNodes::Object parse_object(TokenStream &_pos,
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
  /// given FUNCTION signature. The second in each pair is the
  /// index of the template for which the first in that pair
  /// provides substitutions.
  std::list<std::pair<TemplateInfo::Substitution, uint>>
  find_substitutions(
      const std::string &_name,
      const std::list<std::string> &_signature) const;

  /// The instance managing all the internal data
  ScopeManager scope_manager;
};
