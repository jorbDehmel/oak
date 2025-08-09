/**
 * @file
 */

#include "parser.hpp"
#include "ast_node.hpp"
#include "compiler.hpp"
#include "debug.hpp"
#include "lexer.hpp"
#include "settings.hpp"
#include "symbols.hpp"
#include "type.hpp"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>

/// Prints the previous _n lines, followed by the current line
/// and an indicator to the current token
void print_region(TokenStream &_pos, std::ostream &_where,
                  const uint &_n = 1) {
  debug_print();
  auto start_pos = _pos.tell();

  const auto start_line = _pos.cur().line;
  const auto start_col = _pos.cur().col;

  // Go to first token BEFORE our region
  while (!_pos.at_beg() && _pos.cur().line + _n >= start_line) {
    _pos.prev();
  }

  // Go to first token OF our region
  _pos.next();

  // Top delim
  _where << "v";
  for (uint i = 0; i < 64 - 2; ++i) {
    _where << '~';
  }
  _where << "v\n";

  // Print lines
  uint line = _pos.cur().line, col = 0;
  while (!_pos.done() && _pos.cur().line <= start_line) {
    if (!_pos.cur().original) {
      _pos.next();
      continue;
    }

    // Get to correct line
    if (_pos.cur().line != line) {
      _where << '\n';
      line = _pos.cur().line;
      col = 0;
    }

    // Get to correct column
    while (col < _pos.cur().col) {
      _where << ' ';
      ++col;
    }

    // Print text
    _where << _pos.cur().text;

    // Advance
    col = _pos.cur().col + _pos.cur().text.size();
    _pos.next();
  }

  // Print indicator
  _where << '\n';
  for (uint i = 0; i < start_col; ++i) {
    _where << ' ';
  }
  _where << "^\n";

  // Bottom indicator
  _where << "^";
  for (uint i = 0; i < 64 - 2; ++i) {
    _where << '~';
  }
  _where << "^\n";

  _pos.seek(start_pos);
}

/// Concatenates a token list to a string
std::string concat(const std::list<Lexer::Token> &_what) {
  std::string out = "";
  bool first = true;
  for (const auto &tok : _what) {
    if (first) {
      first = false;
    } else {
      out += " ";
    }
    out += tok.text;
  }
  return out;
}

/**
 * @brief Determines if a name is valid for a struct/enum
 * @param _name The name to analyze
 * @returns True iff _name is a valid struct name
 */
static bool
is_valid_struct_name(const std::string &_name) noexcept {
  // The final chunk after any underscores/namespace operators
  const auto end = std::min(_name.find("_GEN"), _name.size());
  const auto pos = _name.find_last_of('_', end);
  uint i = (pos == std::string::npos ? 0 : pos);

  // Must be camelcase
  for (; i < end; ++i) {
    // A single uppercase
    if (std::isupper(_name[i])) {
      // Followed by zero or more non-uppercase
      while (i + 1 < _name.size() &&
             !std::isupper(_name[i + 1])) {
        ++i;
      }
    } else {
      return false;
    }
  }

  return true;
}

void Parser::parse_global(TokenStream &_pos) {
  debug_print();
  if (settings.debug) {
    settings.ostream << __FUNCTION__ << " at "
                     << _pos.cur().file.string() << ":"
                     << _pos.cur().line << "." << _pos.cur().col
                     << '\n';
  }

  // Iterate and delegate. No macros remain.
  while (!_pos.done()) {
    try {
      parse_statement(_pos);
      while (!_pos.done() && _pos.cur().text == ";") {
        _pos.next();
      }
    } catch (OutOfPPPLError &) {
      throw;
    } catch (std::runtime_error &e) {
      print_region(_pos, settings.ostream, 3);
      throw std::runtime_error(
          "At " + _pos.cur().file.string() + ":" +
          std::to_string(_pos.cur().line) + "." +
          std::to_string(_pos.cur().col) + "\n" + e.what());
    } catch (...) {
      print_region(_pos, settings.ostream, 3);
      throw std::runtime_error(
          "At " + _pos.cur().file.string() + ":" +
          std::to_string(_pos.cur().line) + "." +
          std::to_string(_pos.cur().col) +
          "\nUnknown error during global-scope parsing");
    }
  }
}

// Constructs the equivalent C program at the given path
void Parser::reconstruct(std::ostream &_where) const noexcept {
  debug_print();

  // Include std header
  _where << "#include \"oak/std/std_oak_header.h\"\n";

  // Struct, fn, and enum signatures
  for (const auto &entry : scope_manager.in_order) {
    if (std::holds_alternative<StructInfo>(entry)) {
      _where << "struct " << std::get<StructInfo>(entry).name
             << ";\n";
    } else if (std::holds_alternative<EnumInfo>(entry)) {
      _where << "struct " << std::get<EnumInfo>(entry).name
             << ";\n";
    } else if (std::holds_alternative<FnInfo>(entry)) {
      const auto info = std::get<FnInfo>(entry);
      if (info.name == "main") {
        if (!info.tags.contains("file") ||
            info.tags.at("file") ==
                settings.compile_settings().entry_point) {
          _where << info.t.c_repr(info.name, true) << ";\n";
        }
      } else {
        _where << info.t.c_repr(info.name, false) << ";\n";
      }
    }
  }

  // Struct, fn, and enum definitions
  for (const auto &data : scope_manager.in_order) {
    if (std::holds_alternative<StructInfo>(data)) {
      auto info = std::get<StructInfo>(data);

      _where << "// #line " << info.tags["line"] << " \""
             << info.tags["file"] << "\"\n";

      _where << "struct " << info.name << " {\n";
      for (const auto &item : info.member_order) {
        _where << info.members.at(item).c_repr(item) << ";\n";
      }
      _where << "};\n";
    } else if (std::holds_alternative<EnumInfo>(data)) {
      auto info = std::get<EnumInfo>(data);

      _where << "// " << info.tags["file"] << ":"
             << info.tags["line"] << "\n";

      _where << "struct " << info.name << " {\nenum {\n";
      for (const auto &item : info.option_order) {
        _where << info.name << "_OPT_" << item << ",\n";
      }
      _where << "} __info;\nunion {\n";
      for (const auto &item : info.option_order) {
        _where << info.options.at(item).c_repr(item) << ";\n";
      }
      _where << "} __data;\n};\n";
    } else if (std::holds_alternative<FnInfo>(data)) {
      auto info = std::get<FnInfo>(data);
      if (info.tags.contains("casual") &&
          info.tags.at("casual") == "true") {
        continue;
      } else if (info.tags.contains("autogen") &&
                 info.tags.at("autogen") == "true") {
        _where << "// autogen\n";
      }

      _where << "// " << info.tags["file"] << ":"
             << info.tags["line"] << "\n";

      if (info.name == "main") {
        if (!info.tags.contains("file") ||
            info.tags.at("file") ==
                settings.compile_settings().entry_point) {
          _where << info.t.c_repr(info.name, true);
          _where << "{";
          ASTNodes::reconstruct(info.n, _where);
          _where << "}\n";
        }
      } else {
        _where << info.t.c_repr(info.name, false);
        _where << "{";
        ASTNodes::reconstruct(info.n, _where);
        _where << "}\n";
      }
    }
  }
}

void Parser::dump(std::ostream &_where,
                  TokenStream &_pos) const noexcept {
  debug_print();

  _where << "// Lexed contents of file " << _pos.cur().file;

  uint64_t cur_line = 0;
  for (const auto &cur : _pos) {
    if (cur_line == cur.line) {
      _where << ' ' << cur.text;
    } else {
      _where << '\n' << cur_line << "\t|";
    }
  }

  _where << "\n// Attempted reconstruction\n";

  try {
    reconstruct(_where);
  } catch (std::runtime_error &e) {
    _where << "// FAILURE: " << e.what() << '\n';
  } catch (...) {
    _where << "// UNKNOWN FAILURE\n";
  }

  _where << "// Entries:\n";
  for (const auto &data : scope_manager.in_order) {
    if (std::holds_alternative<StructInfo>(data)) {
      const auto info = std::get<StructInfo>(data);
      _where << info.name << " [";
      for (auto it = info.tags.begin(); it != info.tags.end();
           ++it) {
        if (it != info.tags.begin()) {
          _where << ", ";
        }
        _where << "{\"" << it->first << "\": \"" << it->second
               << "\"}";
      }
      _where << "]\n";
      for (const auto &member : info.member_order) {
        _where << "\t"
               << info.members.at(member).oak_repr(member)
               << '\n';
      }
    } else if (std::holds_alternative<EnumInfo>(data)) {
      const auto info = std::get<EnumInfo>(data);
      _where << info.name << " [";
      for (auto it = info.tags.begin(); it != info.tags.end();
           ++it) {
        if (it != info.tags.begin()) {
          _where << ", ";
        }
        _where << "{\"" << it->first << "\": \"" << it->second
               << "\"}";
      }
      _where << "]\n";
      for (const auto &member : info.option_order) {
        _where << "\t"
               << info.options.at(member).oak_repr(member)
               << '\n';
      }
    } else {
      const auto info = std::get<FnInfo>(data);
      _where << info.name << ":\n\t"
             << info.t.oak_repr(info.name) << '\n';
    }
  }
}

// Parse a single function declaration
// Assumes we have just seen "let NAME (" and are pointing to
// "("
void Parser::parse_function(
    const std::list<std::string> &_names, TokenStream &_pos) {
  debug_print();
  if (settings.debug) {
    settings.ostream
        << __FUNCTION__ << " at " << _pos.cur().file.string()
        << ":" << _pos.cur().line << "." << _pos.cur().col
        << " '" << _pos.cur().text
        << "'\nParsing body/signature for function(s):\n";
    for (const auto &name : _names) {
      settings.ostream << " - " << name << '\n';
    }
  }

  // Finish parsing type
  Type t = parse_type(_pos);
  _pos.next();

  // Special case type restrictions
  for (const auto &name : _names) {
    if (name == "main") {
      const auto args = t.fn_args();
      const auto ret = t.fn_return_type();
      if (!args.empty()) {
        if (args.size() != 2) {
        } else if (!args.front().second.exact_match(
                       Type({"i32"}))) {
          throw std::runtime_error(
              "If provided, " + name +
              "'s first argument (argc) should be of type i32");
        } else if (!args.back().second.exact_match(
                       Type({"[", "]", "[", "]", "i8"}))) {
          throw std::runtime_error("If provided, " + name +
                                   "'s second argument (argv) "
                                   "should be of type [][]i8");
        }
      }
      if (!ret.exact_match(Type({"i32"}))) {
        throw std::runtime_error(
            name + " must have a return type of i32");
      }
    } else if (name == "New" || name == "Del") {
      const auto args = t.fn_args();
      const auto ret = t.fn_return_type();
      if (args.size() != 1) {
        throw std::runtime_error(
            name + " must take only one argument: A pointer to "
                   "the object to act upon");
      } else if (args.front().second.nodes.empty() ||
                 args.front().second.nodes.front().type !=
                     Type::TypeNode::POINTER) {
        throw std::runtime_error(
            name + " must take a pointer as its argument");
      } else if (!ret.exact_match(Type({"void"}))) {
        throw std::runtime_error(
            name + " must have a void return type");
      }
    }
  }

  // Either signature or implementation
  FnInfo to_add;
  to_add.name = "FN_NAME_NOT_PROVIDED";
  to_add.t = t;

  to_add.tags["file"] = _pos.cur().file;
  to_add.tags["line"] = std::to_string(_pos.cur().line);
  to_add.tags["col"] = std::to_string(_pos.cur().col);

  if (_pos.cur() == ";") {
    // Signature
    to_add.tags["casual"] = "true";
  } else {
    // Implementation

    // Add some signatures for recursion
    FnInfo temp_info = to_add;
    temp_info.tags["casual"] = "true";
    for (const auto &name : _names) {
      temp_info.name = name;
      scope_manager.add(name, temp_info);
    }

    // Push stack frame w/ args
    scope_manager.push_frame();

    auto args = t.fn_args();
    for (const auto &p : args) {
      scope_manager.add(p.first, p.second);
    }

    const auto backup =
        settings.compile_settings().cur_return_type;
    settings.compile_settings().cur_return_type =
        t.fn_return_type();

    to_add.n = parse_statement(_pos);

    settings.compile_settings().cur_return_type = backup;

    // Pop stack frame WITHOUT CALLING ARGUMENT DESTRUCTORS
    scope_manager.pop_frame();
  }

  for (const auto &name : _names) {
    to_add.name = name;
    scope_manager.add(name, to_add);
  }

  if (settings.debug) {
    scope_manager.dump(settings.ostream);
  }
}

