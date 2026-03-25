/**
 * @file
 * @brief Overflow for non-class-member parser stuff
 */

#pragma once

#include "lexer.hpp"
#include "settings.hpp"
#include "symbols.hpp"

/// Prints the previous _n lines, followed by the current line
/// and an indicator to the current token
void print_region(TokenStream &_pos, std::ostream &_where,
                  const uint &_n = 1);
/**
 * @brief Gets the real name of a template instantiation
 * @param _name The name of the template
 * @param _substitutions The generics being inserted
 * @returns The template-mangled (but not type-mangled) name
 */
std::string build_generic_prefix(
    const std::string &_name,
    const TemplateInfo::Substitution &_substitutions);

/// Concatenates a token list to a string
std::string concat(const std::list<Lexer::Token> &_what);

/**
 * @brief Determines if a name is valid for a struct/enum
 * @param _warn_info Where to write a warning if invalid
 * @param _type_str EG "Struct", "Enum", "Generic", etc
 * @param _name The name to analyze
 */
void check_camelcase(Settings &_warn_into,
                     const std::string &_type_str,
                     const std::string &_name,
                     const Lexer::Token &_where) noexcept;

/**
 * @brief Runs a command (asserting that it succeeded),
 * logging cout to the returned string
 * @param _cmd The system command to execute
 * @returns The output of that command, given that it
 * succeeded
 */
std::string get_cmd_output(const std::string &_cmd);

/// Static functions for macro operations
namespace Macros {

/**
 * @brief Runs a command, asserts it succeeded, and captures
 * its stdout.
 * @param _cmd The command to run
 * @returns The string output of the command
 */
std::string get_cmd_output(const std::string &_cmd);

/// Internal oak macros
const static std::set<std::string> reserved_macro_names = {
    "alias!",
    "c!",
    "compile_time_error!",
    "compile_time_print!",
    "compile_time_system!",
    "compile_time_warning!",
    "erase!",
    "flag!",
    "include!",
    "link!",
    "namespace_use!",
    "pragma!",
    "rule_bundle!",
    "rule_new!",
    "rule_remove!",
    "rule_use!",
    "size!",
    "str!",
    "type!",
    "unstr!",
    "LINE!",
    "COL!",
    "FILE!",
    "oak_VERSION!",
    "SYSTEM!",
};

/// Goes past and returns a macro occurrence's arguments
std::list<std::list<Lexer::Token>>
get_macro_args(TokenStream &_pos);

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
