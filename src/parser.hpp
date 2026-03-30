/**
 * @file
 * @brief Outlines the Parser class used by the Oak compiler.
 * It is responsible for loading the symbol table and collating
 * that which is used for target reconstruction.
 */

#pragma once

#include "ast_node.hpp"
#include "lexer.hpp"
#include "settings.hpp"
#include "symbols.hpp"
#include "type.hpp"
#include <list>

/**
 * @brief Parses the text once it has been brought to Oak
 * normal form by the rules.
 */
class Parser {
public:
  /// Construct from some settings
  Parser(Settings &_s) : settings(_s) {
  }

  /// A REFERENCE to externally-controlled settings. This is
  /// so that you can centralize the settings for the
  /// compiler, I guess? Seems suboptimal to do it this way.
  Settings &settings;

  /**
   * @brief Given a requested path, return the actual path to
   * (possibly) visit
   * @param _requested The raw path: EG "std/io.oak"
   * @param _cur_file The file which is requesting to resolve
   * the path. This is where all local paths will be from
   * @returns The canonical (fully qualified and standardized)
   * path to visit: Might be local, might be global.
   */
  std::filesystem::path
  resolve_path(const std::string &_requested,
               const std::filesystem::path &_cur_file);

  /// Parse a global scope. NOTE: All includes should have
  /// been handled already!
  void parse_global(TokenStream &_file_contents);

  /// Constructs the equivalent C program in the given
  /// string stream
  void reconstruct(std::ostream &_where) const noexcept;

  /// Dump to the given stream
  void dump(std::ostream &_where,
            TokenStream &_file_contents) const noexcept;

  /// The instance managing all the internal data
  ScopeManager scope_manager;

  // All parse methods leave the iterator pointing to the
  // first token OF the thing parsed, NOT AFTER

  /// Parse a single function declaration
  /// Assumes we have just seen "let NAME (" and are pointing
  /// to "("
  void parse_function(const std::list<std::string> &_names,
                      TokenStream &_pos);

  /// Parse a single struct declaration
  /// Assumes we have just seen "let NAME : struct" and are
  /// pointing to the next token.
  void parse_struct(const std::list<std::string> &_names,
                    TokenStream &_pos);

  /// Parse a single enum declaration
  /// Assumes we have just seen "let NAME : enum" and are
  /// pointing to the next token.
  void parse_enum(const std::list<std::string> &_names,
                  TokenStream &_pos);

  /// Parses a struct/enum's guts
  std::list<std::pair<std::string, Type>>
  parse_members(TokenStream &_pos);

  /// Parses a template and saves it.
  /// Assumes we just saw "let NAMES < SUSPENSIONS >"
  /// and are pointing to the next token ("{")
  void
  parse_template(const std::list<std::string> &_names,
                 const std::list<std::string> &_suspensions,
                 TokenStream &_pos);

  /// Assumes we are pointing to the first token in the
  /// statement Non-global (inside functions)
  ASTNode
  parse_statement(TokenStream &_pos,
                  const std::optional<Type> &_return_type = {});

  /// Assumes we are pointing to "case" or "else"
  /// Non-global (inside match statement)
  ASTNode parse_case(const EnumInfo &_enum_type,
                     TokenStream &_pos,
                     const bool &_is_mutable);

  /// Return the type spec at the specified location
  Type parse_type(TokenStream &_pos);

  /// Called by parse_function_call
  ASTNode
  get_function_call_node(const std::string &_unmangled_name,
                         const std::list<ASTNode> &_args);

  /// Parses a single function call
  ASTNode parse_function_call(TokenStream &_pos);

  /// This is what you should call: The other one is called by
  /// this
  ASTNode parse_object(TokenStream &_pos);

  /// Parses a single or multi-token identifier, handling any
  /// template instantiations therein. Returns an UNMANGLED
  /// and unresolved string. Leaves pointing to first after.
  std::string parse_id(TokenStream &_pos);

  /// Throws an error on invalid type (EG undefined struct
  /// name)
  void validate_type(const Type &_t);

  /// Finds all possible template instantiations to match the
  /// given FUNCTION signature. The second in each pair is the
  /// index of the template for which the first in that pair
  /// provides substitutions.
  std::list<std::pair<TemplateInfo::Substitution, uint>>
  find_substitutions(
      const std::string &_name,
      const std::list<std::string> &_signature) const;

  /// Points to macro name after 'let'. Can be inline or
  /// functional. Erases all traces after done
  void parse_macro(TokenStream &_pos,
                   const uint64_t &_preproc_passes_allowed,
                   const std::list<std::string> &_names);

  /**
   * @brief Load a given dialect file and register it as the
   * current dialect
   */
  void load_dialect_file(const std::filesystem::path &_file);

  /**
   * @brief Load the given file, following any includes found
   * within and doing any preprocessor rules as expected. This
   * is called by do_compilation, and should not be called
   * outside of it!
   */
  void do_file(const std::string &_path,
               const std::filesystem::path &_cur_file);

  /**
   * @brief Tests the input file contents for validity
   */
  void syntax_check(const std::filesystem::path &_fp,
                    const std::string &_text) const;

  /// If debug, prints where we are. The provided text should be
  /// the calling function
  void debug_print_pos(const std::string &_caller,
                       const TokenStream &_pos) const {
    if (settings.debug) {
      settings.ostream
          << _caller << " at " << _pos.cur().file.string()
          << ":" << _pos.cur().line << "." << _pos.cur().col
          << '\n'
          << std::flush;
    }
  }

  /**
   * @brief Attempt to instantiate a template
   * @param _what Information about the template blocks
   * @param _substitutions The values to plug in in place of
   * the generics
   * @param _err_msg If false is returned, this will hold the
   * reason why
   * @returns True iff the blocks were successfully parsed
   */
  bool instantiate(
      TemplateInfo &_what,
      const std::list<std::list<std::string>> &_substitutions,
      std::string &_err_msg);

  /// Replace a macro CALL (not definition) at the given token
  /// stream location. Returns whether or not anything changed
  bool replace_macro(TokenStream &_pos);

  /// Returns a constructor definition function body,
  /// with _where being a sample to copy file/line/col from
  FnInfo get_default_constructor(const StructInfo &_what,
                                 const Lexer::Token &_where);

  /// Returns a destructor definition
  FnInfo get_default_destructor(const StructInfo &_what,
                                const Lexer::Token &_where);

  /// Returns a constructor definition
  FnInfo get_default_constructor(const EnumInfo &_what,
                                 const Lexer::Token &_where);

  /// Returns a destructor definition
  FnInfo get_default_destructor(const EnumInfo &_what,
                                const Lexer::Token &_where);

  /// Returns a list of implementations for each of the wrap_*
  /// functions.
  std::list<FnInfo> get_wrappers(const EnumInfo &_what,
                                 const Lexer::Token &_where);

  friend class OakCompiler;
};