// Parses a struct/enum's guts
std::list<std::pair<std::string, Type>>
Parser::parse_members(TokenStream &_pos) {
  debug_print();
  if (settings.debug) {
    settings.ostream << __FUNCTION__ << " at "
                     << _pos.cur().file.string() << ":"
                     << _pos.cur().line << "." << _pos.cur().col
                     << '\n';
  }

  // Where to write output
  std::list<std::pair<std::string, Type>> out;

  // For keeping track of used names
  std::set<std::string> all_names;

  _pos.next();

  while (_pos.cur() != "}") {
    // One or more comma-separated names
    std::list<std::string> current_names;

    // Mandatory name
    current_names.push_back(_pos.cur());
    _pos.next();

    // Optional names
    while (_pos.cur() == ",") {
      _pos.next();
      current_names.push_back(_pos.cur());
      _pos.next();
    }

    // Colon
    if (_pos.cur() != ":") {
      throw std::runtime_error("Invalid struct/enum body: "
                               "Expected ':' or '.' but saw '" +
                               _pos.cur().text + "'");
    }
    _pos.next();

    // Type
    Type t = parse_type(_pos);
    _pos.next();

    // Add these members
    for (const auto &name : current_names) {
      // Safety check for collisions
      if (all_names.contains(name)) {
        throw std::runtime_error(
            "Cannot have multiple members with name '" + name +
            "'");
      }
      all_names.insert(name);
      out.push_back({name, t});
    }

    // Advance past vestigial commas
    while (_pos.cur() == ",") {
      _pos.next();
    }
  }

  // Leave pointing to ending brace
  return out;
}

std::pair<std::list<std::string>, std::list<std::string>>
Parser::parse_template_pre_post(TokenStream &_pos) {
  debug_print();
  std::pair<std::list<std::string>, std::list<std::string>> out;

  while (_pos.peek(1) == "pre" || _pos.peek(1) == "post") {
    _pos.next(); // Now pointing to block identifier
    bool is_pre = (_pos.cur() == "pre");
    _pos.next(); // Now pointing to "{"
    if (_pos.cur() != "{") {
      throw std::runtime_error(
          "Malformed " + std::string(is_pre ? "pre" : "post") +
          " block: Expected '{', but saw '" + _pos.cur().text +
          "'");
    }

    int count = 0;
    do {
      if (_pos.done()) {
        throw std::runtime_error("Reached EOF before '}'");
      } else if (_pos.cur() == "{") {
        ++count;

        if (count == 1) {
          _pos.next(); // Don't use incr here
          continue;
        }
      } else if (_pos.cur() == "}") {
        --count;
        if (count == 0) {
          break;
        }
      }

      if (is_pre) {
        out.first.push_back(_pos.cur());
      } else {
        out.second.push_back(_pos.cur());
      }

      _pos.next(); // Don't use incr here
    } while (count != 0);
  }

  return out;
}

// Return the type spec at the specified location
Type Parser::parse_type(TokenStream &_pos) {
  debug_print();
  if (settings.debug) {
    settings.ostream << __FUNCTION__ << " at "
                     << _pos.cur().file.string() << ":"
                     << _pos.cur().line << "." << _pos.cur().col
                     << '\n';
  }

  // Special case: type! macro
  if (_pos.cur() == "type!") {
    _pos.next();
    if (_pos.cur() != "(") {
      throw std::runtime_error(
          "Malformed type! macro: Expected '(', but saw '" +
          _pos.cur().text + "'");
    }
    _pos.next();

    // Fix math
    fix_math(_pos);

    // Do the thing
    auto tmp = parse_object(_pos);

    _pos.next();
    if (_pos.cur() != ")") {
      throw std::runtime_error(
          "Malformed type! macro: Expected ')', but saw '" +
          _pos.cur().text + "'");
    }

    return ASTNodes::type(tmp);
  }

  Type out;
  bool first = true;

  do {
    if (first) {
      first = false;
    } else {
      _pos.next();
    }

    if (_pos.cur().text == ";" || _pos.cur().text == "{") {
      throw std::runtime_error("Missing function return type: "
                               "Did you mean '-> void'?");
    }

    out.process_next(_pos.cur().text);

    if (_pos.peek(1).type == "TEMPLATE" &&
        _pos.peek(1).text == "<") {
      if (out.nodes.empty() ||
          out.nodes.back().type != Type::TypeNode::LITERAL) {
        throw std::runtime_error(
            "Cannot append templating onto non-literal-ending "
            "type '" +
            out.oak_repr() + "'");
      }
      _pos.next(); // Now pointing to '<'

      // Leave pointing to closing angle bracket
      std::list<std::list<std::string>> replacements;
      std::list<std::string> cur;
      int count = 0;
      do {
        if (_pos.cur() == "<") {
          ++count;
        } else if (_pos.cur() == ">") {
          --count;
        }

        if (count == 1 && _pos.cur() == ",") {
          replacements.push_back(cur);
          cur.clear();
        } else {
          cur.push_back(_pos.cur());
        }

        _pos.next();
      } while (!_pos.done() && count != 0);

      replacements.push_back(cur);
      _pos.prev();

      replacements.front().pop_front();
      replacements.back().pop_back();

      const std::string original_name =
          out.nodes.back().literal_name;

      if (!replacements.empty()) {
        out.nodes.back().literal_name += "_GEN_";
        bool first = true;
        for (const auto &repl : replacements) {
          if (first) {
            first = false;
          } else {
            out.nodes.back().literal_name += "JOIN_";
          }
          for (const auto &tok : repl) {
            out.nodes.back().literal_name += tok + "_";
          }
        }
        out.nodes.back().literal_name += "ENDGEN";
      }

      if (!scope_manager.contains_atomic_type(
              out.nodes.back().literal_name)) {
        // Attempt template instantiation
        bool success = false;
        const auto res = scope_manager.get(original_name);
        if (res.has_value() &&
            std::holds_alternative<std::list<std::variant<
                FnInfo, std::shared_ptr<TemplateInfo>>>>(
                res.value())) {

          const auto templates =
              std::get<std::list<std::variant<
                  FnInfo, std::shared_ptr<TemplateInfo>>>>(
                  res.value());

          for (auto templates_at_i = templates.begin();
               templates_at_i != templates.end();
               ++templates_at_i) {
            if (!std::holds_alternative<
                    std::shared_ptr<TemplateInfo>>(
                    *templates_at_i)) {
              continue;
            }

            auto t = std::get<std::shared_ptr<TemplateInfo>>(
                *templates_at_i);
            if (t->does_provide(replacements, {"struct"})) {
              if (instantiate(*t, replacements)) {
                success = true;
                break;
              }
            } else if (t->does_provide(replacements,
                                       {"enum"})) {
              if (instantiate(*t, replacements)) {
                success = true;
                break;
              }
            }
          }
        }

        if (!success) {
          throw std::runtime_error(
              "No templates provided generic type '" +
              out.nodes.back().literal_name + "'");
        }
      }
    }
  } while (!out.valid() && !_pos.done());
  return out;
}

// Parse a single struct declaration
// Assumes we have just seen "let NAME : struct" and are
// pointing to the next token.
void Parser::parse_struct(const std::list<std::string> &_names,
                          TokenStream &_pos) {
  debug_print();
  if (settings.debug) {
    settings.ostream << __FUNCTION__ << " at "
                     << _pos.cur().file.string() << ":"
                     << _pos.cur().line << "." << _pos.cur().col
                     << '\n';
  }

  // The struct info to populate
  StructInfo to_add;

  to_add.tags["file"] = _pos.cur().file;
  to_add.tags["line"] = std::to_string(_pos.cur().line);
  to_add.tags["col"] = std::to_string(_pos.cur().col);

  // Casual def
  if (_pos.cur() == ";") {
    to_add.tags["casual"] = "true";
  }

  // Declaration
  else if (_pos.cur() == "{") {
    const auto members = parse_members(_pos);

    for (const auto &member : members) {
      to_add.member_order.push_back(member.first);
      to_add.members[member.first] = member.second;
    }
  }

  // Invalid
  else {
    throw std::runtime_error("Invalid struct declaration: "
                             "Expected ';' or '{' but saw '" +
                             _pos.cur().text + "'");
  }

  // Add these entries
  for (const auto &name : _names) {
    if (!is_valid_struct_name(name)) {
      settings.warn("Struct name \"" + name +
                    "\" does not seem to be camelcase");
    }

    to_add.name = name;
    scope_manager.add(name, to_add);

    // Constructor and destructor autogen go here
    // Create a constructor to parse
    auto default_constructor =
        to_add.get_default_constructor(_pos.cur());
    parse_function({"New"}, default_constructor);
    scope_manager.tag_fn("New", "autogen", "true");

    // Reset, create destructor
    auto default_destructor =
        to_add.get_default_destructor(_pos.cur());
    parse_function({"Del"}, default_destructor);
    scope_manager.tag_fn("Del", "autogen", "true");
  }
}

// Parse a single enum declaration
// Assumes we have just seen "let NAME : enum" and are
// pointing to the next token.
void Parser::parse_enum(const std::list<std::string> &_names,
                        TokenStream &_pos) {
  debug_print();
  if (settings.debug) {
    settings.ostream << __FUNCTION__ << " at "
                     << _pos.cur().file.string() << ":"
                     << _pos.cur().line << "." << _pos.cur().col
                     << '\n';
  }

  // The enum info to populate
  EnumInfo to_add;

  to_add.tags["file"] = _pos.cur().file;
  to_add.tags["line"] = std::to_string(_pos.cur().line);
  to_add.tags["col"] = std::to_string(_pos.cur().col);

  // Casual def
  if (_pos.cur() == ";") {
    to_add.tags["casual"] = "true";
  }

  // Declaration
  else if (_pos.cur() == "{") {
    const auto members = parse_members(_pos);

    for (const auto &member : members) {
      to_add.option_order.push_back(member.first);
      to_add.options[member.first] = member.second;
    }
  }

  // Invalid
  else {
    throw std::runtime_error("Invalid enum declaration: "
                             "Expected ';' or '{' but saw '" +
                             _pos.cur().text + "'");
  }

  // Add these entries
  for (const auto &name : _names) {
    if (!is_valid_struct_name(name)) {
      settings.warn("Enum name \"" + name +
                    "\" does not seem to be camelcase");
    }

    to_add.name = name;
    scope_manager.add(name, to_add);

    // Wrappers
    // wrap_a(self, what)
    for (const auto &p : to_add.get_wrappers(_pos.cur())) {
      scope_manager.add(p.name, p);
    }

    // Constructor
    auto default_constructor =
        to_add.get_default_constructor(_pos.cur());
    parse_function({"New"}, default_constructor);

    // Create destructor
    auto default_destructor =
        to_add.get_default_destructor(_pos.cur());
    parse_function({"Del"}, default_destructor);
  }
}

