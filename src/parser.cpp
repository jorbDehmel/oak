/**
 * @file
 */

#include "parser.hpp"
#include "ast_node.hpp"
#include "debug.hpp"
#include "lexer.hpp"
#include "oakc.hpp"
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

// Parse a global scope
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
      if (_pos.cur() == "let") {
        _pos.next();
        std::list<std::string> names;
        names.push_back(_pos.cur());
        _pos.next();

        // Plural instantiation
        while (_pos.cur() == ",") {
          _pos.next();
          names.push_back(_pos.cur());
          _pos.next();
        }

        // Generics
        std::list<std::string> generics;
        if (_pos.cur() == "<") {
          // Zero or more comma-separated generics
          do {
            _pos.next();
            if (!is_valid_struct_name(_pos.cur())) {
              settings.warn(
                  "Generic '" + _pos.cur().text + "' at " +
                  _pos.cur().file.string() + ":" +
                  std::to_string(_pos.cur().line) + "." +
                  std::to_string(_pos.cur().col) +
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
          // Struct, enum, or invalid global definition
          _pos.next();
          if (_pos.cur() == "struct") {
            _pos.next(); // Now pointing at body

            if (generics.empty()) {
              parse_struct(names, _pos);
            } else if (_pos.cur() == ";") {
              throw std::runtime_error(
                  "Generic struct signatures are illegal");
            } else {
              // Add definition for generic struct(s)
              TemplateInfo info(_pos.cur().file,
                                _pos.cur().line,
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
                specific_info.instantiate_block.push_front(
                    name);
                specific_info.instantiate_block.push_front(
                    "let");
                scope_manager.add(name, specific_info);
              }
            }

            _pos.next();
          } else if (_pos.cur() == "enum") {
            _pos.next();

            if (generics.empty()) {
              parse_enum(names, _pos);
            } else {
              throw std::runtime_error(
                  "Generic enums are unimplemented");
            }

            _pos.next();
          } else {
            throw std::runtime_error(
                "Global scope 'let' error: Expected 'struct' "
                "or 'enum', saw '" +
                _pos.cur().text + "'");
          }
        } else if (_pos.cur() == "(") {
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
        } else if (_pos.cur() == "=") {
          // In Oak: let to = from;
          // In C++: using to = from;
          _pos.next();
          const auto from_name = _pos.cur().text;

          for (const auto &to_name : names) {
            scope_manager.alias(to_name, from_name);
          }

          _pos.next();
        } else {
          throw std::runtime_error(
              "Global scope 'let' error: "
              "Expected '(', '=' or ':', saw '" +
              _pos.cur().text + "'");
        }
      } else if (_pos.cur() == ";") {
        _pos.next();
      }

      else if (_pos.cur() == "compile_time_error!") {
        settings.ostream
            << _pos.cur().file.string() << ":"
            << _pos.cur().line << "." << _pos.cur().col << ">"
            << _pos.cur().text << " Compile-time error:\n";

        // Note: This is after all preprocessing
        const auto args = Macros::get_macro_args_no_erase(_pos);
        std::string msg;
        for (const auto &arg : args) {
          msg += arg.text + " ";
        }
        settings.ostream << msg << '\n';
        throw std::runtime_error(msg);
      } else if (_pos.cur() == "compile_time_warning!") {
        std::stringstream msg_strm;
        msg_strm << _pos.cur().file.string() << ":"
                 << _pos.cur().line << "." << _pos.cur().col
                 << ">" << _pos.cur().text
                 << " Compile-time warning:\n";

        // Note: This is after all preprocessing
        const auto args = Macros::get_macro_args_no_erase(_pos);
        for (const auto &arg : args) {
          msg_strm << arg.text << " ";
        }
        msg_strm << '\n';
        settings.warn(msg_strm.str());

        while (_pos.cur() != ";") {
          _pos.next();
        }
      } else if (_pos.cur() == "compile_time_print!") {
        settings.ostream
            << _pos.cur().file.string() << ":"
            << _pos.cur().line << "." << _pos.cur().col << ">"
            << _pos.cur().text << " Compile-time print:\n";

        // Note: This is after all preprocessing
        const auto args = Macros::get_macro_args_no_erase(_pos);
        std::string msg;
        for (const auto &arg : args) {
          msg += arg.text + " ";
        }
        settings.ostream << msg << '\n';

        while (_pos.cur() != ";") {
          _pos.next();
        }
      }

      else {
        throw std::runtime_error(
            "Global scope parse error: Unexpected token '" +
            _pos.cur().text + "'");
      }
    } catch (OutOfPPPLError &) {
      throw;
    } catch (std::runtime_error &e) {
      throw std::runtime_error(
          "At " + _pos.cur().file.string() + ":" +
          std::to_string(_pos.cur().line) + "." +
          std::to_string(_pos.cur().col) + "\n" + e.what());
    } catch (...) {
      if (_pos.done()) {
        throw;
      }
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
        if (info.tags.at("file") ==
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
      const auto info = std::get<StructInfo>(data);
      _where << "struct " << info.name << " {\n";
      for (const auto &item : info.member_order) {
        _where << info.members.at(item).c_repr(item) << ";\n";
      }
      _where << "};\n";
    } else if (std::holds_alternative<EnumInfo>(data)) {
      const auto info = std::get<EnumInfo>(data);
      _where << "struct " << info.name << "{enum{\n";
      for (const auto &item : info.option_order) {
        _where << info.name << "_OPT_" << item << ",";
      }
      _where << "}__info;union{\n";
      for (const auto &item : info.option_order) {
        _where << info.options.at(item).c_repr(item) << ";";
      }
      _where << "}__data;};\n";
    } else if (std::holds_alternative<FnInfo>(data)) {
      const auto info = std::get<FnInfo>(data);
      if (info.tags.contains("casual") &&
          info.tags.at("casual") == "true") {
        continue;
      } else if (info.tags.contains("autogen") &&
                 info.tags.at("autogen") == "true") {
        _where << "// autogen\n";
      }

      if (info.name == "main") {
        if (info.tags.at("file") ==
            settings.compile_settings().entry_point) {
          _where << info.t.c_repr(info.name, true);
          _where << "{";
          ASTNodes::reconstruct(info.n, _where);
          _where << ";}\n";
        }
      } else {
        _where << info.t.c_repr(info.name, false);
        _where << "{";
        ASTNodes::reconstruct(info.n, _where);
        _where << ";}\n";
      }
    }
  }
}

/// Dump to the given stream
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
    settings.ostream << __FUNCTION__ << " at "
                     << _pos.cur().file.string() << ":"
                     << _pos.cur().line << "." << _pos.cur().col
                     << " '" << _pos.cur().text << "'\n";
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
    to_add.tags = {{"casual", "true"}};

    // Erase autogen
    for (const auto &name : _names) {
      scope_manager.drop_fn_with_tag(name, "autogen", "true");
    }
  } else {
    // Implementation

    // Add some signatures for recursion
    FnInfo temp_info = to_add;
    temp_info.tags = {{"casual", "true"}};
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

    // Erase signatures
    for (const auto &name : _names) {
      scope_manager.drop_fn_with_tag(name, "casual", "true");
      scope_manager.drop_fn_with_tag(name, "autogen", "true");
    }
  }

  for (const auto &name : _names) {
    to_add.name = name;
    scope_manager.add(name, to_add);
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

/// Parses the (pre, post) regions of a template if they
/// exist. This should be called after any generic body
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
        for (uint i = 0; i < templates.at(original_name).size();
             ++i) {
          if (templates.at(original_name)
                  .at(i)
                  .does_provide(replacements, {"struct"})) {
            if (templates.at(original_name)
                    .at(i)
                    .attempt_instantiation(*this, replacements,
                                           settings)) {
              success = true;
              break;
            }
          } else if (templates.at(original_name)
                         .at(i)
                         .does_provide(replacements,
                                       {"enum"})) {
            if (templates.at(original_name)
                    .at(i)
                    .attempt_instantiation(*this, replacements,
                                           settings)) {
              success = true;
              break;
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

    scope_manager.add(name, to_add);

    // Constructor and destructor autogen go here
    uint64_t line = _pos.cur().line, col = _pos.cur().col;
    Lexer lexer;

    // Create a constructor to parse
    std::string to_lex = "(self: ^" + name + ") -> void { ";
    for (const auto &member : to_add.member_order) {
      to_lex += "New(self." + member + "); ";
    }
    to_lex += "}";

    // Parse and mark as autogen
    auto to_parse =
        lexer.lex(to_lex, _pos.cur().file, line, col);
    parse_function({"New"}, to_parse);
    scope_manager.tag_fn("New", "autogen", "true");

    // Reset, create destructor
    to_lex = "(self: ^" + name + ") -> void {";
    for (auto it = to_add.member_order.rbegin();
         it != to_add.member_order.rend(); ++it) {
      to_lex += "Del(self." + *it + ");";
    }
    to_lex += "}";

    // Parse and mark
    to_parse = lexer.lex(to_lex, _pos.cur().file, line, col);
    parse_function({"Del"}, to_parse);
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

    scope_manager.add(name, to_add);

    // Wrappers
    // wrap_a(self, what)
    for (const auto &p : to_add.options) {
      const auto wrapper_name = "wrap_" + p.first;
      FnInfo to_add;
      to_add.name = wrapper_name;

      to_add.tags["file"] = _pos.cur().file;
      to_add.tags["line"] = std::to_string(_pos.cur().line);
      to_add.tags["col"] = std::to_string(_pos.cur().col);

      // Construct wrapper type
      to_add.t.append_fn();
      to_add.t.nodes.back().following_arg_name = "self";
      to_add.t.append_ptr();
      to_add.t.append_literal(name); // enum name
      to_add.t.append_join();
      to_add.t.nodes.back().following_arg_name = "__data";
      to_add.t.append_type(p.second);
      to_add.t.append_maps();
      to_add.t.append_literal("void");

      // Node
      ASTNodes::RawCFormat child;

      // clang-format off
      child.format_string =
        "{ self->__info = " + name + "_OPT_" + p.first +
        "; self->__data." + p.first +
        " = __data; }";
      // clang-format on

      to_add.n.children = {ASTNodes::RawCFormat()};
      to_add.tags["autogen"] = "true";

      // Insert fn
      scope_manager.add(wrapper_name, to_add);
    }

    // Constructor, destructor here
    uint64_t line = _pos.cur().line, col = _pos.cur().col;
    Lexer lexer;

    // Create a constructor to parse
    const std::string op = to_add.option_order.front();
    const std::string text = "(self: ^" + name +
                             ") -> void {"
                             "let __data: " +
                             to_add.options.at(op).oak_repr() +
                             "; wrap_" + op +
                             "(self, __data);"
                             "}";
    auto to_parse = lexer.lex(text, _pos.cur().file, line, col);

    // Parse and mark as autogen
    parse_function({"New"}, to_parse);
    scope_manager.tag_fn("New", "autogen", "true");

    // Create destructor
    // NOTE: Enum destructors are not override-able
    std::string to_lex = "(self: ^" + name +
                         ") -> void {\n"
                         "match (self) {\n";

    for (const auto &option : to_add.option_order) {
      to_lex +=
          "case " + option + "(" +
          to_add.options.at(option).ref().oak_repr("data") +
          ") {\n"
          "Del(data);\n"
          "}\n";
    }
    to_lex.append("}\n}");

    line = _pos.cur().line;
    col = _pos.cur().col;

    debug_print();
    to_parse = lexer.lex(to_lex, _pos.cur().file, line, col);
    parse_function({"Del"}, to_parse);
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

  // A statement can be a function call, a (possibly compound)
  // if statement, a match statement, nothing, a variable
  // declaration, or a while statement

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

    return ASTNodes::Statement({out});
  }

  if (_pos.cur() == ";") {
    // Unit statement
    return ASTNodes::Statement();
  } else if (_pos.cur() == "let") {
    // Variable declaration
    // Collect names
    std::set<std::string> names;

    do {
      // Fluff
      _pos.next();

      // Name
      names.insert(_pos.cur());
      _pos.next();
    } while (_pos.cur() == ",");

    if (_pos.cur() != ":") {
      throw std::runtime_error(
          "Expected ':' after 'let' statement. Instead saw '" +
          _pos.cur().text + "'");
    }
    _pos.next();

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
      TokenStream new_call(
          {Lexer::Token("New", tok.file, tok.line, tok.col,
                        "ID"),
           Lexer::Token("(", tok.file, tok.line, tok.col,
                        "OPERATOR"),
           Lexer::Token(name, tok.file, tok.line, tok.col,
                        "ID"),
           Lexer::Token(")", tok.file, tok.line, tok.col,
                        "OPERATOR")});
      out.new_calls.push_back(parse_function_call(new_call));
    }

    return ASTNodes::Statement({out});
  } else if (_pos.cur() == "{") {
    // Scope
    ASTNodes::Statement out;

    // Add a frame to the scope stack
    scope_manager.push_frame();

    _pos.next();
    while (_pos.cur() != "}") {
      out.children.push_back(parse_statement(_pos));
      _pos.next();
    }

    // Remove that scope frame
    out.children.push_back(scope_manager.pop_frame());
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
    *out.condition = parse_object(_pos);

    if (!ASTNodes::type(*out.condition)
             .cast_match(Type({"bool"}))) {
      throw std::runtime_error(
          "Statement condition type '" +
          ASTNodes::type(*out.condition).oak_repr() +
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
    ASTNodes::Statement body = parse_statement(_pos);

    out.then_body = body;

    // Optional else clause
    _pos.next();
    if (!_pos.done() && _pos.cur() == "else") {
      // Else clause
      _pos.next();
      out.else_body = parse_statement(_pos);
    } else {
      _pos.prev();
    }

    return ASTNodes::Statement({out});
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
    *out.condition = parse_object(_pos);
    if (!ASTNodes::type(*out.condition)
             .cast_match(Type({"bool"}))) {
      throw std::runtime_error(
          "Statement condition type '" +
          ASTNodes::type(*out.condition).oak_repr() +
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
    return ASTNodes::Statement({out});
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

    const auto enum_name = target_type.struct_name();

    const auto info = scope_manager.at<EnumInfo>(enum_name);

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
      new_target.args = {target};
      target = new_target;
    }
    out.upon = target;

    if (_pos.cur() != "{") {
      throw std::runtime_error(
          "Missing opening curly brace in 'match' statement.");
    }
    _pos.next();

    while (_pos.cur() != "}") {
      out.branches.push_back(
          parse_case(info, _pos, is_mutable));
      _pos.next();
    }

    return ASTNodes::Statement({out});
  } else if (_pos.cur() == "return") {
    // Return statement
    ASTNodes::Return out;
    _pos.next();
    if (_pos.cur() != ";") {
      out.value = parse_object(_pos);

      if (!settings.compile_settings()
               .cur_return_type.exact_match(
                   out.value.value().type)) {
        throw std::runtime_error(
            "Invalid return type '" +
            out.value.value().type.oak_repr() +
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
    return ASTNodes::Statement({out});
  } else {
    // Function call
    auto ret = parse_function_call(_pos);
    _pos.next();
    if (_pos.cur() != ";") {
      throw std::runtime_error(
          "Missing semicolon after function call.");
    }
    return ASTNodes::Statement({ret});
  }
}

/// Assumes we are pointing to "case" or "else"
/// Non-global (inside match statement)
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
    out.body = parse_statement(_pos);

    // Pop frame, calling destructors
    out.body.children.push_back(scope_manager.pop_frame());

    // Pop from locals stack WITHOUT CALLING DESTRUCTOR ON
    // CAPTURE
    scope_manager.pop_frame();

    return out;
  } else if (_pos.cur() == "else") {
    // Statement
    _pos.next();
    ASTNodes::Statement out;
    out.children = {parse_statement(_pos)};
    return out;
  } else {
    throw std::runtime_error(
        "Error within match statement: Expected 'case' or "
        "'else', but saw '" +
        _pos.cur().text + "'");
  }
}

/// Parse a function call
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
    throw std::runtime_error("Expected function call");
  }
  _pos.next();

  std::list<ASTNodes::Node> args;
  while (_pos.cur() != ")") {
    if (_pos.cur() != ",") {
      args.push_back(parse_object(_pos));
    }
    _pos.next();
  }

  //////////////////////////////////////////////////////////////
  // Special cases here

  // Array access via the 'Get' operator
  if (unmangled_name == "Get" && args.size() == 2 &&
      (ASTNodes::type(args.back()).cast_match(Type({"u128"})) ||
       ASTNodes::type(args.back())
           .cast_match(Type({"i128"}))) &&
      (ASTNodes::type(args.front()).nodes.front().type ==
           Type::TypeNode::SIZED_ARRAY ||
       ASTNodes::type(args.front()).nodes.front().type ==
           Type::TypeNode::UNSIZED_ARRAY)) {
    // Only resolvable at reconstruction-time
    ASTNodes::ArrAccess out;
    out.upon = std::make_shared(args.front());
    out.index = std::make_shared(args.back());
    return out;
  }

  // alloc!
  else if (unmangled_name == "alloc!") {
    // 1-arg
    if (args.size() == 1) {
      if (args.front().type.nodes.empty() ||
          args.front().type.nodes.front().type !=
              Type::TypeNode::POINTER) {
        throw std::runtime_error(
            "Expected pointer type for alloc!(into), instead "
            "saw '" +
            args.front().type->oak_repr() + "'");
      }

      out.node_type = Node::RAW_C_FMT;
      out.type = Type({"void"});
      out.children.push_back(out.children.front());
      out.c_name = "% = (" +
                   out.children.front().type->c_repr() +
                   ") malloc(sizeof(" +
                   out.children.front().type->deref().c_repr() +
                   ")); assert(% != NULL)";
      return out;
    }

    // 2-arg
    else if (args.size() == 2) {
      if (args.front().type.nodes.empty() ||
          args.front().type.nodes.front().type !=
              Type::TypeNode::UNSIZED_ARRAY) {
        throw std::runtime_error(
            "Expected unsized array type for alloc!(into, "
            "size), instead saw '" +
            args.front().type.oak_repr() + "'");
      }

      out.node_type = Node::RAW_C_FMT;
      out.type = Type({"void"});
      out.children.push_back(out.children.front());
      out.c_name = "% = (" +
                   out.children.front().type->c_repr() +
                   ") calloc(%, sizeof(" +
                   out.children.front().type->deref().c_repr() +
                   ")); assert(% != NULL)";
      return out;
    }

    // Error case
    else {
      throw std::runtime_error("alloc! takes 1 or 2 args.");
    }
  }

  // free!
  else if (unmangled_name == "free!") {
    if (out.children.size() != 1 ||
        args.front().type->nodes.empty() ||
        (args.front().type->nodes.front().type !=
             Type::TypeNode::POINTER &&
         args.front().type->nodes.front().type !=
             Type::TypeNode::UNSIZED_ARRAY)) {
      throw std::runtime_error(
          "Expected pointer or unsized array type for "
          "free!(to_free), instead saw '" +
          args.front().type->oak_repr() + "'");
    }

    out.node_type = Node::RAW_C_FMT;
    out.type = Type({"void"});
    out.c_name = "free((void *)(%))";
    return out;
  }

  // New on pointer or unsized array types
  else if (unmangled_name == "New" && args.size() == 1 &&
           (args.front().type->nodes.front().type ==
                Type::TypeNode::POINTER ||
            args.front().type->nodes.front().type ==
                Type::TypeNode::UNSIZED_ARRAY)) {
    out.node_type = Node::RAW_C_FMT;
    out.type = Type({"void"});
    out.c_name = "% = 0;";
    return out;
  }

  // New on atomic types
  else if (unmangled_name == "New" && args.size() == 1 &&
           Type::is_built_in_type(args.front().type.value())) {
    out.node_type = Node::RAW_C_FMT;
    out.type = Type({"void"});
    out.c_name = "% = 0;";
    return out;
  }

  // Del on atomic types
  else if (unmangled_name == "Del" && args.size() == 1 &&
           Type::is_built_in_type(args.front().type.value())) {
    out.node_type = Node::NONE;
    return out;
  }

  // Del on pointer or unsized array types
  else if (unmangled_name == "Del" && args.size() == 1 &&
           args.front().type.has_value() &&
           (args.front().type->nodes.front().type ==
                Type::TypeNode::POINTER ||
            args.front().type->nodes.front().type ==
                Type::TypeNode::UNSIZED_ARRAY)) {
    out.node_type = Node::NONE;
    return out;
  }

  // Pointer copy
  else if (unmangled_name == "Copy" && args.size() == 2 &&
           args.back().type->nodes.front().type ==
               Type::TypeNode::POINTER &&
           args.front().type->cast_match(*args.back().type)) {
    out.node_type = Node::RAW_C_FMT;
    out.type = Type({"void"});
    out.c_name = "% = %;";
    return out;
  }

  // Special case: Local fn pointer
  else if (!locals.empty() && out.token.has_value()) {
    for (auto frame = locals.rbegin(); frame != locals.rend();
         ++frame) {
      if (frame->contains(out.token.value().text)) {
        const Type local_var_type =
            frame->at(out.token.value().text);

        if (local_var_type.is_fn_ptr()) {
          const auto fn_type = local_var_type.deref();
          const auto needed_args = fn_type.fn_args();

          if (args.size() != needed_args.size()) {
            throw std::runtime_error(
                "Expected " +
                std::to_string(needed_args.size()) +
                " args in fn pointer call, but saw " +
                std::to_string(args.size()));
          }

          for (uint i = 0; i < needed_args.size(); ++i) {
            if (!needed_args[i].second.exact_match(
                    args[i].type.value())) {
              throw std::runtime_error(
                  "Expected type '" +
                  needed_args[i].second.oak_repr() +
                  "' for arg " + std::to_string(i) +
                  " of fn pointer call, but saw type '" +
                  args[i].type->oak_repr() + "'");
            }
          }

          out.type = fn_type.fn_return_type();
          out.c_name = out.token.value().text;
          return out;
        } else {
          throw std::runtime_error(
              "Cannot call variable with non-function-pointer "
              "type '" +
              local_var_type.oak_repr(out.token.value().text) +
              "'");
        }
      }
    }
  }

  // End special cases
  //////////////////////////////////////////////////////////////

  const auto to_return =
      scope_manager.get_fn(unmangled_name, args);
  if (!to_return.has_value()) {
    throw std::runtime_error("Call resolution failed");
  }
  return to_return.value();
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

  auto cur = _pos.cur();
  const auto literal_type = Lexer::get_literal_type(cur);
  if (literal_type.has_value()) {
    // Literal
    ASTNodes::Object out;
    out.raw_text = cur;
    out.type = literal_type.value();
    return out;
  } else {
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
    Type t = scope_manager.at<Type>(name);

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
      _pos.next(); // pointing at .
      _pos.next(); // pointing at member name
      const auto member_name = _pos.cur().text;

      // Auto-deref for member access
      while (!t.nodes.empty() &&
             t.nodes.front().type == Type::TypeNode::POINTER) {
        name = "(*" + name + ")";
        t = t.deref();
      }

      const auto info =
          scope_manager.get(t.struct_name()).value().get();

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
}

/// Returns whether the given substitutions would cause the
/// `provides_block` list to match the given list
bool TemplateInfo::does_provide(
    const std::list<std::list<std::string>> &_substitutions,
    const std::list<std::string> &_desired) const {
  debug_print();

  std::list<std::string> tokenized;
  for (const auto &i : provides_block) {
    tokenized.push_back(i);
  }

  const auto will_provide =
      replace(tokenized, generics, _substitutions);

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

/// Returns a list of tokens based on _to_augment wherein
/// all occurrences of generics are replaced with their
/// corresponding replacements
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

TokenStream TemplateInfo::instantiate(
    const std::list<std::list<std::string>> &_substitutions) {
  debug_print();
  // Check for existing instances
  if (existing_instances.contains(_substitutions)) {
    return true;
  }

  // Build validation block
  auto replaced_validation_block =
      Lexer::tokify(replace(validate, generics, _substitutions),
                    path, line, col);

  // Run validation block
  Parser backup = _p;
  try {
    _p.parse_global(replaced_validation_block);
  } catch (...) {
    _p = backup;
    return false;
  }

  // Replace the instantiation block
  auto replaced_instantiation_block = Lexer::tokify(
      replace(instantiate_block, generics, _substitutions),
      path, line, col);

  // Struct name fix
  if (provides_block.size() == 1 &&
      provides_block.front() == "struct") {
    auto it = std::next(replaced_instantiation_block.begin());
    const auto repl =
        replace(generics, generics, _substitutions);

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
    _p.parse_global(replaced_instantiation_block);
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
  existing_instances.insert(_substitutions);

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
          !(std::holds_alternative<StructInfo>(
                entry.value().get()) ||
            std::holds_alternative<EnumInfo>(
                entry.value().get()))) {
        throw std::runtime_error("Atomic type '" +
                                 node.literal_name +
                                 "' does not exist.");
      }
    }
  }
}

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

void Macros::replace(TokenStream &_pos,
                     const Settings &_csettings,
                     OakCompiler &_oakc) {
  debug_print();

  const auto name_tok = _pos.cur();
  const std::string name =
      _pos.cur().text.substr(0, _pos.cur().text.find('!') + 1);
  const std::string nonexistence_replacement =
      _pos.cur().text.substr(_pos.cur().text.find('!') + 1);

  if (name == "LINE!") {
    _pos.tell()->type = "NUMBER";
    _pos.tell()->text = std::to_string(_pos.cur().line) + "u64";
    return;
  } else if (name == "COL!") {
    _pos.tell()->type = "NUMBER";
    _pos.tell()->text = std::to_string(_pos.cur().col) + "u64";
    return;
  } else if (name == "FILE!") {
    _pos.tell()->type = "STRING";
    _pos.tell()->text = '"' + _pos.cur().file.string() + '"';
    return;
  } else if (name == "oak_VERSION!") {
    _pos.tell()->type = "STRING";
    _pos.tell()->text = '"' + OakCompiler::version + '"';
    return;
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
    return;
  }

  if (!macros.contains(name)) {
    if (!nonexistence_replacement.empty()) {
      _pos.tell()->text = nonexistence_replacement;
      Lexer::classify_type(*_pos.tell());
      return;
    } else {
      throw std::runtime_error("Macro '" + name +
                               "' has no definition");
    }
  }

  if (std::holds_alternative<InlineMacro>(macros.at(name))) {
    // Inline
    const auto to_remove = _pos.tell();
    for (const auto &item :
         std::get<Inline>(macros.at(name)).contents) {
      _pos.insert(to_remove, Lexer::Token(name_tok, item));
    }
    _pos.seek(std::prev(to_remove));
    _pos.erase(to_remove);
  } else if (_pos.peek(1).text == "(") {
    // Functional
    auto args = get_macro_args(_pos);
    _pos.next();

    const auto exe =
        std::get<CompiledMacro>(macros.at(name)).executable;

    if (!std::filesystem::exists(exe)) {
      throw std::runtime_error("Compiled macro " +
                               exe.string() +
                               " does not exist!");
    }

    // Prepare call
    std::string command = exe;
    for (const auto &arg : args) {
      TokenStream stream = arg;
      _oakc.preprocess(stream);
      std::string arg_text;
      for (const auto &tok : arg) {
        if (!arg_text.empty()) {
          arg_text.push_back(' ');
        }
        arg_text += tok;
      }
      command += " " + Macros::make_string_literal(arg_text);
    }

    if (_csettings.debug) {
      _csettings.ostream << "Running macro call '" << command
                         << "'\n";
    }

    // Run call and get replacement
    const auto replacement = get_cmd_output(command);

    if (_csettings.debug) {
      _csettings.ostream << "Macro returned text:\n```oak\n"
                         << replacement << "\n```\n";
    }

    // Lex replacement
    Lexer l;
    uint64_t junk_line = 0, junk_col = 0;
    auto lexed_replacement =
        l.lex(replacement, name_tok.file, junk_line, junk_col);

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
}

/// STRIPS QUOTES OFF OF a macro occurrence's
/// args. Then returns those args WITHOUT ERASURE. NO RECURSE
std::list<Lexer::Token>
Macros::get_macro_args_no_erase(TokenStream &_pos) {
  debug_print();

  // Points to name
  uint depth = 0;
  std::list<Lexer::Token> out;
  auto it = _pos.tell();
  Lexer::Token cur(_pos.cur(), "");
  cur.text.clear();

  do {
    ++it;

    if (*it == "(") {
      ++depth;
      if (depth == 1) {
        continue;
      }
    } else if (*it == ")") {
      --depth;
      if (depth == 0) {
        break;
      }
    }

    if (depth == 1 && *it == ",") {
      if (!cur.text.empty()) {
        out.push_back(cur);
        cur = Lexer::Token(*it, "");
      }
    } else {
      if (!cur.text.empty()) {
        cur.text.push_back(' ');
      }
      cur.text += it->text;
    }
  } while (!_pos.done());
  if (!cur.text.empty()) {
    out.push_back(cur);
  }

  return out;
}

std::list<std::list<Lexer::Token>>
Macros::get_macro_args(TokenStream &_pos) {
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
  const auto first_after_range = _pos.tell();

  // Delete everything related to macro call, leave pointing to
  // item after call
  _pos.erase(range_start, first_after_range);
  return out;
}

// Points to macro name after 'let'. Can be inline or
// functional. Erases all traces after done
std::variant<InlineMacro, CompiledMacro>
Parser::parse_macro(TokenStream &_pos,
                    const uint64_t &_preproc_passes_allowed) {
  debug_print();

  // let
  const auto range_start = std::prev(_pos.tell());

  // name!
  const auto name = _pos.cur().text;

  _pos.next();

  std::variant<InlineMacro, CompiledMacro> out;

  // Either '=' or '('
  if (_pos.cur().text == "=") {
    // Scan until ;
    InlineMacro info;

    _pos.next();
    while (!_pos.done() && _pos.cur().text != ";") {
      info.contents.push_back(_pos.cur());
      _pos.next();
    }

    out = info;
  } else if (_pos.cur().text == "(") {
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
            "Functional macro definition '" + name +
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
        throw std::runtime_error("Functional macro '" + name +
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
      oc.settings.compile_settings().mode =
          Settings::CompileSettings::TRANSLATE_COMPILE_AND_LINK;
      oc.settings.compile_settings().entry_point = source_path;
      oc.settings.compile_settings().target = executable_path;

      if (oc.settings.debug) {
        oc.settings.ostream
            << "Compiling macro '" << name << "' from "
            << source_path << " to " << executable_path << '\n';
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
            "\nDuring compilation of macro '" + name + "':\n" +
            e.what());
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
    out = c;
  } else {
    throw std::runtime_error(
        "Malformed macro definition for " + name +
        ": Expected '=' or '(', but saw '" + _pos.cur().text +
        "'");
  }

  // Erase range
  _pos.next();
  const auto first_after_range = _pos.tell();
  _pos.erase(range_start, first_after_range);

  return out;
}

ASTNodes::Call Parser::resolve_fn_call(
    const std::string &_name,
    const std::list<ASTNodes::Object> &_args) {
  const std::string failure_msg =
      "No matching function found for call '" +
      fn_call_str(_name, _args) + "'";

  auto candidate = scope_manager.get_fn(_name, _args);
  if (candidate.has_value()) {
    // No templates needed
    return candidate.value();
  }

  // Only used for error checking
  const auto raw = scope_manager.get(_name);
  if (!raw.has_value() ||
      !std::holds_alternative<ScopeManager::FnValue>(
          raw.value().get())) {
    throw std::runtime_error(failure_msg);
  }

  // Template case
  // Do any templates
  try {
    // Find signature
    // TODO: Make this suck less
    // Note: This is immediately converted to std::string,
    // so the file, line, and col don't matter
    Lexer l;
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
      for (const auto &tok : l.lex(arg.type.oak_repr(), "NULL",
                                   junk_line, junk_col)) {
        signature.push_back(tok.text);
      }
    }
    signature.push_back(")");
    // Note: No return type!

    // If there exist some substitutions such that some
    // template exactly matches the signature, do that
    for (auto &entry :
         scope_manager.at<ScopeManager::FnValue>(_name)) {
      if (!std::holds_alternative<TemplateInfo>(entry)) {
        continue;
      }
      TemplateInfo &t = std::get<TemplateInfo>(entry);

      const auto substitutions =
          t.find_substitutions(_name, signature);
      if (substitutions.has_value()) {
        t.instantiate(substitutions.value());

        // Don't allow templates this time!
        candidate = scope_manager.get_fn(_name, _args);
        if (candidate.has_value()) {
          return candidate.value();
        } else {
          throw std::runtime_error(failure_msg);
        }
      }
    }
  } catch (std::runtime_error &_e) {
    throw std::runtime_error("Error during template checking "
                             "requested by function call '" +
                             fn_call_str(_name, _args) +
                             "':\n" + _e.what());
  } catch (...) {
    throw std::runtime_error(
        "Unknown error during template checking "
        "requested by function call '" +
        fn_call_str(_name, _args) + "'");
  }
}
