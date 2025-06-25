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

/**
 * @brief An error class thrown when we surpass the PreProcessor
 * Pass limit.
 */
class OutOfPPPLError : public std::runtime_error {
public:
  /// Initialize
  OutOfPPPLError(const std::string &_what)
      : std::runtime_error(_what) {
  }
};

/// Static functions for macro operations
namespace Macros {

/**
 * @brief Runs a command, asserts it succeeded, and captures
 * its stdout.
 * @param _cmd The command to run
 * @returns The string output of the command
 */
std::string get_cmd_output(const std::string &_cmd);

/// Internal oak macros which are deferred to parse time
/// (EG size!, type!)
const static std::set<std::string> reserved_macro_names = {
    "size!",
    "type!",
    "c!",
    "alloc!",
    "free!",
    "compile_time_error!",
    "compile_time_warning!",
    "compile_time_print!",
    "str!",
    "unstr!"};

/// Erases and returns a macro occurrence's args.
std::list<std::list<Lexer::Token>>
get_macro_args(TokenStream &_pos);

/// STRIPS QUOTES OFF OF a macro occurrence's
/// args. Then returns those args WITHOUT ERASURE and
/// WITHOUT recursion! This should only be used after all
/// preprocessing!
std::list<Lexer::Token>
get_macro_args_no_erase(TokenStream &_pos);

/**
 * @brief Strips string literal delimiters off a string
 * literal. For example: "fizz" -> fizz, 'buzz' -> buzz.
 * @param _str_lit The string literal to strip
 * @returns The stripped string literal
 */
std::string strip_string_literal(const std::string &_str_lit);

/**
 * @brief Inverse of strip_string_literal.
 * @param _contents The contents to embed in double quotes
 * @returns The string literal
 */
std::string make_string_literal(const std::string &_contents);

}; // namespace Macros

/**
 * @brief Parses the text once it has been brought to Oak normal
 * form by the rules.
 */
class Parser {
public:
  ///
  Parser(Settings &_s) : settings(_s) {
  }

  ///
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

  /// Parse a global scope. NOTE: All includes should have been
  /// handled already!
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

  /// Parses the (pre, post) regions of a template if they
  /// exist. This should be called after any generic body
  std::pair<std::list<std::string>, std::list<std::string>>
  parse_template_pre_post(TokenStream &_pos);

  /// Assumes we are pointing to the first token in the
  /// statement Non-global (inside functions)
  ASTNodes::Statement
  parse_statement(TokenStream &_pos,
                  const Type &_return_type = {});

  /// Assumes we are pointing to "case" or "else"
  /// Non-global (inside match statement)
  std::variant<ASTNodes::Case, ASTNodes::Statement>
  parse_case(const EnumInfo &_enum_type, TokenStream &_pos,
             const bool &_is_mutable);

  /// Return the type spec at the specified location
  Type parse_type(TokenStream &_pos);

  /// Parses a single function call
  ASTNodes::Node parse_function_call(TokenStream &_pos);

  /// This is what you should call: The other one is called by
  /// this
  ASTNodes::Node parse_object(TokenStream &_pos);

  /// Resolves a function call through any means necessary.
  /// This may involve templates!
  ASTNodes::Call
  resolve_fn_call(const std::string &_name,
                  const std::list<ASTNodes::Object> &_args);

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
  std::variant<InlineMacro, CompiledMacro>
  parse_macro(TokenStream &_pos,
              const uint64_t &_preproc_passes_allowed);

  /**
   * @brief Preprocess until a fixed point is reached. Expect
   * the token stream position to be undefined after.
   */
  uint64_t preprocess(TokenStream &_token_stream);

  /**
   * @brief Load a given dialect file and register it as the
   * current dialect
   */
  void load_dialect_file(const std::filesystem::path &_file);

  /**
   * @brief Parse and turn all math into operator calls
   */
  void fix_math(TokenStream &_token_stream);

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

  ///
  bool instantiate(
      TemplateInfo &_what,
      const std::list<std::list<std::string>> &_substitutions);

  ///
  void replace_macro(TokenStream &_pos);

  friend class OakCompiler;
};