// Assumes we are pointing to the first token in the statement
ASTNodes::Statement
Parser::parse_statement(TokenStream &_pos,
                        const Type &_return_type) {
  debug_print();
  if (settings.debug) {
    settings.ostream << __FUNCTION__ << " at "
                     << _pos.cur().file.string() << ":"
                     << _pos.cur().line << "." << _pos.cur().col
                     << '\n';
  }

  if (_pos.cur() == "c!") {
    _pos.next();
    if (_pos.cur() != "(") {
      throw std::runtime_error(
          "Invalid c! macro: Expected '(', but saw '" +
          _pos.cur().text + "'");
    }
    _pos.next();

    // One string literal argument
    ASTNodes::RawCFormat out;
    out.format_string =
        Macros::strip_string_literal(_pos.cur().text);

    _pos.next();
    if (_pos.cur() != ")") {
      throw std::runtime_error(
          "Invalid c! macro: Expected ')', but saw '" +
          _pos.cur().text + "'");
    }
    _pos.next();

    return ASTNodes::Statement(
        {ASTNodes::OptBox<ASTNodes::Node>(out)});
  } else if (_pos.cur() == "compile_time_error!") {
    std::stringstream msg_strm;
    msg_strm << _pos.cur().file.string() << ":"
             << _pos.cur().line << "." << _pos.cur().col << ">"
             << _pos.cur().text << " Compile-time error:\n";
    const auto args = Macros::get_macro_args(_pos);
    for (const auto &arg : args) {
      for (const auto &tok : arg) {
        msg_strm << tok.text + " ";
      }
      msg_strm << " ";
    }
    msg_strm << '\n';

    settings.ostream << msg_strm.str();
    throw std::runtime_error(msg_strm.str());
  } else if (_pos.cur() == "compile_time_warning!") {
    std::stringstream msg_strm;
    msg_strm << _pos.cur().file.string() << ":"
             << _pos.cur().line << "." << _pos.cur().col << ">"
             << _pos.cur().text << " Compile-time warning:\n";
    const auto args = Macros::get_macro_args(_pos);
    for (const auto &arg : args) {
      for (const auto &tok : arg) {
        msg_strm << tok.text + " ";
      }
      msg_strm << " ";
    }
    msg_strm << '\n';

    settings.warn(msg_strm.str());
  } else if (_pos.cur() == "compile_time_print!") {
    settings.ostream << _pos.cur().file.string() << ":"
                     << _pos.cur().line << "." << _pos.cur().col
                     << ">" << _pos.cur().text
                     << " Compile-time print:\n";
    const auto args = Macros::get_macro_args(_pos);
    for (const auto &arg : args) {
      for (const auto &tok : arg) {
        settings.ostream << tok.text + " ";
      }
      settings.ostream << " ";
    }
    settings.ostream << '\n';
  } else if (_pos.cur() == "alias!") {
    // In Oak: alias!(to, from);
    // In C++: using to = from;
    const auto args = Macros::get_macro_args(_pos);
    if (args.size() != 2) {
      throw std::runtime_error(
          "'alias!' takes two arguments: to and from");
    }
    scope_manager.alias(concat(args.front()),
                        concat(args.back()));
  } else if (_pos.cur() == "erase!") {
    const auto args = Macros::get_macro_args(_pos);
    for (const auto &entry : args) {
      scope_manager.erase(concat(entry));
    }
  } else if (_pos.cur() == "namespace_use!") {
    // In Oak: namespace::use!("std");
    // In C++: using namespace std;

    const auto args = Macros::get_macro_args(_pos);
    if (args.size() != 1) {
      throw std::runtime_error("'namespace::use!' takes one "
                               "string argument: The prefix to "
                               "remove");
    }
    scope_manager.remove_prefix(concat(args.front()));
  } else if (_pos.cur() == "include!") {
    auto raw_args = Macros::get_macro_args(_pos);

    std::list<Lexer::Token> args;
    for (auto it = raw_args.begin(); it != raw_args.end();
         ++it) {
      TokenStream cur_arg(*it);
      Lexer::Token to_add = cur_arg.cur();
      for (cur_arg.next(); !cur_arg.done(); cur_arg.next()) {
        to_add.text += ' ';
        to_add.text += cur_arg.cur().text;
      }
      to_add.text = Macros::strip_string_literal(to_add.text);
      args.push_back(to_add);
    }

    try {
      for (const auto &f : args) {
        // const auto backup = rules.purge_entry_points();
        do_file(f.text, f.file);
        // rules.purge_entry_points();
        // for (const auto &item : backup) {
        //   rules.add_entry_point(item);
        // }
        // std::cerr << __FILE__ << ":" << __LINE__
        //           << "> Unimplemented\n"
        //           << std::flush;
      }
    } catch (OutOfPPPLError &e) {
      throw OutOfPPPLError(
          "In file included from " + _pos.cur().file.string() +
          ":" + std::to_string(_pos.cur().line) + "." +
          std::to_string(_pos.cur().col) + "\n" + e.what());
    } catch (std::runtime_error &e) {
      throw std::runtime_error(
          "In file included from " + _pos.cur().file.string() +
          ":" + std::to_string(_pos.cur().line) + "." +
          std::to_string(_pos.cur().col) + "\n" + e.what());
    } catch (...) {
      throw std::runtime_error(
          "In file included from " + _pos.cur().file.string() +
          ":" + std::to_string(_pos.cur().line) + "." +
          std::to_string(_pos.cur().col) + "\nUnknown error");
    }
  } else if (_pos.cur() == "link!") {

    auto raw_args = Macros::get_macro_args(_pos);

    std::list<Lexer::Token> args;
    for (auto it = raw_args.begin(); it != raw_args.end();
         ++it) {
      TokenStream cur_arg(*it);
      Lexer::Token to_add = cur_arg.cur();
      for (cur_arg.next(); !cur_arg.done(); cur_arg.next()) {
        to_add.text += ' ';
        to_add.text += cur_arg.cur().text;
      }
      args.push_back(to_add);
    }

    for (auto it = args.begin(); it != args.end(); ++it) {
      it->text = Macros::strip_string_literal(it->text);
    }

    for (const auto &f : args) {
      settings.compile_settings().objects.push_back(
          resolve_path(f.text, f.file));
    }
  } else if (_pos.cur() == "flag!") {

    auto raw_args = Macros::get_macro_args(_pos);

    std::list<Lexer::Token> args;
    for (auto it = raw_args.begin(); it != raw_args.end();
         ++it) {
      TokenStream cur_arg(*it);
      Lexer::Token to_add = cur_arg.cur();
      for (cur_arg.next(); !cur_arg.done(); cur_arg.next()) {
        to_add.text += ' ';
        to_add.text += cur_arg.cur().text;
      }
      args.push_back(to_add);
    }

    for (auto it = args.begin(); it != args.end(); ++it) {
      it->text = Macros::strip_string_literal(it->text);
    }

    for (const auto &f : args) {
      settings.compile_settings().link_flags.push_back(f.text);
    }
  } else if (_pos.cur() == "pragma!") {

    auto raw_args = Macros::get_macro_args(_pos);

    std::list<Lexer::Token> args;
    for (auto it = raw_args.begin(); it != raw_args.end();
         ++it) {
      TokenStream cur_arg(*it);
      Lexer::Token to_add = cur_arg.cur();
      for (cur_arg.next(); !cur_arg.done(); cur_arg.next()) {
        to_add.text += ' ';
        to_add.text += cur_arg.cur().text;
      }
      args.push_back(to_add);
    }

    for (auto it = args.begin(); it != args.end(); ++it) {
      it->text = Macros::strip_string_literal(it->text);
    }

    if (args.size() == 1) {
      args.push_back(Lexer::Token(args.front(), ""));
    }

    settings.compile_settings()
        .pragmas[_pos.cur().file][args.front().text] =
        std::next(args.begin())->text;
  } else if (_pos.cur() == "rule_new!") {

    auto raw_args = Macros::get_macro_args(_pos);

    std::list<Lexer::Token> args;
    for (auto it = raw_args.begin(); it != raw_args.end();
         ++it) {
      TokenStream cur_arg(*it);

      Lexer::Token to_add = cur_arg.cur();
      for (cur_arg.next(); !cur_arg.done(); cur_arg.next()) {
        to_add.text += ' ';
        to_add.text += cur_arg.cur().text;
      }
      args.push_back(to_add);
    }

    for (auto it = args.begin(); it != args.end(); ++it) {
      it->text = Macros::strip_string_literal(it->text);
    }

    if (args.size() < 3) {
      throw std::runtime_error(
          "Malformed rule::new! call: Arguments must "
          "be "
          "rule_name, input_rule, output_rule, "
          "[engine_name], [prerequisites...]");
    }

    // Name, input, output (using sapling engine)
    std::string name = args.front();
    std::string engine = "sapling";
    std::list<std::string> prereqs;
    if (args.size() == 4) {
      // Name, input, output, engine
      engine = *std::next(args.begin(), 3);
    } else if (args.size() > 4) {
      // Name, input, output, engine, prerequisites
      engine = *std::next(args.begin(), 3);
      for (auto it = std::next(args.begin(), 4);
           it != args.end(); ++it) {
        prereqs.push_back(_pos.cur());
      }
    }

    // Rule to_add(*std::next(args.begin()),
    //             *std::next(args.begin(), 2), prereqs,
    //             engine);

    // rules.register_rule(name, to_add);
    // std::cerr << __FILE__ << ":" << __LINE__
    //           << "> Unimplemented\n"
    //           << std::flush;
  } else if (_pos.cur() == "rule_use!") {

    auto raw_args = Macros::get_macro_args(_pos);

    std::list<Lexer::Token> args;
    for (auto it = raw_args.begin(); it != raw_args.end();
         ++it) {
      TokenStream cur_arg(*it);

      Lexer::Token to_add = cur_arg.cur();
      for (cur_arg.next(); !cur_arg.done(); cur_arg.next()) {
        to_add.text += ' ';
        to_add.text += cur_arg.cur().text;
      }
      args.push_back(to_add);
    }

    // for (const auto &_ : args) {
    //   rules.add_entry_point(
    //       Macros::strip_string_literal(arg));
    //   std::cerr << __FILE__ << ":" << __LINE__
    //             << "> Unimplemented\n"
    //             << std::flush;
    // }
    // std::cerr << __FILE__ << ":" << __LINE__
    //           << "> Unimplemented\n"
    //           << std::flush;
  } else if (_pos.cur() == "rule_remove!") {

    auto raw_args = Macros::get_macro_args(_pos);

    std::list<Lexer::Token> args;
    for (auto it = raw_args.begin(); it != raw_args.end();
         ++it) {
      TokenStream cur_arg(*it);

      Lexer::Token to_add = cur_arg.cur();
      for (cur_arg.next(); !cur_arg.done(); cur_arg.next()) {
        to_add.text += ' ';
        to_add.text += cur_arg.cur().text;
      }
      args.push_back(to_add);
    }

    for (const auto &_ : args) {
      // rules.remove_entry_point(
      //     Macros::strip_string_literal(arg));
      std::cerr << __FILE__ << ":" << __LINE__
                << "> Unimplemented\n"
                << std::flush;
    }
  } else if (_pos.cur() == "rule_bundle!") {

    auto raw_args = Macros::get_macro_args(_pos);

    std::list<Lexer::Token> args;
    for (auto it = raw_args.begin(); it != raw_args.end();
         ++it) {
      TokenStream cur_arg(*it);

      Lexer::Token to_add = cur_arg.cur();
      for (cur_arg.next(); !cur_arg.done(); cur_arg.next()) {
        to_add.text += ' ';
        to_add.text += cur_arg.cur().text;
      }
      args.push_back(to_add);
    }

    std::list<std::string> entails;
    for (auto it = std::next(args.begin()); it != args.end();
         ++it) {
      entails.push_back(Macros::strip_string_literal(it->text));
    }

    // rules.register_bundle(
    //     Macros::strip_string_literal(args.front()),
    //     entails);
    // std::cerr << __FILE__ << ":" << __LINE__
    //           << "> Unimplemented\n"
    //           << std::flush;
  } else if (_pos.cur() == "unstr!") {

    Lexer::Token to_add(_pos.cur());
    auto raw_args = Macros::get_macro_args(_pos);
    to_add.text.clear();

    for (auto it = raw_args.begin(); it != raw_args.end();
         ++it) {
      for (auto inner_it = it->begin(); inner_it != it->end();
           ++inner_it) {
        if (!to_add.text.empty()) {
          to_add.text += ' ';
        }
        to_add.text += inner_it->text;
      }
    }

    to_add.text = Macros::strip_string_literal(to_add.text);

    uint64_t dummy_line = to_add.line, dummy_col = to_add.col;
    auto to_insert = Lexer::lex(to_add.text, to_add.file,
                                dummy_line, dummy_col);
    _pos.insert(_pos.tell(), to_insert);
  } else if (_pos.cur() == "str!") {

    Lexer::Token to_add(_pos.cur());
    auto raw_args = Macros::get_macro_args(_pos);
    to_add.text.clear();

    for (auto it = raw_args.begin(); it != raw_args.end();
         ++it) {
      for (auto inner_it = it->begin(); inner_it != it->end();
           ++inner_it) {
        if (!to_add.text.empty()) {
          to_add.text += ' ';
        }
        to_add.text += inner_it->text;
      }
    }

    // Ensure exactly one set of enclosing quotes
    to_add.text = Macros::make_string_literal(
        Macros::strip_string_literal(to_add.text));
    Lexer::classify_type(to_add);
    _pos.insert(_pos.tell(), to_add);
  } else if (_pos.cur() == "compile_time_system!") {

    settings.ostream
        << _pos.cur().file.string() << ":" << _pos.cur().line
        << "." << _pos.cur().col
        << ">\ncompile_time::system! asks to run `";

    auto raw_args = Macros::get_macro_args(_pos);

    std::list<Lexer::Token> args;
    for (auto it = raw_args.begin(); it != raw_args.end();
         ++it) {
      TokenStream cur_arg(*it);

      Lexer::Token to_add = cur_arg.cur();
      for (cur_arg.next(); !cur_arg.done(); cur_arg.next()) {
        to_add.text += ' ';
        to_add.text += cur_arg.cur().text;
      }
      args.push_back(to_add);
    }

    for (auto it = args.begin(); it != args.end(); ++it) {
      it->text = Macros::strip_string_literal(it->text);
    }

    std::string cmd;
    for (const auto &arg : args) {
      if (!cmd.empty()) {
        cmd.push_back(' ');
      }
      cmd += arg.text;
    }

    settings.ostream << cmd << "` at "
                     << _pos.cur().file.parent_path() << "\n"
                     << std::flush;

    if (!settings.compile_settings().no_confirm) {
      settings.ostream << "Allow? [N/y/a] ";
      char choice = std::cin.get();

      switch (choice) {
      default:
        throw std::runtime_error("Abort!");
      case 'a':
      case 'A':
        settings.ostream << "Not asking again!\n";
        settings.compile_settings().no_confirm = true;
      case 'y':
      case 'Y':
        break;
      }
    } else {
      settings.ostream << "(no_confirm is enabled, so running "
                          "without asking)\n";
    }

    const auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(
        _pos.cur().file.parent_path());

    auto result = system(cmd.c_str());

    std::filesystem::current_path(old_cwd);

    if (result != 0) {
      throw std::runtime_error(
          "System call '" + cmd +
          "' exited with nonzero exit code " +
          std::to_string(result));
    }
  }

  // Delegate for macro calls
  else if (_pos.cur().text.ends_with("!")) {
    replace_macro(_pos);
    return ASTNodes::Statement();
  }

  else if (_pos.cur() == ";") {
    // Unit statement
    return ASTNodes::Statement();
  } else if (_pos.cur() == "let") {
    // Variable declaration
    // Collect names
    std::list<std::string> names;
    bool is_macro = false;

    do {
      // Fluff
      _pos.next();

      // Name
      names.push_back(_pos.cur());
      if (names.back().ends_with("!")) {
        is_macro = true;
      }
      _pos.next();
    } while (_pos.cur() == ",");

    // Generics
    std::list<std::string> generics;
    if (_pos.cur() == "<") {
      // Zero or more comma-separated generics
      if (is_macro) {
        throw std::runtime_error("Macros cannot be templated");
      }

      do {
        _pos.next();
        if (!is_valid_struct_name(_pos.cur())) {
          settings.warn("Generic '" + _pos.cur().text +
                        "' at " + _pos.cur().file.string() +
                        ":" + std::to_string(_pos.cur().line) +
                        "." + std::to_string(_pos.cur().col) +
                        " does not appear to be camelcase");
        }
        generics.push_back(_pos.cur());
        _pos.next();
      } while (_pos.cur() == ",");

      if (_pos.cur() != ">") {
        throw std::runtime_error(
            "Malformed generic: Expected '>', but saw '" +
            _pos.cur().text + "'");
      }
      _pos.next();
    }

    if (_pos.cur() == ":") {
      // Variables, structs, and enums
      _pos.next();
      if (_pos.cur() == "struct") {
        // Struct
        _pos.next(); // Now pointing at body

        if (generics.empty()) {
          parse_struct(names, _pos);
        } else if (_pos.cur() == ";") {
          throw std::runtime_error(
              "Generic struct signatures are illegal");
        } else {
          // Add definition for generic struct(s)
          TemplateInfo info(_pos.cur().file, _pos.cur().line,
                            _pos.cur().col);
          info.generics = generics;

          // Grab body here
          int count = 0;
          do {
            if (_pos.done()) {
              throw std::runtime_error(
                  "Generic struct signature must be "
                  "defined");
            } else if (_pos.cur() == "{") {
              ++count;
            } else if (_pos.cur() == "}") {
              --count;
            }
            info.instantiate_block.push_back(_pos.cur());
            _pos.next();
          } while (count != 0);
          _pos.prev();

          const auto p = parse_template_pre_post(_pos);
          for (const auto &item : p.second) {
            info.instantiate_block.push_back(item);
          }
          info.validate_block = p.first;

          for (const auto &name : names) {
            TemplateInfo specific_info = info;
            specific_info.provides_block = {"struct"};
            specific_info.instantiate_block.push_front(
                "struct");
            specific_info.instantiate_block.push_front(":");
            specific_info.instantiate_block.push_front(name);
            specific_info.instantiate_block.push_front("let");
            scope_manager.add(name, specific_info);
          }
        }

        _pos.next();
        return ASTNodes::Statement({});
      } else if (_pos.cur() == "enum") {
        // enum
        _pos.next();

        if (generics.empty()) {
          parse_enum(names, _pos);
        } else {
          throw std::runtime_error(
              "Generic enums are unimplemented");
        }

        _pos.next();
        return ASTNodes::Statement({});
      } else {
        // Variable

        // Get type
        Type t = parse_type(_pos);
        validate_type(t);

        ASTNodes::Declaration out;
        out.type = t;

        for (const auto &name : names) {
          out.names.push_back(name);
          scope_manager.add(name, t);

          // Literal `New` call
          const auto tok = _pos.cur();
          TokenStream new_call({Lexer::Token(tok, "New"),
                                Lexer::Token(tok, "("),
                                Lexer::Token(tok, name),
                                Lexer::Token(tok, ")")});
          out.new_calls.push_back(
              parse_function_call(new_call));
        }

        ASTNodes::Statement stmt_out(
            {ASTNodes::OptBox<ASTNodes::Node>(out)});

        if (_pos.cur() == "=") {
          // Instantiation-assignment copy
          throw std::runtime_error(
              "Instantiation-assignment combination operator "
              "(let A: B = C;) is unimplemented");
        }

        return stmt_out;
      }
      return ASTNodes::Statement({});
    } else if (_pos.cur() == "(") {
      if (is_macro) {
        parse_macro(
            _pos,
            settings.compile_settings().preprocess_pass_limit,
            names);
        _pos.prev();
        return ASTNodes::Statement();
      } else {
        // Function
        if (generics.empty()) {
          parse_function(names, _pos);
        } else {
          // Grab rest of signature
          TemplateInfo info(_pos.cur().file, _pos.cur().line,
                            _pos.cur().col);
          info.generics = generics;

          // Finish parsing type
          while (!_pos.done() && _pos.cur() != "{") {
            if (_pos.peek(1) != ":") {
              info.provides_block.push_back(_pos.cur());
            } else {
              info.provides_block.push_back("_");
            }

            info.instantiate_block.push_back(_pos.cur());

            _pos.next();
            if (_pos.cur() == ";") {
              // Generic signature
              throw std::runtime_error(
                  "Generic function signatures are illegal");
            }
          }

          // Grab body
          int count = 0;
          do {
            if (_pos.done()) {
              throw std::runtime_error(
                  "Generic function signatures must be "
                  "defined");
            } else if (_pos.cur() == "{") {
              ++count;
            } else if (_pos.cur() == "}") {
              --count;
            }
            info.instantiate_block.push_back(_pos.cur());
            _pos.next();
          } while (count != 0);
          _pos.prev();

          // Parse pre and post blocks
          const auto p = parse_template_pre_post(_pos);
          info.validate_block = p.first;
          for (const auto &item : p.second) {
            info.instantiate_block.push_back(item);
          }

          // Add to template table
          for (const auto &name : names) {
            TemplateInfo instance_info = info;

            instance_info.provides_block.push_front(name);
            instance_info.provides_block.push_front("let");

            instance_info.instantiate_block.push_front(name);
            instance_info.instantiate_block.push_front("let");

            scope_manager.add(name, instance_info);
          }
        }

        _pos.next();
        return ASTNodes::Statement({});
      }
    } else if (is_macro && _pos.cur().text == "=") {
      // Inline macro definition
      parse_macro(
          _pos,
          settings.compile_settings().preprocess_pass_limit,
          names);
      _pos.prev();
      return ASTNodes::Statement();
    }
    return ASTNodes::Statement({});
  } else if (_pos.cur() == "{") {
    // Scope
    ASTNodes::Statement out;

    // Add a frame to the scope stack
    scope_manager.push_frame();

    _pos.next();
    while (_pos.cur() != "}") {
      out.children.push_back(ASTNodes::OptBox<ASTNodes::Node>(
          parse_statement(_pos)));
      _pos.next();
    }

    // Remove that scope frame
    out.children.push_back(ASTNodes::OptBox<ASTNodes::Node>(
        scope_manager.pop_frame()));
    return out;
  } else if (_pos.cur() == "if") {
    // If statement
    _pos.next();
    bool has_parenthesis = true;
    if (_pos.cur() != "(") {
      has_parenthesis = false;
    } else {
      _pos.next();
    }

    // Condition is a single boolean object
    ASTNodes::If out;

    out.condition = parse_object(_pos);

    if (!ASTNodes::type(out.condition.get())
             .cast_match(Type({"bool"}))) {
      throw std::runtime_error(
          "Statement condition type '" +
          ASTNodes::type(out.condition.get()).oak_repr() +
          "' is not castable to bool.");
    }

    // Closing parenthesis
    _pos.next();
    if (has_parenthesis) {
      if (_pos.cur() != ")") {
        throw std::runtime_error(
            "Missing ending parenthesis in 'if' statement.");
      }
      _pos.next();
    }

    // Body
    out.then_body =
        ASTNodes::OptBox<ASTNodes::Node>(parse_statement(_pos));

    // Optional else clause
    _pos.next();
    if (!_pos.done() && _pos.cur() == "else") {
      // Else clause
      _pos.next();
      out.else_body = ASTNodes::OptBox<ASTNodes::Node>(
          parse_statement(_pos));
    } else {
      _pos.prev();
    }

    return ASTNodes::Statement(
        {ASTNodes::OptBox<ASTNodes::Node>(out)});
  } else if (_pos.cur() == "while") {
    // While loop
    _pos.next();
    bool has_parenthesis = true;
    if (_pos.cur() != "(") {
      has_parenthesis = false;
    } else {
      _pos.next();
    }

    // Condition is a single boolean object
    ASTNodes::While out;

    out.condition = parse_object(_pos);

    if (!ASTNodes::type(out.condition.get())
             .cast_match(Type({"bool"}))) {
      throw std::runtime_error(
          "Statement condition type '" +
          ASTNodes::type(out.condition.get()).oak_repr() +
          "' is not castable to bool.");
    }

    // Closing parenthesis
    _pos.next();
    if (has_parenthesis) {
      if (_pos.cur() != ")") {
        throw std::runtime_error(
            "Missing ending parenthesis in 'while' statement.");
      }
      _pos.next();
    }

    // Body
    ASTNodes::Statement body = parse_statement(_pos);

    out.body = body;
    return ASTNodes::Statement(
        {ASTNodes::OptBox<ASTNodes::Node>(out)});
  } else if (_pos.cur() == "match") {
    // Match statement
    /*
    match (NAME) {
        case first(a: foo) {}
        case second(b: fizz) {}
        else {}
    }
    */
    _pos.next();
    bool has_parenthesis = true;
    if (_pos.cur() != "(") {
      has_parenthesis = false;
    } else {
      _pos.next();
    }

    // Target is an enum
    auto target = parse_object(_pos);

    Type target_type = ASTNodes::type(target);

    const bool is_mutable = (target_type.nodes.front().type ==
                             Type::TypeNode::POINTER);
    if (is_mutable) {
      target_type = target_type.deref();
    }

    const std::string enum_name = target_type.struct_name();
    const EnumInfo info = scope_manager.at<EnumInfo>(enum_name);

    // Closing parenthesis
    _pos.next();
    if (has_parenthesis) {
      if (_pos.cur() != ")") {
        throw std::runtime_error(
            "Missing ending parenthesis in 'match' statement.");
      }
      _pos.next();
    }

    // 0th child is operand, rest are cases
    ASTNodes::Match out;
    out.enum_name = enum_name;
    out.is_mutable = is_mutable;

    if (is_mutable) {
      ASTNodes::RawCFormat new_target;
      new_target.format_string = "(*%)";
      new_target.args = {(target)};
      target = new_target;
    }
    out.upon = target;

    if (_pos.cur() != "{") {
      throw std::runtime_error(
          "Missing opening curly brace in 'match' statement.");
    }
    _pos.next();

    while (_pos.cur() != "}") {
      const auto res = parse_case(info, _pos, is_mutable);
      if (std::holds_alternative<ASTNodes::Case>(res)) {
        out.branches.push_back(ASTNodes::OptBox<ASTNodes::Node>(
            std::get<ASTNodes::Case>(res)));
      } else {
        out.branches.push_back(ASTNodes::OptBox<ASTNodes::Node>(
            std::get<ASTNodes::Statement>(res)));
      }
      _pos.next();
    }

    return ASTNodes::Statement(
        {ASTNodes::OptBox<ASTNodes::Node>(out)});
  } else if (_pos.cur() == "return") {
    // Return statement
    ASTNodes::Return out;
    _pos.next();
    if (_pos.cur() != ";") {
      out.value = parse_object(_pos);

      if (!settings.compile_settings()
               .cur_return_type.exact_match(
                   ASTNodes::type(out.value.get()))) {
        throw std::runtime_error(
            "Invalid return type '" +
            ASTNodes::type(out.value.get()).oak_repr() +
            "' for fn w/ return "
            "type '" +
            settings.compile_settings()
                .cur_return_type.oak_repr() +
            "'");
      }
    } else if (!settings.compile_settings()
                    .cur_return_type.exact_match({"void"})) {
      throw std::runtime_error(
          "Invalid return type 'void' for fn w/ return type '" +
          settings.compile_settings()
              .cur_return_type.oak_repr() +
          "'");
    }
    return ASTNodes::Statement(
        {ASTNodes::OptBox<ASTNodes::Node>(out)});
  } else {
    // Function call
    auto ret = parse_function_call(_pos);
    return ASTNodes::Statement({ret});
  }
  return ASTNodes::Statement({});
}

std::variant<ASTNodes::Case, ASTNodes::Statement>
Parser::parse_case(const EnumInfo &_enum_type,
                   TokenStream &_pos, const bool &_is_mutable) {
  debug_print();
  if (settings.debug) {
    settings.ostream << __FUNCTION__ << " at "
                     << _pos.cur().file.string() << ":"
                     << _pos.cur().line << "." << _pos.cur().col
                     << '\n';
  }

  if (_pos.cur() == "case") {
    _pos.next();

    // Case name
    const auto case_name = _pos.cur().text;
    if (!_enum_type.options.contains(case_name)) {
      throw std::runtime_error("'" + case_name +
                               "' is not a valid enum option");
    }

    // Open parenthesis
    _pos.next();
    if (_pos.cur() != "(") {
      throw std::runtime_error("Malformed 'case' statement: "
                               "Expected '(', but saw '" +
                               _pos.cur().text + "'");
    }
    _pos.next();

    // Arg w/ type
    const auto passed_name = _pos.cur();
    _pos.next();

    if (_pos.cur() != ":") {
      throw std::runtime_error("Malformed 'case' statement: "
                               "Expected ':', but saw '" +
                               _pos.cur().text + "'");
    }
    _pos.next();

    Type passed_type = parse_type(_pos);

    if (_is_mutable) {
      // Pointer or exact allowed
      if (!passed_type.exact_match(
              _enum_type.options.at(case_name)) &&
          !passed_type.deref().exact_match(
              _enum_type.options.at(case_name))) {
        throw std::runtime_error(
            "Invalid type for mutable case '" + case_name +
            "': Expected '" +
            _enum_type.options.at(case_name).oak_repr() +
            "' (or a pointer to that), but saw '" +
            passed_type.oak_repr() + "'");
      }
    } else {
      // Only exact allowed
      if (!passed_type.exact_match(
              _enum_type.options.at(case_name))) {
        throw std::runtime_error(
            "Invalid type for immutable case '" + case_name +
            "': Expected '" +
            _enum_type.options.at(case_name).oak_repr() +
            "', but saw '" + passed_type.oak_repr() + "'");
      }
    }

    // End parenthesis
    _pos.next();
    if (_pos.cur() != ")") {
      throw std::runtime_error("Malformed 'case' statement: "
                               "Expected ')', but saw '" +
                               _pos.cur().text + "'");
    }
    _pos.next();

    // Push to locals stack
    scope_manager.push_frame();
    scope_manager.add(passed_name, passed_type);

    // Push frame to be popped
    scope_manager.push_frame();

    ASTNodes::Case out;
    out.case_name = case_name;
    out.passed_name = passed_name;
    out.type = passed_type;

    // Statement
    auto statement = parse_statement(_pos);

    // Pop frame, calling destructors
    statement.children.push_back(
        ASTNodes::OptBox<ASTNodes::Node>(
            scope_manager.pop_frame()));
    out.body = statement;

    // Pop from locals stack WITHOUT CALLING DESTRUCTOR ON
    // CAPTURE
    scope_manager.pop_frame();

    return out;
  } else if (_pos.cur() == "else") {
    // Statement
    _pos.next();
    ASTNodes::Statement out;
    out.children = {ASTNodes::OptBox<ASTNodes::Node>(
        parse_statement(_pos))};
    return out;
  } else {
    throw std::runtime_error(
        "Error within match statement: Expected 'case' or "
        "'else', but saw '" +
        _pos.cur().text + "'");
  }
}

ASTNodes::Node Parser::get_function_call_node(
    const std::string &_unmangled_name,
    const std::list<ASTNodes::Node> &_args) {
  //////////////////////////////////////////////////////////////
  // Special cases here

  // Array access via the 'Get' operator
  if (_unmangled_name == "Get" && _args.size() == 2 &&
      (ASTNodes::type(_args.back())
           .cast_match(Type({"u128"})) ||
       ASTNodes::type(_args.back())
           .cast_match(Type({"i128"}))) &&
      (ASTNodes::type(_args.front()).nodes.front().type ==
           Type::TypeNode::SIZED_ARRAY ||
       ASTNodes::type(_args.front()).nodes.front().type ==
           Type::TypeNode::UNSIZED_ARRAY)) {
    // Only resolvable at reconstruction-time
    ASTNodes::ArrAccess out;
    out.upon = ASTNodes::OptBox<ASTNodes::Node>(_args.front());
    out.index = ASTNodes::OptBox<ASTNodes::Node>(_args.back());
    out.return_type = ASTNodes::type(out.upon.get());
    out.return_type.nodes.pop_front();
    return out;
  }

  // alloc!
  else if (_unmangled_name == "alloc!") {
    // 1-arg
    if (_args.size() == 1) {
      if (ASTNodes::type(_args.front()).nodes.empty() ||
          ASTNodes::type(_args.front()).nodes.front().type !=
              Type::TypeNode::POINTER) {
        throw std::runtime_error(
            "Expected pointer type for alloc!(into), instead "
            "saw '" +
            ASTNodes::type(_args.front()).oak_repr() + "'");
      }

      ASTNodes::RawCFormat out;
      out.type = Type({"void"});
      out.args = {_args.front(), _args.front()};
      out.format_string =
          "% = (" +
          ASTNodes::type(out.args.front().get()).c_repr() +
          ") malloc(sizeof(" +
          ASTNodes::type(out.args.front().get())
              .deref()
              .c_repr() +
          ")); assert(% != NULL)";
      return out;
    }

    // 2-arg
    else if (_args.size() == 2) {
      if (ASTNodes::type(_args.front()).nodes.empty() ||
          ASTNodes::type(_args.front()).nodes.front().type !=
              Type::TypeNode::UNSIZED_ARRAY) {
        throw std::runtime_error(
            "Expected unsized array type for alloc!(into, "
            "size), instead saw '" +
            ASTNodes::type(_args.front()).oak_repr() + "'");
      }

      ASTNodes::RawCFormat out;
      out.type = Type({"void"});
      out.args = {_args.front(), _args.back()};
      out.args.push_back(_args.front());
      out.format_string =
          "% = (" +
          ASTNodes::type(out.args.front().get()).c_repr() +
          ") calloc(%, sizeof(" +
          ASTNodes::type(out.args.front().get()).c_repr() +
          ")); assert(% != NULL)";
      return out;
    }

    // Error case
    else {
      throw std::runtime_error("alloc! takes 1 or 2 args.");
    }
  }

  // free!
  else if (_unmangled_name == "free!") {
    if (_args.size() != 1 ||
        ASTNodes::type(_args.front()).nodes.empty() ||
        (ASTNodes::type(_args.front()).nodes.front().type !=
             Type::TypeNode::POINTER &&
         ASTNodes::type(_args.front()).nodes.front().type !=
             Type::TypeNode::UNSIZED_ARRAY)) {
      throw std::runtime_error(
          "Expected pointer or unsized array type for "
          "free!(to_free), instead saw '" +
          ASTNodes::type(_args.front()).oak_repr() + "'");
    }

    ASTNodes::RawCFormat out;
    out.args = {_args.front()};
    out.type = Type({"void"});
    out.format_string = "free((void *)(%))";
    return out;
  }

  // New on pointer or unsized array types
  else if (_unmangled_name == "New" && _args.size() == 1 &&
           (ASTNodes::type(_args.front()).nodes.front().type ==
                Type::TypeNode::POINTER ||
            ASTNodes::type(_args.front()).nodes.front().type ==
                Type::TypeNode::UNSIZED_ARRAY)) {
    ASTNodes::RawCFormat out;
    out.args = {_args.front()};
    out.type = Type({"void"});
    out.format_string = "% = 0";
    return out;
  }

  // New on sized array types
  else if (_unmangled_name == "New" && _args.size() == 1 &&
           ASTNodes::type(_args.front()).nodes.front().type ==
               Type::TypeNode::SIZED_ARRAY) {
    const auto size = ASTNodes::type(_args.front())
                          .nodes.front()
                          .sized_array_size;

    ASTNodes::Statement out;
    for (uint i = 0; i < size; ++i) {
      ASTNodes::RawCFormat arg;
      arg.format_string = "(%)[" + std::to_string(i) + "]";
      arg.args = {_args.front()};

      arg.type = ASTNodes::type(_args.front());
      arg.type->nodes.pop_front();

      out.children.push_back(
          get_function_call_node("New", {arg}));
    }
    return out;
  }

  // New on atomic types
  else if (_unmangled_name == "New" && _args.size() == 1 &&
           Type::is_built_in_type(
               ASTNodes::type(_args.front()))) {
    ASTNodes::RawCFormat out;
    out.args = {_args.front()};
    out.type = Type({"void"});
    out.format_string = "% = 0;";
    return out;
  }

  // Del on atomic types
  else if (_unmangled_name == "Del" && _args.size() == 1 &&
           Type::is_built_in_type(
               ASTNodes::type(_args.front()))) {
    return ASTNodes::Statement();
  }

  // Del on pointer or unsized array types
  else if (_unmangled_name == "Del" && _args.size() == 1 &&
           (ASTNodes::type(_args.front()).nodes.front().type ==
                Type::TypeNode::POINTER ||
            ASTNodes::type(_args.front()).nodes.front().type ==
                Type::TypeNode::UNSIZED_ARRAY)) {
    return ASTNodes::Statement();
  }

  // New on sized array types
  else if (_unmangled_name == "Del" && _args.size() == 1 &&
           ASTNodes::type(_args.front()).nodes.front().type ==
               Type::TypeNode::SIZED_ARRAY) {
    const auto size = ASTNodes::type(_args.front())
                          .nodes.front()
                          .sized_array_size;

    ASTNodes::Statement out;
    for (uint i = 0; i < size; ++i) {
      ASTNodes::RawCFormat arg;
      arg.format_string = "(%)[" + std::to_string(i) + "]";
      arg.args = {_args.front()};

      arg.type = ASTNodes::type(_args.front());
      arg.type->nodes.pop_front();

      out.children.push_back(
          get_function_call_node("Del", {arg}));
    }
    return out;
  }

  // Pointer copy
  else if (_unmangled_name == "Copy" && _args.size() == 2 &&
           ASTNodes::type(_args.back()).nodes.front().type ==
               Type::TypeNode::POINTER &&
           ASTNodes::type(_args.front())
               .cast_match(ASTNodes::type(_args.back()))) {
    ASTNodes::RawCFormat out;
    out.args = {_args.front(), _args.back()};
    out.type = Type({"void"});
    out.format_string = "% = %";
    return out;
  }

  // Special case: Local fn pointer
  else if (scope_manager.contains(_unmangled_name) &&
           std::holds_alternative<Type>(
               scope_manager.get(_unmangled_name).value())) {
    const Type local_var_type =
        scope_manager.at<Type>(_unmangled_name);

    if (local_var_type.is_fn_ptr()) {
      const auto fn_type = local_var_type.deref();
      const auto needed_args = fn_type.fn_args();

      if (_args.size() != needed_args.size()) {
        throw std::runtime_error(
            "Expected " + std::to_string(needed_args.size()) +
            " args in fn pointer call, but saw " +
            std::to_string(_args.size()));
      }

      auto args_at_i = _args.begin();
      for (uint i = 0;
           i < needed_args.size() && args_at_i != _args.end();
           ++i, ++args_at_i) {
        if (!needed_args[i].second.exact_match(
                ASTNodes::type(*args_at_i))) {
          throw std::runtime_error(
              "Expected type '" +
              needed_args[i].second.oak_repr() + "' for arg " +
              std::to_string(i) +
              " of fn pointer call, but saw type '" +
              ASTNodes::type(*args_at_i).oak_repr() + "'");
        }
      }

      ASTNodes::Call out;
      out.return_type = fn_type.fn_return_type();
      out.mangled_c_fn_name = _unmangled_name;

      auto needed_arg = needed_args.begin();
      for (const auto &arg : _args) {
        ASTNodes::Call::Arg to_add;

        to_add.name = ASTNodes::OptBox(arg);
        to_add.type = ASTNodes::type(arg);
        to_add.derefs = 0;

        if (!to_add.type.cast_match(needed_arg->second)) {
          to_add.derefs = -1;
          for (Type t = to_add.type.ref();
               !t.exact_match(needed_arg->second);
               t = t.deref()) {
            ++to_add.derefs;
          }
        }

        out.args.push_back(to_add);
        ++needed_arg;
      }

      return out;
    } else {
      throw std::runtime_error(
          "Cannot call variable with non-function-pointer "
          "type '" +
          local_var_type.oak_repr(_unmangled_name) + "'");
    }
  }

  // End special cases
  //////////////////////////////////////////////////////////////

  return scope_manager.get_fn(_unmangled_name, _args);
}

ASTNodes::Node Parser::parse_function_call(TokenStream &_pos) {
  debug_print();
  if (settings.debug) {
    settings.ostream << __FUNCTION__ << " at "
                     << _pos.cur().file.string() << ":"
                     << _pos.cur().line << "." << _pos.cur().col
                     << '\n';
  }

  // Special case: size!
  if (_pos.cur() == "size!") {
    _pos.next();
    if (_pos.cur() != "(") {
      throw std::runtime_error(
          "Invalid size! macro: Expected '(', but saw '" +
          _pos.cur().text + "'");
    }
    _pos.next();

    // One type argument
    ASTNodes::RawCFormat out;
    out.type = Type({"uint"});
    out.format_string =
        "sizeof(" + parse_type(_pos).c_repr() + ")";

    _pos.next();
    if (_pos.cur() != ")") {
      throw std::runtime_error(
          "Invalid size! macro: Expected ')', but saw '" +
          _pos.cur().text + "'");
    }

    return out;
  }

  // Function call
  const std::string unmangled_name = _pos.cur();

  _pos.next();
  if (_pos.cur() != "(") {
    throw std::runtime_error("Expected function call: Saw '" +
                             _pos.cur().text + "'");
  }
  _pos.next();

  std::list<ASTNodes::Node> args;
  while (_pos.cur() != ")") {
    if (_pos.cur() != ",") {
      args.push_back(parse_object(_pos));
    }
    _pos.next();
  }

  return get_function_call_node(unmangled_name, args);
}

ASTNodes::Node Parser::parse_object(TokenStream &_pos) {
  debug_print();
  if (settings.debug) {
    settings.ostream << __FUNCTION__ << " at "
                     << _pos.cur().file.string() << ":"
                     << _pos.cur().line << "." << _pos.cur().col
                     << '\n';
  }

  /*
  object = name | function_call | object . name ;
  */

  // Open parenthesis immediately: No-capture lambda
  if (_pos.cur() == "(") {
    // Create name
    uint lambda_counter = 1;
    while (scope_manager.contains(
        "__oak_lambda_" + std::to_string(lambda_counter))) {
      ++lambda_counter;
    }
    const std::string lambda_name =
        "__oak_lambda_" + std::to_string(lambda_counter);

    // Parse function
    scope_manager.push_capture_frame();
    parse_function({lambda_name}, _pos);

    const auto captures = scope_manager.get_captures();
    for (const auto &capture : captures) {
      settings.warn("Illegal capture " + capture);
    }

    scope_manager.pop_frame();

    // Return a fn pointer to that lambda
    ASTNodes::Object out;
    out.type =
        std::get<FnInfo>(
            scope_manager.at<ScopeManager::FnValue>(lambda_name)
                .back())
            .t;
    out.raw_text = out.type.mangle(lambda_name);
    return out;
  }

  // Open parenthesis follows: Function call
  else if (_pos.peek(1).text == "(") {
    return parse_function_call(_pos);
  }

  // Var instance
  auto cur = _pos.cur();
  const auto literal_type = Lexer::get_literal_type(cur);
  if (literal_type.has_value()) {
    // Literal
    ASTNodes::Object out;
    out.raw_text = cur;
    out.type = literal_type.value();
    return out;
  }
  // Name
  uint derefs = 0;
  while (!_pos.done() && _pos.cur() == "^") {
    ++derefs;
    _pos.next();
  }

  if (_pos.done()) {
    throw std::runtime_error(
        "'^' operator must operate on a variable.");
  }

  std::string name = _pos.cur();
  auto value = scope_manager.get(name);
  if (!value.has_value()) {
    throw std::runtime_error("Failed to resolve object '" +
                             name + "'");
  }

  // Variable instance: Not fn ptr
  if (std::holds_alternative<Type>(value.value())) {
    Type t = std::get<Type>(value.value());

    // Derefs
    if (derefs > 0) {
      for (uint i = 0; i < derefs; ++i) {
        name = "(*" + name + ")";

        if (!t.nodes.empty() &&
            t.nodes.front().type == Type::TypeNode::POINTER) {
          t.nodes.pop_front();
        } else {
          throw std::runtime_error(
              "Cannot dereference non-pointer type '" +
              t.oak_repr() + "'");
        }
      }
    }

    // Member access
    while (_pos.peek(1).text == ".") {
      _pos.next(); // Pointing at .
      _pos.next(); // Pointing at member name
      const auto member_name = _pos.cur().text;

      // Auto-deref for member access
      while (!t.nodes.empty() &&
             t.nodes.front().type == Type::TypeNode::POINTER) {
        name = "(*" + name + ")";
        t = t.deref();
      }

      const auto info =
          scope_manager.get(t.struct_name()).value();

      if (std::holds_alternative<StructInfo>(info)) {
        const StructInfo struct_info =
            std::get<StructInfo>(info);

        if (!struct_info.members.contains(member_name)) {
          throw std::runtime_error(
              "Struct '" + t.struct_name() +
              "' has no member '" + member_name + "'");
        }

        t = struct_info.members.at(member_name);
      } else {
        const EnumInfo enum_info = std::get<EnumInfo>(info);

        if (!enum_info.options.contains(member_name)) {
          throw std::runtime_error("Enum '" + t.struct_name() +
                                   "' has no option '" +
                                   member_name + "'");
        }

        t = enum_info.options.at(member_name);
      }

      name += "." + member_name;
    }

    ASTNodes::Object out;
    out.raw_text = name;
    out.type = t;
    return out;
  }

  // If we're out here, it should be a fn ptr
  if (!std::holds_alternative<ScopeManager::FnValue>(
          value.value())) {
    throw std::runtime_error(
        "Symbol '" + name +
        "' is neither a variable instance nor a function");
  }

  auto l = std::get<ScopeManager::FnValue>(value.value());
  if (l.size() != 1) {
    throw std::runtime_error(
        "Function pointers can only be taken when the target "
        "function name has no type overloads");
  } else if (!std::holds_alternative<FnInfo>(l.front())) {
    throw std::runtime_error(
        "Cannot use template as an object");
  }

  // Fn ptr
  auto instance = std::get<FnInfo>(l.front());
  ASTNodes::Object out;
  out.raw_text = "(&" + instance.t.mangle(name) + ")";
  out.type = instance.t.ref();
  return out;
}

bool TemplateInfo::does_provide(
    const std::list<std::list<std::string>> &_substitutions,
    const std::list<std::string> &_desired) const {
  debug_print();

  std::list<std::string> tokenized;
  for (const auto &i : provides_block) {
    tokenized.push_back(i);
  }

  const auto will_provide = TemplateInfo::replace(
      tokenized, generics, _substitutions);

  if (will_provide.size() != _desired.size()) {
    return false;
  }
  auto l = will_provide.begin();
  auto r = _desired.begin();
  while (l != will_provide.end() && r != _desired.end()) {
    if (*l != *r) {
      return false;
    }
    ++l, ++r;
  }
  return true;
}

std::list<std::string> TemplateInfo::replace(
    const std::list<std::string> &_to_augment,
    const std::list<std::string> &_generics,
    const std::list<std::list<std::string>> &_replacements) {
  debug_print();
  // Ensure valid substitutions
  if (_generics.size() < _replacements.size()) {
    throw std::runtime_error(
        "Too many generic substitutions provided! Expected "
        "<= " +
        std::to_string(_generics.size()) + ", but got " +
        std::to_string(_replacements.size()));
  }

  // Build substitution map
  std::map<std::string, std::list<std::string>>
      substitution_map;
  auto generic = _generics.begin();
  auto substitution = _replacements.begin();
  while (substitution != _replacements.end()) {
    substitution_map[*generic] = *substitution;
    ++generic;
    ++substitution;
  }

  // Replace
  std::list<std::string> out;
  for (const auto &t : _to_augment) {
    if (substitution_map.contains(t)) {
      for (const auto &replacement : substitution_map.at(t)) {
        out.push_back(replacement);
      }
    } else {
      out.push_back(t);
    }
  }
  return out;
}

bool Parser::instantiate(
    TemplateInfo &_what,
    const std::list<std::list<std::string>> &_substitutions) {
  debug_print();
  // Check for existing instances
  if (_what.existing_instances.contains(_substitutions)) {
    return true;
  }

  // Build validation block
  auto replaced_validation_block = Lexer::tokify(
      TemplateInfo::replace(_what.validate_block,
                            _what.generics, _substitutions),
      _what.path, _what.line, _what.col);

  // Run validation block
  Parser tester = *this;
  try {
    tester.parse_global(replaced_validation_block);
  } catch (...) {
    return false;
  }

  // Replace the instantiation block
  auto replaced_instantiation_block = Lexer::tokify(
      TemplateInfo::replace(_what.instantiate_block,
                            _what.generics, _substitutions),
      _what.path, _what.line, _what.col);

  // Struct name fix
  if (_what.provides_block.size() == 1 &&
      _what.provides_block.front() == "struct") {
    auto it = std::next(replaced_instantiation_block.begin());
    const auto repl = TemplateInfo::replace(
        _what.generics, _what.generics, _substitutions);

    it->text += "_GEN_";
    bool first = true;
    for (const auto &gen : repl) {
      if (first) {
        first = false;
      } else {
        it->text += "JOIN_";
      }
      it->text += gen + "_";
    }
    it->text += "ENDGEN";
  }

  if (settings.debug) {
    settings.ostream << "Parsing template block:\n```oak\n";
    uint counter = 0;
    for (const auto &tok : replaced_instantiation_block) {
      settings.ostream << tok.text << ' ';

      counter += tok.text.size() + 1;
      if (counter >= 32) {
        settings.ostream << '\n';
        counter = 0;
      }
    }
    settings.ostream << "\n```\n";
  }

  // Run instantiation block
  try {
    parse_global(replaced_instantiation_block);
  } catch (std::runtime_error &e) {
    std::string msg;
    for (const auto &item : replaced_instantiation_block) {
      if (!msg.empty()) {
        msg += " ";
      }
      msg += item;
    }
    throw std::runtime_error(
        "During instantiation of template block:\n" + msg +
        "\nError occurred:\n" + e.what());
  } catch (...) {
    std::string msg;
    for (const auto &item : replaced_instantiation_block) {
      if (!msg.empty()) {
        msg += " ";
      }
      msg += item;
    }
    throw std::runtime_error(
        "During instantiation of template block:\n" + msg +
        "\nUnknown error occurred.");
  }

  // Log any success
  _what.existing_instances.insert(_substitutions);

  return true;
}

void Parser::validate_type(const Type &_t) {
  debug_print();
  for (const auto &node : _t.nodes) {
    if (node.type == Type::TypeNode::LITERAL) {
      if (Type::is_built_in_type(node.literal_name)) {
        continue;
      }

      const auto entry = scope_manager.get(node.literal_name);

      if (!entry.has_value() ||
          !(std::holds_alternative<StructInfo>(entry.value()) ||
            std::holds_alternative<EnumInfo>(entry.value()))) {
        throw std::runtime_error("Atomic type '" +
                                 node.literal_name +
                                 "' does not exist.");
      }
    }
  }
}

/**
 * @brief Runs a command (asserting that it succeeded), logging
 * cout to the returned string
 * @param _cmd The system command to execute
 * @returns The output of that command, given that it succeeded
 */
std::string get_cmd_output(const std::string &_cmd) {
  debug_print();
  char buffer[128];
  std::string result;
  FILE *pipe = popen(_cmd.c_str(), "r");

  if (!pipe) {
    throw std::runtime_error("'popen' failed for command '" +
                             std::string(_cmd) + "'");
  }

  memset(buffer, '\0', 128);
  while (fgets(buffer, 128, pipe) != nullptr) {
    result.append(buffer, strnlen(buffer, 128));
    memset(buffer, '\0', 128);
  }

  int code = pclose(pipe) / 256;
  if (code != 0) {
    throw std::runtime_error("Command '" + std::string(_cmd) +
                             "' failed with error code " +
                             std::to_string(code));
  }

  return result;
}

std::string
Macros::strip_string_literal(const std::string &_str_lit) {
  debug_print();
  const static std::set<char> str_chars = {'\'', '"', '`'};

  // Strip \" and the likes from within
  std::string out = _str_lit;
  while (out.front() == out.back() &&
         str_chars.contains(out.front())) {
    std::string tmp;
    for (uint i = 1; i + 1 < out.size(); ++i) {
      if (i + 2 < out.size() && out[i] == '\\') {
        ++i;
      }
      tmp.push_back(out[i]);
    }
    out = tmp;
  }
  return out;
}

std::string
Macros::make_string_literal(const std::string &_contents) {
  std::string out = "\"";

  for (uint i = 0; i < _contents.size(); ++i) {
    if (_contents[i] == '"' || _contents[i] == '\\') {
      out += "\\";
    }
    out += _contents[i];
  }

  out += "\"";
  return out;
}

std::list<std::list<Lexer::Token>>
Macros::get_macro_args(TokenStream &_pos, const bool &_erase) {
  debug_print();

  // Points to name
  const auto range_start = _pos.tell();
  uint depth = 0;

  std::list<std::list<Lexer::Token>> out;
  std::list<Lexer::Token> cur;

  do {
    _pos.next();

    if (_pos.cur() == "(") {
      ++depth;
      if (depth == 1) {
        continue;
      }
    } else if (_pos.cur() == ")") {
      --depth;
      if (depth == 0) {
        break;
      }
    }

    if (depth == 1 && _pos.cur() == ",") {
      if (!cur.empty()) {
        out.push_back(cur);
        cur.clear();
      }
    } else {
      cur.push_back(_pos.cur());
    }
  } while (!_pos.done());
  if (!cur.empty()) {
    out.push_back(cur);
  }
  _pos.next();

  if (_erase) {
    // Delete everything related to macro call
    const auto first_after_range = _pos.tell();
    _pos.erase(range_start, first_after_range);
  }

  // Leave pointing to item after call
  return out;
}

// Points to macro name after 'let'. Can be inline or
// functional. Erases all traces after done
void Parser::parse_macro(
    TokenStream &_pos, const uint64_t &_preproc_passes_allowed,
    const std::list<std::string> &_names) {
  debug_print();
  if (settings.debug) {
    settings.ostream << __FUNCTION__ << " at "
                     << _pos.cur().file.string() << ":"
                     << _pos.cur().line << "." << _pos.cur().col
                     << '\n';
  }

  // Either '=' or '('
  if (_pos.cur().text == "=") {
    // Scan until ;
    InlineMacro info;

    _pos.next();
    while (!_pos.done() && _pos.cur().text != ";") {
      info.contents.push_back(_pos.cur());
      _pos.next();
    }

    for (const auto &name : _names) {
      scope_manager.add(name, info);
    }
  } else if (_pos.cur().text == "(") {
    for (const auto &name : _names) {
      // Scrape definition
      std::list<Lexer::Token> contents;
      contents.push_back(Lexer::Token(_pos.cur(), "let"));
      contents.push_back(Lexer::Token(_pos.cur(), "main"));

      // Until first "{"
      while (_pos.cur().text != "{") {
        contents.push_back(_pos.cur());
        _pos.next();

        if (_pos.done()) {
          throw std::runtime_error(
              "Functional macro definition '" + _names.front() +
              "' must be followed by body");
        }
      }

      contents.push_back(_pos.cur());
      _pos.next();

      uint count = 1;
      while (count != 0) {
        if (_pos.cur().text == "{") {
          ++count;
        } else if (_pos.cur().text == "}") {
          --count;
        }

        contents.push_back(_pos.cur());
        if (_pos.done()) {
          throw std::runtime_error(
              "Functional macro '" + _names.front() +
              "' has no ending curly brace");
        }

        _pos.next();
      }
      _pos.prev();

      // Write to file
      const std::filesystem::path source_path =
          _pos.cur().file.string() + "." +
          name.substr(0, name.size() - 1) + ".macro.oak";
      const std::filesystem::path executable_path =
          source_path.string() + ".out";

      // Skip compilation if possible
      bool do_compile = true;
      if (std::filesystem::exists(executable_path)) {
        const auto src_last_write =
            std::filesystem::last_write_time(_pos.cur().file);
        const auto exe_last_write =
            std::filesystem::last_write_time(executable_path);

        if (src_last_write < exe_last_write) {
          do_compile = false;
        }
      }

      if (do_compile) {
        std::ofstream f(source_path);
        if (!f.is_open()) {
          throw std::runtime_error(
              "Failed to write macro source file '" +
              source_path.string() + "'");
        }

        uint64_t cur_line = 1;

        for (const auto &item : contents) {
          if (item.line != cur_line) {
            f << '\n';
            cur_line = item.line;
          } else {
            f << ' ';
          }
          f << item.text;
        }
        f.close();

        // Compile to executable
        std::stringstream macro_compilation_log;
        OakCompiler oc(macro_compilation_log);
        oc.settings.compile_settings().preprocess_pass_limit =
            _preproc_passes_allowed;
        oc.settings.compile_settings().do_syntax_check = false;
        oc.settings.compile_settings().mode = Settings::
            CompileSettings::TRANSLATE_COMPILE_AND_LINK;
        oc.settings.compile_settings().entry_point =
            source_path;
        oc.settings.compile_settings().target = executable_path;

        if (oc.settings.debug) {
          oc.settings.ostream << "Compiling macro '" << name
                              << "' from " << source_path
                              << " to " << executable_path
                              << '\n';
        }

        try {
          oc();
        } catch (OutOfPPPLError &e) {
          throw OutOfPPPLError(
              "During compilation of macro '" + name +
              "': Surpassed preprocessor pass limit! "
              "Self-referential macro is likely");
        } catch (std::runtime_error &e) {
          throw std::runtime_error(
              "From macro compiler:\n" +
              macro_compilation_log.str() +
              "\nDuring compilation of macro '" + name +
              "':\n" + e.what());
        } catch (...) {
          throw std::runtime_error(
              "From macro compiler:\n" +
              macro_compilation_log.str() +
              "\nUnknown error occurred during "
              "compilation of macro '" +
              name + "'");
        }
      }

      // Save executable
      CompiledMacro c;
      c.executable = executable_path;
      scope_manager.add(name, c);
    }
  } else {
    throw std::runtime_error(
        "Malformed macro definition for " + _names.front() +
        ": Expected '=' or '(', but saw '" + _pos.cur().text +
        "'");
  }

  // Leave pointing to first after
  _pos.next();
}

ASTNodes::Call Parser::resolve_fn_call(
    const std::string &_name,
    const std::list<ASTNodes::Object> &_args) {
  const std::string failure_msg =
      "No matching function found for call '" +
      fn_call_str(_name, _args) + "'";

  std::list<ASTNodes::Node> args_in_disguise;
  for (const auto &arg : _args) {
    args_in_disguise.push_back(arg);
  }

  try {
    return scope_manager.get_fn(_name, args_in_disguise);
  } catch (std::runtime_error &) {
  }

  // Only used for error checking
  const auto raw = scope_manager.get(_name);
  if (!raw.has_value() ||
      !std::holds_alternative<ScopeManager::FnValue>(
          raw.value())) {
    throw std::runtime_error(failure_msg);
  }

  // Template case
  // Do any templates
  try {
    // Find signature
    // TODO: Make this suck less
    // Note: This is immediately converted to std::string,
    // so the file, line, and col don't matter
    uint64_t junk_line = 0, junk_col = 0;
    std::list<std::string> signature = {"let", _name, "("};
    for (const auto &arg : _args) {
      // Ignore on first arg
      if (signature.size() != 3) {
        signature.push_back(",");
      }

      // Anonymous arg name
      signature.push_back("_");
      signature.push_back(":");

      // Arg type
      for (const auto &tok :
           Lexer::lex(arg.type.oak_repr(), "NULL", junk_line,
                      junk_col)) {
        signature.push_back(tok.text);
      }
    }
    signature.push_back(")");
    // Note: No return type!

    // If there exist some substitutions such that some
    // template exactly matches the signature, do that
    for (auto &entry :
         scope_manager.at<ScopeManager::FnValue>(_name)) {
      if (!std::holds_alternative<
              std::shared_ptr<TemplateInfo>>(entry)) {
        continue;
      }
      auto t = std::get<std::shared_ptr<TemplateInfo>>(entry);

      const auto substitutions =
          t->find_substitutions(_name, signature);
      if (substitutions.has_value()) {
        instantiate(*t, substitutions.value());

        // Don't allow templates this time!
        return scope_manager.get_fn(_name, args_in_disguise);
      }
    }
  } catch (std::runtime_error &_e) {
    throw std::runtime_error("Error during template checking "
                             "requested by function call '" +
                             fn_call_str(_name, _args) +
                             "':\n" + _e.what());
  } catch (...) {
  }
  throw std::runtime_error(
      "Unknown error during template checking "
      "requested by function call '" +
      fn_call_str(_name, _args) + "'");
}

void Parser::syntax_check(const std::filesystem::path &_fp,
                          const std::string &_text) const {
  debug_print();
  // All detected errors: line, col, message
  std::list<std::tuple<uint64_t, uint64_t, std::string>> errors;

  // Scan for syntax errors here

  uint64_t line = 1, col = 0;
  std::stack<char> enclosure;
  size_t i;

  auto incr = [&]() {
    if (col == 65) {
      errors.push_back({line, col, "Line too long!"});
    }
    ++i, ++col;
  };

  for (i = 0; i < _text.size(); ++i, ++col) {
    // Comments
    if (_text.at(i) == '/' && i + 1 < _text.size() &&
        _text.at(i + 1) == '/') {
      while (i < _text.size() && _text.at(i) != '\n') {
        incr();
      }
      if (_text.at(i) == '\n') {
        ++line, col = 0;
      }
    } else if (_text.at(i) == '/' && i + 1 < _text.size() &&
               _text.at(i + 1) == '*') {
      while (i + 1 < _text.size() &&
             !(_text.at(i) == '*' && _text.at(i + 1) == '/')) {
        if (_text.at(i) == '\n') {
          ++line, col = 0;
        }
        incr();
      }
      incr();
    }

    // Strings
    if (_text.at(i) == '\'') {
      bool skip = false;
      incr();
      while (i < _text.size()) {
        if (skip) {
          skip = false;
        } else if (_text.at(i) == '\\') {
          skip = true;
          incr();
          continue;
        } else if (_text.at(i) == '\'') {
          break;
        } else if (_text.at(i) == '\n') {
          ++line, col = 0;
          break;
        }
        incr();
      }
    } else if (_text.at(i) == '"') {
      bool skip = false;
      incr();
      while (i < _text.size()) {
        if (skip) {
          skip = false;
        } else if (_text.at(i) == '\\') {
          skip = true;
          incr();
          continue;
        } else if (_text.at(i) == '"') {
          break;
        } else if (_text.at(i) == '\n') {
          ++line, col = 0;
          break;
        }
        incr();
      }
    } else if (_text.at(i) == '`') {
      bool skip = false;
      incr();
      while (i < _text.size()) {
        if (skip) {
          skip = false;
        } else if (_text.at(i) == '\\') {
          skip = true;
          incr();
          continue;
        } else if (_text.at(i) == '`') {
          break;
        } else if (_text.at(i) == '\n') {
          ++line, col = 0;
          break;
        }
        incr();
      }
    }

    else {
      // Everything else
      switch (_text.at(i)) {
      case '\n':
        ++line, col = 0;
        break;
      case '[':
      case '(':
      case '{':
        enclosure.push(_text.at(i));
        break;
      case ']':
        if (enclosure.empty()) {
          errors.push_back({line, col, "Too many ']'"});
        } else if (enclosure.top() != '[') {
          errors.push_back({line, col,
                            std::string("Tried to end '") +
                                enclosure.top() +
                                "' with ']'"});
        } else {
          enclosure.pop();
        }
        break;
      case ')':
        if (enclosure.empty()) {
          errors.push_back({line, col, "Too many ')'"});
        } else if (enclosure.top() != '(') {
          errors.push_back({line, col,
                            std::string("Tried to end '") +
                                enclosure.top() +
                                "' with ')'"});
        } else {
          enclosure.pop();
        }
        break;
      case '}':
        if (enclosure.empty()) {
          errors.push_back({line, col, "Too many '}'"});
        } else if (enclosure.top() != '{') {
          errors.push_back({line, col,
                            std::string("Tried to end '") +
                                enclosure.top() +
                                "' with '}'"});
        } else {
          enclosure.pop();
        }
        break;
      }

      if (col == 65) {
        errors.push_back({line, col, "Line too long!"});
      }
    }
  }

  // If errors were found, throw them
  if (!errors.empty()) {
    for (const auto &p : errors) {
      settings.ostream << _fp.string() << ":" << std::get<0>(p)
                       << "." << std::get<1>(p) << "> '"
                       << std::get<2>(p) << "'\n";
    }

    throw std::runtime_error(std::to_string(errors.size()) +
                             " syntax error(s) occurred.");
  }
}

void Parser::fix_math(TokenStream &_pos) {
  debug_print();
  if (settings.debug) {
    settings.ostream << __FUNCTION__ << " at "
                     << _pos.cur().file.string() << ":"
                     << _pos.cur().line << "." << _pos.cur().col
                     << '\n';
  }

  // Iterate through the token stream, replace all instances of
  // the given operator with the given op fn call name (EG '+'
  // -> 'Add'). Precedence is embedded in the order in which you
  // call this lambda
  const auto resolve_binary_operator =
      [&](const std::map<std::string, std::string> &_ops) {
        // Scan stream
        for (_pos.reset();
             !_pos.done() && _pos.cur().text != ";";
             _pos.next()) {
          // On match
          if (_pos.cur().type == "OPERATOR" &&
              _ops.contains(_pos.cur().text)) {
            std::list<Lexer::Token>::iterator
                first_of_lhs, // First tok in lhs
                first_after_lhs =
                    _pos.tell(), // The single-token op
                first_after_rhs; // First tok after rhs

            // Find lhs
            first_of_lhs = std::prev(_pos.tell());
            if (_pos.at_beg() || *first_of_lhs == "(") {
              throw std::runtime_error(
                  "At " + _pos.cur().file.string() + ":" +
                  std::to_string(_pos.cur().line) + "." +
                  std::to_string(_pos.cur().col) +
                  "> Malformed operator '" +
                  first_after_lhs->text + "' LHS");
            } else if (*first_of_lhs == ")") {
              int depth = 0;
              do {
                if (*first_of_lhs == "(") {
                  ++depth;
                } else if (*first_of_lhs == ")") {
                  --depth;
                }
                --first_of_lhs;
              } while (depth != 0);
              if (first_of_lhs->type != "ID") {
                ++first_of_lhs;
              }

              // Erase matched parenthesis
              while (first_of_lhs->text == "(" &&
                     std::prev(first_after_lhs)->text == ")") {
                first_of_lhs = _pos.erase(first_of_lhs);
                _pos.erase(std::prev(first_after_lhs));
              }
            }
            while (std::prev(first_of_lhs)->text == ".") {
              first_of_lhs = std::prev(first_of_lhs, 2);
            }

            // Find rhs
            first_after_rhs = std::next(_pos.tell());
            if (first_after_rhs->type == "EOF" ||
                *first_after_rhs == ")") {
              throw std::runtime_error(
                  "At " + _pos.cur().file.string() + ":" +
                  std::to_string(_pos.cur().line) + "." +
                  std::to_string(_pos.cur().col) +
                  "> Malformed operator '" +
                  first_after_lhs->text + "' LHS");
            } else if (first_after_rhs->text == "(") {
              // Parenthesization
              auto start_pos = first_after_rhs;
              int depth = 1;
              do {
                ++first_after_rhs;
                if (first_after_rhs->text == "(") {
                  ++depth;
                } else if (first_after_rhs->text == ")") {
                  --depth;

                  if (depth == 0) {
                    ++first_after_rhs;
                  }
                }
              } while (depth != 0);

              // Erase matched parenthesis
              while (start_pos->text == "(" &&
                     std::prev(first_after_rhs)->text == ")") {
                start_pos = _pos.erase(start_pos);
                _pos.erase(std::prev(first_after_rhs));
              }
            } else if (std::next(first_after_rhs)->text ==
                       "(") {
              // Function call
              int depth = 0;
              do {
                ++first_after_rhs;
                if (first_after_rhs->text == "(") {
                  ++depth;
                } else if (first_after_rhs->text == ")") {
                  --depth;

                  if (depth == 0) {
                    ++first_after_rhs;
                  }
                }
              } while (depth != 0);
            } else {
              ++first_after_rhs;
            }

            while (first_after_rhs->text == ".") {
              first_after_rhs = std::next(first_after_rhs, 2);
            }

            // Operate
            // "lhs _operator rhs" -> "_op_name ( lhs , rhs )"
            // "lhs _operator (...)" -> "_op_name ( lhs , ... )"
            _pos.insert(
                first_of_lhs,
                Lexer::Token(_pos.cur(),
                             _ops.at(first_after_lhs->text)));
            _pos.insert(first_of_lhs,
                        Lexer::Token(_pos.cur(), "("));

            // Separating comma
            first_after_lhs->text = ",";
            first_after_lhs->type = "OPERATOR";

            // End parenthesis
            _pos.insert(first_after_rhs,
                        Lexer::Token(_pos.cur(), ")"));
          }
        }
      };

  // Same, but for prefix unary operators
  // Note: There are no suffix unary operators in oak, and all
  // prefix unary operators have the same precendence
  const auto resolve_unary_operator =
      [&](const std::string &_operator,
          const std::string &_op_name) {
        // Scan stream
        for (_pos.reset();
             !_pos.done() && _pos.cur().text != ";";
             _pos.next()) {
          // On match
          if (_pos.cur().type == "OPERATOR" &&
              _pos.cur().text == _operator) {
            std::list<Lexer::Token>::iterator
                first_after_lhs =
                    _pos.tell(), // The single-token op
                first_after_rhs; // First tok after rhs

            // Find rhs
            first_after_rhs = std::next(_pos.tell());
            if (_pos.at_beg() || *first_after_rhs == ")") {
              throw std::runtime_error(
                  "At " + _pos.cur().file.string() + ":" +
                  std::to_string(_pos.cur().line) + "." +
                  std::to_string(_pos.cur().col) +
                  "> Malformed operator '" + _operator +
                  "' LHS");
            } else if (std::next(first_after_rhs)->text ==
                       "(") {
              int depth = 0;
              do {
                ++first_after_rhs;
                if (*first_after_rhs == "(") {
                  ++depth;
                } else if (*first_after_rhs == ")") {
                  --depth;

                  if (depth == 0) {
                    ++first_after_rhs;
                  }
                }
              } while (depth != 0);
            } else {
              ++first_after_rhs;
            }

            while (first_after_rhs->text == ".") {
              first_after_rhs = std::next(first_after_rhs, 2);
            }

            // Operate
            // "_operator rhs" -> "_op_name ( rhs )"
            _pos.insert(first_after_lhs,
                        Lexer::Token(_pos.cur(), _op_name));
            *first_after_lhs = Lexer::Token(_pos.cur(), "(");
            _pos.insert(first_after_rhs,
                        Lexer::Token(_pos.cur(), ")"));
          }
        }
      };

  // Unary operator precedence
  const static std::list<std::pair<std::string, std::string>>
      unary_precedence = {{"!", "Not"},
                          {"++", "Incr"},
                          {"--", "Decr"},
                          {"~", "Flip"}};

  // Binary operator precedence
  const static std::list<std::map<std::string, std::string>>
      binary_precedence = {
          {
              {"&", "And"},
              {"|", "Or"},
          },
          {
              {"*", "Mult"},
              {"/", "Div"},
              {"%", "Mod"},
          },
          {
              {"+", "Add"},
              {"-", "Sub"},
          },
          {
              {"==", "Eq"},
              {"!=", "Neq"},
          },
          {{"<", "Less"},
           {">", "Great"},
           {"<=", "Leq"},
           {">=", "Greq"}},
          {
              {"&&", "Andd"},
              {"||", "Orr"},
          },
          {
              {"=", "Copy"},
              {"&=", "AndEq"},
              {"|=", "OrEq"},
              {"<<=", "LBSEq"},
              {">>=", "RBSEq"},
              {"*=", "MultEq"},
              {"/=", "DivEq"},
              {"%=", "ModEq"},
              {"+=", "AddEq"},
              {"-=", "SubEq"},
              {"&&=", "AnddEq"},
              {"||=", "OrrEq"},
          },
      };

  for (const auto &i : unary_precedence) {
    resolve_unary_operator(i.first, i.second);
  }

  for (const auto &i : binary_precedence) {
    resolve_binary_operator(i);
  }
}

void Parser::load_dialect_file(
    const std::filesystem::path &_path) {
  debug_print();

  // Parse that file
  OakCompiler c(settings.ostream);
  Settings::CompileSettings &csettings =
      c.settings.compile_settings();
  csettings = settings.compile_settings();
  csettings.mode = Settings::CompileSettings::NOTHING;
  csettings.entry_point = _path;
  c();

  // pragma!("export_dialect", "one_rule_name");
  if (csettings.pragmas.at(_path).contains("export_dialect")) {
    // for (const auto &rule : c.rules.rules) {
    //   rules.register_rule(rule.first, rule.second);
    // }
    // for (const auto &bundle : c.rules.bundles) {
    //   rules.register_bundle(bundle.first, bundle.second);
    // }
    std::cerr << __FILE__ << ":" << __LINE__
              << "> Unimplemented\n"
              << std::flush;

    settings.dialect =
        csettings.pragmas.at(_path).at("export_dialect");
  } else {
    throw std::runtime_error(
        "Dialect file '" + _path.string() +
        "' does contain `pragma!(\"export_dialect\", "
        "\"...\");`");
  }
}

std::filesystem::path
Parser::resolve_path(const std::string &_requested,
                     const std::filesystem::path &_cur_file) {
  const auto local = _cur_file.parent_path() / _requested;
  const auto global =
      settings.compile_settings().include_path / _requested;
  const bool global_exists = std::filesystem::exists(global);

  if (std::filesystem::exists(local)) {
    if (global_exists &&
        std::filesystem::canonical(global) !=
            std::filesystem::canonical(local)) {
      settings.warn("Choosing local file " + _requested +
                    " over package file of same name");
    }
    return std::filesystem::canonical(local);
  } else if (global_exists) {
    return std::filesystem::canonical(global);
  } else {
    throw std::runtime_error("Failed to resolve file '" +
                             _requested + "'");
  }
}

void Parser::do_file(const std::string &_path,
                     const std::filesystem::path &_cur_file) {
  debug_print();

  const auto path = resolve_path(_path, _cur_file);

  Settings::CompileSettings &csettings =
      settings.compile_settings();

  if (csettings.visited.contains(path)) {
    if (settings.debug) {
      settings.ostream << "Ignoring repeat inclusion " << path
                       << '\n';
    }
    return;
  }

  if (settings.debug) {
    settings.ostream << "Visiting file " << path << '\n';
  }

  const auto path_write_time =
      std::filesystem::last_write_time(path);
  if (path_write_time > csettings.most_recent_mod_time) {
    csettings.most_recent_mod_time = path_write_time;
  }
  csettings.visited.insert(path);

  // Load and lex
  if (!std::filesystem::exists(path)) {
    throw std::runtime_error("File " + path.string() +
                             " does not exist.");
  } else if (!std::filesystem::is_regular_file(path)) {
    throw std::runtime_error(
        "File " + path.string() +
        " exists, but is not a regular file.");
  }

  std::string text;
  std::ifstream source(path);
  if (!source.is_open()) {
    throw std::runtime_error("Failed to open file " +
                             path.string());
  }

  text.assign(std::istreambuf_iterator<char>(source),
              std::istreambuf_iterator<char>());
  source.close();

  uint64_t line = 1, col = 0;
  TokenStream token_stream({});

  try {
    token_stream = Lexer::lex(text, path, line, col, true);
  } catch (OutOfPPPLError &e) {
    // If requested, dump
    if (csettings.dump_file.has_value()) {
      dump(*csettings.dump_file.value(), token_stream);
    }

    throw OutOfPPPLError("Error occurred while lexing " +
                         path.string() + ":\n" + e.what());
  } catch (std::runtime_error &e) {
    // If requested, dump
    if (csettings.dump_file.has_value()) {
      dump(*csettings.dump_file.value(), token_stream);
    }

    throw std::runtime_error("Error occurred while lexing " +
                             path.string() + ":\n" + e.what());
  } catch (...) {
    // If requested, dump
    if (csettings.dump_file.has_value()) {
      dump(*csettings.dump_file.value(), token_stream);
    }

    throw std::runtime_error(
        "An unknown error occurred while lexing " +
        path.string() + "");
  }

  // If requested, syntax check
  if (csettings.do_syntax_check) {
    syntax_check(path, text);
    token_stream.reset();
  }

  // Do actual parsing here
  parse_global(token_stream);
  token_stream.reset();

  // If requested, dump
  if (csettings.dump_file.has_value()) {
    dump(*csettings.dump_file.value(), token_stream);
  }

  if (settings.debug) {
    settings.ostream << "Exiting file " << path << '\n';
  }
}

bool Parser::replace_macro(TokenStream &_pos) {
  debug_print();
  if (settings.debug) {
    settings.ostream << __FUNCTION__ << " at "
                     << _pos.cur().file.string() << ":"
                     << _pos.cur().line << "." << _pos.cur().col
                     << '\n';
  }

  const auto name_tok = _pos.cur();
  const std::string name =
      _pos.cur().text.substr(0, _pos.cur().text.find('!') + 1);
  const std::string nonexistence_replacement =
      _pos.cur().text.substr(_pos.cur().text.find('!') + 1);

  if (name == "LINE!") {
    _pos.tell()->text = std::to_string(_pos.cur().line) + "u64";
    Lexer::classify_type(*_pos.tell());
    return true;
  } else if (name == "COL!") {
    _pos.tell()->text = std::to_string(_pos.cur().col) + "u64";
    Lexer::classify_type(*_pos.tell());
    return true;
  } else if (name == "FILE!") {
    _pos.tell()->text = '"' + _pos.cur().file.string() + '"';
    Lexer::classify_type(*_pos.tell());
    return true;
  } else if (name == "oak_VERSION!") {
    _pos.tell()->text = '"' + acorn_version + '"';
    Lexer::classify_type(*_pos.tell());
    return true;
  } else if (name == "SYSTEM!") {
    _pos.tell()->type = "STRING";
#if (defined(WIN32) || defined(WINNT))
    _pos.tell()->text = "\"WINDOWS\"";
#elif (defined(unix) || defined(__unix__))
    _pos.tell()->text = "\"UNIX\"";
#elif (defined(__APPLE__) || defined(__MACH__))
    _pos.tell()->text = "\"OSX\"";
#else
    _pos.tell()->text = "\"OTHER\"";
#endif
    Lexer::classify_type(*_pos.tell());
    return true;
  }

  if (Macros::reserved_macro_names.contains(name)) {
    // Just skip it
    Macros::get_macro_args(_pos);
    return false;
  }

  const auto res = scope_manager.get(name);
  if (!res.has_value()) {
    if (!nonexistence_replacement.empty()) {
      _pos.tell()->text = nonexistence_replacement;
      Lexer::classify_type(*_pos.tell());
    } else {
      throw std::runtime_error("Macro '" + name +
                               "' has no definition");
    }
  } else if (std::holds_alternative<InlineMacro>(res.value())) {
    // Inline
    const auto to_remove = _pos.tell();
    for (const auto &item :
         std::get<InlineMacro>(res.value()).contents) {
      _pos.insert(to_remove, Lexer::Token(name_tok, item));
    }
    _pos.seek(std::prev(to_remove));
    _pos.erase(to_remove);
  } else if (_pos.peek(1).text == "(") {
    // Functional
    auto args = Macros::get_macro_args(_pos, true);

    const auto exe =
        std::get<CompiledMacro>(res.value()).executable;

    if (!std::filesystem::exists(exe)) {
      throw std::runtime_error("Compiled macro " +
                               exe.string() +
                               " does not exist!");
    }

    // Prepare call
    std::string command = exe;
    for (const auto &arg : args) {
      std::string arg_text;
      for (const auto &tok : arg) {
        if (!arg_text.empty()) {
          arg_text.push_back(' ');
        }
        arg_text += tok;
      }
      command += " " + Macros::make_string_literal(arg_text);
    }

    if (settings.debug) {
      settings.ostream << "Running macro call '" << command
                       << "'\n";
    }

    // Run call and get replacement
    const auto replacement = get_cmd_output(command);

    if (settings.debug) {
      settings.ostream << "Macro returned text:\n```oak\n"
                       << replacement << "\n```\n";
    }

    // Lex replacement
    uint64_t junk_line = 0, junk_col = 0;
    auto lexed_replacement = Lexer::lex(
        replacement, name_tok.file, junk_line, junk_col);

    // Do replacement
    for (const auto &t : lexed_replacement) {
      Lexer::Token to_insert = t;
      to_insert.file = name_tok.file;
      to_insert.line = name_tok.line;
      to_insert.col = name_tok.col;
      _pos.insert(_pos.tell(), to_insert);
    }

    // Decr one
    _pos.prev();
  } else {
    // Error
    throw std::runtime_error(
        "Compiled macro '" + name +
        "' must be invoked as a function call.");
  }
  return true;
}
