/**
 * @file
 */

#include "parser.hpp"
#include "ast_node.hpp"
#include "debug.hpp"
#include "lexer.hpp"
#include "macro.hpp"
#include "settings.hpp"
#include "type.hpp"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <iostream>
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

ASTNodes::Statement
Parser::pop_frame(const ASTNodes::Statement &_old_node,
                  Settings &_settings) {
  debug_print();

  if (locals.empty()) {
    throw std::runtime_error("Cannot pop from empty context");
  }

  ASTNodes::Statement out = _old_node;
  const auto old_frame = locals.back();
  Lexer lexer;
  uint64_t line = 0, col = 0;

  out.children = _old_node.children;
  for (const auto &p : old_frame) {
    auto to_parse =
        lexer.lex("Del(" + p.first + ");", "NULL", line, col);

    // Parse and mark
    const Node destructor =
        parse_function_call(to_parse, _settings);

    auto first_return = out.children.begin();
    while (first_return != out.children.end() &&
           !(first_return->token.has_value() &&
             first_return->token->text == "return")) {
      ++first_return;
    }
    out.children.insert(first_return, destructor);
  }

  locals.pop_back();

  debug_print();

  return out;
}

// Parse a global scope
void Parser::parse_global(TokenStream &_pos,
                          Settings &_settings) {
  debug_print();
  if (_settings.debug) {
    _settings.ostream << __FUNCTION__ << " at "
                      << _pos.cur().file.string() << ":"
                      << _pos.cur().line << "."
                      << _pos.cur().col << '\n';
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
              _settings.warn(
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
              parse_struct(names, _pos, _settings);
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
                info.instantiate.push_back(_pos.cur());
                _pos.next();
              } while (count != 0);
              _pos.prev();

              const auto p = parse_template_pre_post(_pos);
              for (const auto &item : p.second) {
                info.instantiate.push_back(item);
              }
              info.validate = p.first;

              for (const auto &name : names) {
                TemplateInfo specific_info = info;
                specific_info.provides = {"struct"};
                specific_info.instantiate.push_front("struct");
                specific_info.instantiate.push_front(":");
                specific_info.instantiate.push_front(name);
                specific_info.instantiate.push_front("let");
                templates[name].push_back(specific_info);
              }
            }

            _pos.next();
          } else if (_pos.cur() == "enum") {
            _pos.next();

            if (generics.empty()) {
              parse_enum(names, _pos, _settings);
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
            parse_function(names, _pos, _settings);
          } else {
            // Grab rest of signature
            TemplateInfo info(_pos.cur().file, _pos.cur().line,
                              _pos.cur().col);
            info.generics = generics;

            // Finish parsing type
            while (!_pos.done() && _pos.cur() != "{") {
              if (_pos.peek(1) != ":") {
                info.provides.push_back(_pos.cur());
              } else {
                info.provides.push_back("_");
              }

              info.instantiate.push_back(_pos.cur());

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
              info.instantiate.push_back(_pos.cur());
              _pos.next();
            } while (count != 0);
            _pos.prev();

            // Parse pre and post blocks
            const auto p = parse_template_pre_post(_pos);
            info.validate = p.first;
            for (const auto &item : p.second) {
              info.instantiate.push_back(item);
            }

            // Add to template table
            for (const auto &name : names) {
              TemplateInfo instance_info = info;

              instance_info.provides.push_front(name);
              instance_info.provides.push_front("let");

              instance_info.instantiate.push_front(name);
              instance_info.instantiate.push_front("let");

              templates[name].push_back(instance_info);
            }
          }

          _pos.next();
        } else if (_pos.cur() == "=") {
          // In Oak: let to = from;
          // In C++: using to = from;
          _pos.next();
          const auto from_name = _pos.cur().text;

          for (const auto &to_name : names) {
            for (const auto &d : functions[from_name]) {
              functions[to_name].push_back(d);
            }
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
        _settings.ostream
            << _pos.cur().file.string() << ":"
            << _pos.cur().line << "." << _pos.cur().col << ">"
            << _pos.cur().text << " Compile-time error:\n";

        // Note: This is after all preprocessing
        const auto args =
            MacroManager::get_macro_args_no_erase(_pos);
        std::string msg;
        for (const auto &arg : args) {
          msg += arg.text + " ";
        }
        _settings.ostream << msg << '\n';
        throw std::runtime_error(msg);
      } else if (_pos.cur() == "compile_time_warning!") {
        std::stringstream msg_strm;
        msg_strm << _pos.cur().file.string() << ":"
                 << _pos.cur().line << "." << _pos.cur().col
                 << ">" << _pos.cur().text
                 << " Compile-time warning:\n";

        // Note: This is after all preprocessing
        const auto args =
            MacroManager::get_macro_args_no_erase(_pos);
        for (const auto &arg : args) {
          msg_strm << arg.text << " ";
        }
        msg_strm << '\n';
        _settings.warn(msg_strm.str());

        while (_pos.cur() != ";") {
          _pos.next();
        }
      } else if (_pos.cur() == "compile_time_print!") {
        _settings.ostream
            << _pos.cur().file.string() << ":"
            << _pos.cur().line << "." << _pos.cur().col << ">"
            << _pos.cur().text << " Compile-time print:\n";

        // Note: This is after all preprocessing
        const auto args =
            MacroManager::get_macro_args_no_erase(_pos);
        std::string msg;
        for (const auto &arg : args) {
          msg += arg.text + " ";
        }
        _settings.ostream << msg << '\n';

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
void Parser::reconstruct(std::ostream &_where,
                         const Settings::CompileSettings
                             &_csettings) const noexcept {
  debug_print();

  // Include std header
  _where << "#include \"oak/std/std_oak_header.h\"\n";

  // Struct and enum signatures
  for (const auto &g : global_types_order) {
    _where << "struct " << g << ";\n";
  }

  // Function signatures
  for (const auto &p : functions) {
    for (const auto &info : p.second) {
      if (info.name == "main") {
        if (info.tags.at("file") == _csettings.entry_point) {
          _where << info.t.c_repr(info.name, true) << ";\n";
        }
      }

      // Avoid duplicating aliases
      else if (p.first == info.name) {
        _where << info.t.c_repr(info.name, false) << ";\n";
      }
    }
  }

  // Struct and enum definitions
  for (const auto &name : global_types_order) {
    auto data = global_types.at(name);
    if (std::holds_alternative<StructInfo>(data)) {
      const auto info = std::get<StructInfo>(data);
      _where << "struct " << name << " {\n";
      for (const auto &item : info.member_order) {
        _where << info.members.at(item).c_repr(item) << ";\n";
      }
      _where << "};\n";
    } else {
      const EnumInfo info = std::get<EnumInfo>(data);
      _where << "struct " << name << "{enum{\n";
      for (const auto &item : info.option_order) {
        _where << name << "_OPT_" << item << ",";
      }
      _where << "}__info;union{\n";
      for (const auto &item : info.option_order) {
        _where << info.options.at(item).c_repr(item) << ";";
      }
      _where << "}__data;};\n";
    }
  }

  // Function definitions
  for (const auto &p : functions) {
    for (const auto &info : p.second) {
      if (info.tags.contains("casual") &&
          info.tags.at("casual") == "true") {
        continue;
      } else if (info.tags.contains("autogen") &&
                 info.tags.at("autogen") == "true") {
        _where << "// autogen\n";
      }

      if (info.name == "main") {
        if (info.tags.at("file") == _csettings.entry_point) {
          _where << info.t.c_repr(info.name, true);
          _where << "{";
          ASTNodes::reconstruct(info.n, _where);
          _where << ";}\n";
        }
      } else if (p.first == info.name) {
        _where << info.t.c_repr(info.name, false);
        _where << "{";
        ASTNodes::reconstruct(info.n, _where);
        _where << ";}\n";
      }
    }
  }
}

/// Dump to the given stream
void Parser::dump(std::ostream &_where, TokenStream &_pos,
                  const Settings::CompileSettings &_csettings)
    const noexcept {
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
    reconstruct(_where, _csettings);
  } catch (std::runtime_error &e) {
    _where << "// FAILURE: " << e.what() << '\n';
  } catch (...) {
    _where << "// UNKNOWN FAILURE\n";
  }

  _where << "// Structs and enums:\n";
  for (const auto &name : global_types_order) {
    _where << name << " [";
    auto data = global_types.at(name);
    if (std::holds_alternative<StructInfo>(data)) {
      const auto info = std::get<StructInfo>(data);
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
    } else {
      const auto info = std::get<EnumInfo>(data);
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
    }
  }

  _where << "// Function signatures:\n";
  for (const auto &p : functions) {
    _where << p.first << ":\n";
    for (const auto &item : p.second) {
      _where << "\t" << item.t.oak_repr(p.first) << '\n';
    }
  }
}

// Parse a single function declaration
// Assumes we have just seen "let NAME (" and are pointing to
// "("
void Parser::parse_function(
    const std::list<std::string> &_names, TokenStream &_pos,
    Settings &_settings) {
  debug_print();
  if (_settings.debug) {
    _settings.ostream << __FUNCTION__ << " at "
                      << _pos.cur().file.string() << ":"
                      << _pos.cur().line << "."
                      << _pos.cur().col << " '"
                      << _pos.cur().text << "'\n";
  }

  // Finish parsing type
  Type t = parse_type(_pos, _settings);
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
      if (functions.contains(name)) {
        for (auto it = functions.at(name).begin();
             it != functions.at(name).end(); ++it) {
          if (it->t.exact_match(t) &&
              it->tags.contains("autogen") &&
              it->tags.at("autogen") == "true") {
            auto to_delete = it;
            --it;
            functions.at(name).erase(to_delete);
          }
        }
      }
    }
  } else {
    // Implementation

    // Add some signatures for recursion
    FnInfo temp_info = to_add;
    temp_info.tags = {{"casual", "true"}};
    for (const auto &name : _names) {
      temp_info.name = name;
      functions[name].push_back(temp_info);
    }

    // Push stack frame w/ args
    std::map<std::string, Type> arg_map;
    auto args = t.fn_args();
    for (const auto &p : args) {
      arg_map[p.first] = p.second;
    }

    locals.push_back(arg_map);

    const auto backup =
        _settings.compile_settings().cur_return_type;
    _settings.compile_settings().cur_return_type =
        t.fn_return_type();

    to_add.n = parse_statement(_pos, _settings);

    _settings.compile_settings().cur_return_type = backup;

    // Pop stack frame WITHOUT CALLING ARGUMENT DESTRUCTORS
    locals.pop_back();

    // Erase signatures
    for (const auto &name : _names) {
      for (auto it = functions.at(name).begin();
           it != functions.at(name).end(); ++it) {
        if (to_add.t.exact_match(it->t)) {
          if (it->tags.contains("casual") &&
              it->tags.at("casual") == "true") {
            auto to_delete = it;
            --it;
            functions.at(name).erase(to_delete);
          } else if (it->tags.contains("autogen") &&
                     it->tags.at("autogen") == "true") {
            auto to_delete = it;
            --it;
            functions.at(name).erase(to_delete);
          }
        }
      }
    }
  }

  for (const auto &name : _names) {
    to_add.name = name;
    functions[name].push_back(to_add);
  }
}

// Parses a struct/enum's guts
std::list<std::pair<std::string, Type>>
Parser::parse_members(TokenStream &_pos, Settings &_settings) {
  debug_print();
  if (_settings.debug) {
    _settings.ostream << __FUNCTION__ << " at "
                      << _pos.cur().file.string() << ":"
                      << _pos.cur().line << "."
                      << _pos.cur().col << '\n';
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
    Type t = parse_type(_pos, _settings);
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
Type Parser::parse_type(TokenStream &_pos,
                        Settings &_settings) {
  debug_print();
  if (_settings.debug) {
    _settings.ostream << __FUNCTION__ << " at "
                      << _pos.cur().file.string() << ":"
                      << _pos.cur().line << "."
                      << _pos.cur().col << '\n';
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

    auto tmp = parse_object(_pos, _settings);

    _pos.next();
    if (_pos.cur() != ")") {
      throw std::runtime_error(
          "Malformed type! macro: Expected ')', but saw '" +
          _pos.cur().text + "'");
    }

    return tmp.type.value();
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

      if (!global_types.contains(
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
                                           _settings)) {
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
                                           _settings)) {
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
                          TokenStream &_pos,
                          Settings &_settings) {
  debug_print();
  if (_settings.debug) {
    _settings.ostream << __FUNCTION__ << " at "
                      << _pos.cur().file.string() << ":"
                      << _pos.cur().line << "."
                      << _pos.cur().col << '\n';
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
    const auto members = parse_members(_pos, _settings);

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
    if (global_types.contains(name) && // Disallow overwriting
        !(std::holds_alternative<StructInfo>(
              global_types.at(name)) && // Except for
                                        // structs
          std::get<StructInfo>(global_types.at(name))
                  .tags["casual"] ==
              "true") // That are only casually defined
    ) {

      // Construct the existing type as a str
      std::string existing_type_str;

      if (std::holds_alternative<StructInfo>(
              global_types.at(name))) {
        existing_type_str = "struct";
      } else if (std::holds_alternative<EnumInfo>(
                     global_types.at(name))) {
        existing_type_str = "enum";
      }

      // Throw appropriate error message
      throw std::runtime_error("Cannot replace " +
                               existing_type_str + " '" + name +
                               "' w/ struct of same name");
    } else if (!is_valid_struct_name(name)) {
      _settings.warn("Struct name \"" + name +
                     "\" does not seem to be camelcase");
    }

    global_types_order.push_back(name);
    global_types[name] = to_add;

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
    parse_function({"New"}, to_parse, _settings);
    functions.at("New").back().tags["autogen"] = "true";

    // Reset, create destructor
    to_lex = "(self: ^" + name + ") -> void {";
    for (auto it = to_add.member_order.rbegin();
         it != to_add.member_order.rend(); ++it) {
      to_lex += "Del(self." + *it + ");";
    }
    to_lex += "}";

    // Parse and mark
    to_parse = lexer.lex(to_lex, _pos.cur().file, line, col);
    parse_function({"Del"}, to_parse, _settings);
    functions.at("Del").back().tags["autogen"] = "true";
  }
}

// Parse a single enum declaration
// Assumes we have just seen "let NAME : enum" and are
// pointing to the next token.
void Parser::parse_enum(const std::list<std::string> &_names,
                        TokenStream &_pos,
                        Settings &_settings) {
  debug_print();
  if (_settings.debug) {
    _settings.ostream << __FUNCTION__ << " at "
                      << _pos.cur().file.string() << ":"
                      << _pos.cur().line << "."
                      << _pos.cur().col << '\n';
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
    const auto members = parse_members(_pos, _settings);

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
    if (global_types.contains(name) && // Disallow overwriting
        !(std::holds_alternative<EnumInfo>(
              global_types.at(name)) && // Except for
                                        // enums
          std::get<EnumInfo>(global_types.at(name))
                  .tags["casual"] ==
              "true") // That are only casually defined
    ) {
      // Construct the existing type as a str
      std::string existing_type_str;

      if (std::holds_alternative<StructInfo>(
              global_types.at(name))) {
        existing_type_str = "struct";
      } else if (std::holds_alternative<EnumInfo>(
                     global_types.at(name))) {
        existing_type_str = "enum";
      }

      // Throw appropriate error message
      throw std::runtime_error("Cannot replace " +
                               existing_type_str + " '" + name +
                               "' w/ enum of same name");
    } else if (!is_valid_struct_name(name)) {
      _settings.warn("Enum name \"" + name +
                     "\" does not seem to be camelcase");
    }

    global_types_order.push_back(name);
    global_types[name] = to_add;

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
      functions[wrapper_name].push_back(to_add);
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
    parse_function({"New"}, to_parse, _settings);
    functions.at("New").back().tags["autogen"] = "true";

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
    parse_function({"Del"}, to_parse, _settings);
  }
}

// Assumes we are pointing to the first token in the statement
Node Parser::parse_statement(TokenStream &_pos,
                             Settings &_settings,
                             const Type &_return_type) {
  debug_print();
  if (_settings.debug) {
    _settings.ostream << __FUNCTION__ << " at "
                      << _pos.cur().file.string() << ":"
                      << _pos.cur().line << "."
                      << _pos.cur().col << '\n';
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
    Node out(Node::RAW_C_FMT);
    out.c_name =
        MacroManager::strip_string_literal(_pos.cur().text);

    _pos.next();
    if (_pos.cur() != ")") {
      throw std::runtime_error(
          "Invalid c! macro: Expected ')', but saw '" +
          _pos.cur().text + "'");
    }
    _pos.next();

    return out;
  }

  if (_pos.cur() == ";") {
    // Unit statement
    Node out(Node::NONE);
    return out;
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
    Type t = parse_type(_pos, _settings);
    validate_type(t);

    ASTNodes::Declaration out;
    out.type = t;
    out.token = Lexer::Token(_pos.cur(), "");

    // Add all to symbol table
    for (const auto &name : names) {
      if (!out.token.value().text.empty()) {
        out.token.value().text += ", ";
      }
      out.token.value().text += name;

      locals.back()[name] = t;

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
      out.children.push_back(
          parse_function_call(new_call, _settings));
    }

    return out;
  } else if (_pos.cur() == "{") {
    // Scope
    ASTNodes::Statement out;

    // Add a frame to the scope stack
    locals.push_back({});

    _pos.next();
    while (_pos.cur() != "}") {
      out.children.push_back(parse_statement(_pos, _settings));
      _pos.next();
    }

    // Remove that scope frame
    out = pop_frame(out, _settings);

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
    Node condition = parse_object(_pos, _settings);
    if (!condition.type.value().cast_match(Type({"bool"}))) {
      throw std::runtime_error("Statement condition type '" +
                               condition.type->oak_repr() +
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
    Node body = parse_statement(_pos, _settings);

    Node out(Node::IF);
    out.children = {condition, body};

    // Optional else clause
    _pos.next();
    if (!_pos.done() && _pos.cur() == "else") {
      // Else clause
      _pos.next();
      out.children.push_back(parse_statement(_pos, _settings));
    } else {
      _pos.prev();
    }

    return out;
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
    Node condition = parse_object(_pos, _settings);
    if (!condition.type.value().cast_match(Type({"bool"}))) {
      throw std::runtime_error("Statement condition type '" +
                               condition.type->oak_repr() +
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
    Node body = parse_statement(_pos, _settings);

    Node out(Node::WHILE);
    out.children = {condition, body};
    return out;
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
    Node target = parse_object(_pos, _settings);
    Type target_type = target.type.value();

    const bool is_mutable = (target_type.nodes.front().type ==
                             Type::TypeNode::POINTER);
    if (is_mutable) {
      target_type = target_type.deref();
    }

    const auto enum_name = target_type.struct_name();

    if (!global_types.contains(enum_name) ||
        !std::holds_alternative<EnumInfo>(
            global_types.at(enum_name))) {
      throw std::runtime_error("Enum type '" + enum_name +
                               "' does not exist.");
    }

    const auto info =
        std::get<EnumInfo>(global_types.at(enum_name));

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
    Node out(Node::MATCH);
    out.c_name = enum_name;

    if (is_mutable) {
      Node new_target(Node::NodeType::RAW_C_FMT);
      new_target.c_name = "(*%)";
      new_target.children = {target};
      target = new_target;
    }
    out.children = {target};
    out.is_mutable_match = is_mutable;

    if (_pos.cur() != "{") {
      throw std::runtime_error(
          "Missing opening curly brace in 'match' statement.");
    }
    _pos.next();

    while (_pos.cur() != "}") {
      out.children.push_back(
          parse_case(info, _pos, _settings, is_mutable));
      _pos.next();
    }

    return out;
  } else if (_pos.cur() == "return") {
    // Return statement
    Node out(Node::STMT);
    out.token = _pos.cur();
    _pos.next();
    if (_pos.cur() != ";") {
      out.children = {parse_object(_pos, _settings)};

      if (!_settings.compile_settings()
               .cur_return_type.exact_match(
                   out.children.front().type.value())) {
        throw std::runtime_error(
            "Invalid return type '" +
            out.children.front().type.value().oak_repr() +
            "' for fn w/ return "
            "type '" +
            _settings.compile_settings()
                .cur_return_type.oak_repr() +
            "'");
      }
    } else if (!_settings.compile_settings()
                    .cur_return_type.exact_match({"void"})) {
      throw std::runtime_error(
          "Invalid return type 'void' for fn w/ return type '" +
          _settings.compile_settings()
              .cur_return_type.oak_repr() +
          "'");
    }
    return out;
  } else {
    // Function call
    Node ret = parse_function_call(_pos, _settings);
    _pos.next();
    if (_pos.cur() != ";") {
      throw std::runtime_error(
          "Missing semicolon after function call.");
    }
    return ret;
  }
}

/// Assumes we are pointing to "case" or "else"
/// Non-global (inside match statement)
ASTNodes::Case Parser::parse_case(const EnumInfo &_enum_type,
                                  TokenStream &_pos,
                                  Settings &_settings,
                                  const bool &_is_mutable) {
  debug_print();
  if (_settings.debug) {
    _settings.ostream << __FUNCTION__ << " at "
                      << _pos.cur().file.string() << ":"
                      << _pos.cur().line << "."
                      << _pos.cur().col << '\n';
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

    Type passed_type = parse_type(_pos, _settings);

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
    locals.push_back({{passed_name, passed_type}});

    // Push frame to be popped
    locals.push_back({});

    ASTNodes::Case out;
    out.c_name = case_name;

    Node first_child(Node::NONE);
    first_child.token = passed_name;
    first_child.type = passed_type;
    out.children = {first_child};

    // Statement
    out.children.push_back(parse_statement(_pos, _settings));

    // Pop frame, calling destructors
    out = pop_frame(out, _settings);

    // Pop from locals stack WITHOUT CALLING DESTRUCTOR ON
    // CAPTURE
    locals.pop_back();

    return out;
  } else if (_pos.cur() == "else") {
    // Statement
    _pos.next();
    Node out(Node::NONE);
    out.children = {parse_statement(_pos, _settings)};
    return out;
  } else {
    throw std::runtime_error(
        "Error within match statement: Expected 'case' or "
        "'else', but saw '" +
        _pos.cur().text + "'");
  }
}

/// Parse a function call
Node Parser::parse_function_call(TokenStream &_pos,
                                 Settings &_settings) {
  debug_print();
  if (_settings.debug) {
    _settings.ostream << __FUNCTION__ << " at "
                      << _pos.cur().file.string() << ":"
                      << _pos.cur().line << "."
                      << _pos.cur().col << '\n';
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
    Node out(Node::RAW_C_FMT);
    out.type = Type({"uint"});
    out.c_name =
        "sizeof(" + parse_type(_pos, _settings).c_repr() + ")";

    _pos.next();
    if (_pos.cur() != ")") {
      throw std::runtime_error(
          "Invalid size! macro: Expected ')', but saw '" +
          _pos.cur().text + "'");
    }

    return out;
  }

  // Function call
  Node out(Node::CALL);
  out.token = _pos.cur();

  _pos.next();
  if (_pos.cur() != "(") {
    throw std::runtime_error("Expected function call");
  }
  _pos.next();

  std::vector<Type> args;
  while (_pos.cur() != ")") {
    if (_pos.cur() != ",") {
      out.children.push_back(parse_object(_pos, _settings));
      args.push_back(out.children.back().type.value());
    }
    _pos.next();
  }

  //////////////////////////////////////////////////////////////
  // Special cases here

  // Array access via the 'Get' operator
  if (out.token.value() == "Get" && out.children.size() == 2 &&
      (out.children.back().type->cast_match(Type({"u128"})) ||
       out.children.back().type->cast_match(Type({"i128"}))) &&
      (out.children.front().type->nodes.front().type ==
           Type::TypeNode::SIZED_ARRAY ||
       out.children.front().type->nodes.front().type ==
           Type::TypeNode::UNSIZED_ARRAY)) {
    // Only resolvable at reconstruction-time
    out.node_type = Node::ARR;
    out.type = out.children.front().type;
    out.type->nodes.pop_front();
    return out;
  }

  // alloc!
  else if (out.token.has_value() &&
           out.token.value() == "alloc!") {
    // 1-arg
    if (out.children.size() == 1) {
      if (!out.children.front().type.has_value() ||
          out.children.front().type->nodes.empty() ||
          out.children.front().type->nodes.front().type !=
              Type::TypeNode::POINTER) {
        throw std::runtime_error(
            "Expected pointer type for alloc!(into), instead "
            "saw '" +
            out.children.front().type->oak_repr() + "'");
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
    else if (out.children.size() == 2) {
      if (out.children.front().type->nodes.empty() ||
          out.children.front().type->nodes.front().type !=
              Type::TypeNode::UNSIZED_ARRAY) {
        throw std::runtime_error(
            "Expected unsized array type for alloc!(into, "
            "size), instead saw '" +
            out.children.front().type->oak_repr() + "'");
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
  else if (out.token.has_value() &&
           out.token.value() == "free!") {
    if (out.children.size() != 1 ||
        out.children.front().type->nodes.empty() ||
        (out.children.front().type->nodes.front().type !=
             Type::TypeNode::POINTER &&
         out.children.front().type->nodes.front().type !=
             Type::TypeNode::UNSIZED_ARRAY)) {
      throw std::runtime_error(
          "Expected pointer or unsized array type for "
          "free!(to_free), instead saw '" +
          out.children.front().type->oak_repr() + "'");
    }

    out.node_type = Node::RAW_C_FMT;
    out.type = Type({"void"});
    out.c_name = "free((void *)(%))";
    return out;
  }

  // New on pointer or unsized array types
  else if (out.token.has_value() &&
           out.token.value() == "New" &&
           out.children.size() == 1 &&
           (out.children.front().type->nodes.front().type ==
                Type::TypeNode::POINTER ||
            out.children.front().type->nodes.front().type ==
                Type::TypeNode::UNSIZED_ARRAY)) {
    out.node_type = Node::RAW_C_FMT;
    out.type = Type({"void"});
    out.c_name = "% = 0;";
    return out;
  }

  // New on atomic types
  else if (out.token.has_value() &&
           out.token.value() == "New" &&
           out.children.size() == 1 &&
           Type::is_built_in_type(
               out.children.front().type.value())) {
    out.node_type = Node::RAW_C_FMT;
    out.type = Type({"void"});
    out.c_name = "% = 0;";
    return out;
  }

  // // New on sized array types
  // else if (out.token.has_value() &&
  //          out.token.value() == "New" &&
  //          out.children.size() == 1 &&
  //          out.children.front().type->nodes.front().type ==
  //              Type::TypeNode::SIZED_ARRAY) {
  //   Node new_out(Node::STMT);
  //   Lexer l;
  //   uint64_t line, col;
  //   for (uint i = 0; i < out.children.front()
  //                            .type->nodes.front()
  //                            .sized_array_size;
  //        ++i) {
  //     line = out.token->line;
  //     col = out.token->col;
  //     auto lexed = l.lex(
  //         "New(Get(" + out.children.front().c_name.value() +
  //             ", " + std::to_string(i) + "uint));",
  //         out.token->file, line, col);
  //     new_out.children.push_back(
  //         parse_function_call(lexed, _settings));
  //     return new_out;
  //   }
  // }

  // Del on atomic types
  else if (out.token.has_value() &&
           out.token.value() == "Del" &&
           out.children.size() == 1 &&
           Type::is_built_in_type(
               out.children.front().type.value())) {
    out.node_type = Node::NONE;
    return out;
  }

  // Del on pointer or unsized array types
  else if (out.token.has_value() &&
           out.token.value() == "Del" &&
           out.children.size() == 1 &&
           out.children.front().type.has_value() &&
           (out.children.front().type->nodes.front().type ==
                Type::TypeNode::POINTER ||
            out.children.front().type->nodes.front().type ==
                Type::TypeNode::UNSIZED_ARRAY)) {
    out.node_type = Node::NONE;
    return out;
  }

  // // Del on sized array
  // else if (out.token.has_value() &&
  //          out.token.value() == "Del" &&
  //          out.children.size() == 1 &&
  //          out.children.front().type->nodes.front().type ==
  //              Type::TypeNode::SIZED_ARRAY) {
  //   Node new_out(Node::STMT);
  //   Lexer l;
  //   uint64_t line, col;
  //   for (uint i = 0; i < out.children.front()
  //                            .type->nodes.front()
  //                            .sized_array_size;
  //        ++i) {
  //     line = out.token->line;
  //     col = out.token->col;
  //     auto lexed = l.lex(
  //         "Del(Get(" + out.children.front().c_name.value() +
  //             ", " + std::to_string(i) + "uint));",
  //         out.token->file, line, col);
  //     new_out.children.push_back(
  //         parse_function_call(lexed, _settings));
  //     return new_out;
  //   }
  // }

  // Pointer copy
  else if (out.token.has_value() &&
           out.token.value() == "Copy" &&
           out.children.size() == 2 &&
           out.children.back().type->nodes.front().type ==
               Type::TypeNode::POINTER &&
           out.children.front().type->cast_match(
               *out.children.back().type)) {
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

          if (out.children.size() != needed_args.size()) {
            throw std::runtime_error(
                "Expected " +
                std::to_string(needed_args.size()) +
                " args in fn pointer call, but saw " +
                std::to_string(out.children.size()));
          }

          for (uint i = 0; i < needed_args.size(); ++i) {
            if (!needed_args[i].second.exact_match(
                    out.children[i].type.value())) {
              throw std::runtime_error(
                  "Expected type '" +
                  needed_args[i].second.oak_repr() +
                  "' for arg " + std::to_string(i) +
                  " of fn pointer call, but saw type '" +
                  out.children[i].type->oak_repr() + "'");
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

  // Return type only
  FnInfo f;
  std::vector<int> derefs;
  out.type = resolve_fn_call(out.token.value().text, args, f,
                             derefs, _settings);

  // Build c-name
  out.c_name = f.t.mangle(f.name);

  // Modify children as needed
  if (!derefs.empty()) {
    for (uint i = 0; i < out.children.size(); ++i) {
      if (derefs[i] == -1) {
        Node new_child(Node::RAW_C_FMT);
        new_child.children = {out.children[i]};
        new_child.c_name = "&%";
        out.children[i] = new_child;
      } else if (derefs[i] > 0) {
        Node new_child(Node::RAW_C_FMT);
        new_child.children = {out.children[i]};

        new_child.c_name = "";
        new_child.c_name->reserve(derefs[i]);
        for (int j = 0; j < derefs[i]; ++j) {
          new_child.c_name->push_back('*');
        }
        new_child.c_name->push_back('%');

        out.children[i] = new_child;
      }
    }
  }

  return out;
}

/// Resolve the given variable
Type Parser::resolve_variable(const Lexer::Token &_name,
                              std::string &_new_name) {
  debug_print();

  if (!locals.empty()) {
    for (auto frame = locals.rbegin(); frame != locals.rend();
         ++frame) {
      if (frame->contains(_name)) {
        _new_name = _name.text;
        return frame->at(_name);
      }
    }
  }

  // Fn ptrs
  if (functions.contains(_name.text)) {
    if (functions.at(_name.text).size() != 1) {
      throw std::runtime_error(
          "Cannot make pointer to overridden function '" +
          _name.text + "'.");
    }

    const auto fn_type = functions.at(_name.text).front().t;
    _new_name = fn_type.mangle(_name.text);
    return fn_type.ref();
  }

  throw std::runtime_error("Variable '" + _name.text +
                           "' does not exist.");
}

Node Parser::parse_object(TokenStream &_pos,
                          Settings &_settings) {
  debug_print();
  if (_settings.debug) {
    _settings.ostream << __FUNCTION__ << " at "
                      << _pos.cur().file.string() << ":"
                      << _pos.cur().line << "."
                      << _pos.cur().col << '\n';
  }

  /*
  object = name | function_call | object . name ;
  */

  // Open parenthesis immediately: No-capture lambda
  if (_pos.cur() == "(") {
    // Create name
    uint lambda_counter = 1;
    while (functions.contains("__oak_lambda_" +
                              std::to_string(lambda_counter))) {
      ++lambda_counter;
    }
    const std::string lambda_name =
        "__oak_lambda_" + std::to_string(lambda_counter);

    // Parse function
    auto stack = locals;
    locals.clear();
    parse_function({lambda_name}, _pos, _settings);
    locals = stack;

    // Return a fn pointer to that lambda
    Node out(Node::OBJECT);
    out.token = Lexer::Token(_pos.cur(), lambda_name);
    out.c_name = "";
    out.type =
        resolve_variable(out.token.value(), out.c_name.value());
    out.token.value().text = out.c_name.value();
    return out;
  }

  // Open parenthesis follows: Function call
  else if (_pos.peek(1).text == "(") {
    return parse_function_call(_pos, _settings);
  }

  auto cur = _pos.cur();
  const auto literal_type = Lexer::get_literal_type(cur);
  if (literal_type.has_value()) {
    // Literal
    Node out(Node::OBJECT);
    out.c_name = cur;
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

    std::string name;
    Type t = resolve_variable(_pos.cur(), name);

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

      const auto info = global_types.at(t.struct_name());

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

    Node out(Node::OBJECT);
    out.c_name = name;
    out.type = t;
    return out;
  }
}

/// Returns whether the given substitutions would cause the
/// `provides` list to match the given list
bool Parser::TemplateInfo::does_provide(
    const std::list<std::list<std::string>> &_substitutions,
    const std::list<std::string> &_desired) const {
  debug_print();

  std::list<std::string> tokenized;
  for (const auto &i : provides) {
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
std::list<std::string> Parser::TemplateInfo::replace(
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

bool Parser::TemplateInfo::attempt_instantiation(
    Parser &_p,
    const std::list<std::list<std::string>> &_substitutions,
    Settings &_settings) {
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
    _p.parse_global(replaced_validation_block, _settings);
  } catch (...) {
    _p = backup;
    return false;
  }

  // Replace the instantiation block
  auto replaced_instantiation_block = Lexer::tokify(
      replace(instantiate, generics, _substitutions), path,
      line, col);

  // Struct name fix
  if (provides.size() == 1 && provides.front() == "struct") {
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

  if (_settings.debug) {
    _settings.ostream << "Parsing template block:\n```oak\n";
    uint counter = 0;
    for (const auto &tok : replaced_instantiation_block) {
      _settings.ostream << tok.text << ' ';

      counter += tok.text.size() + 1;
      if (counter >= 32) {
        _settings.ostream << '\n';
        counter = 0;
      }
    }
    _settings.ostream << "\n```\n";
  }

  // Run instantiation block
  try {
    _p.parse_global(replaced_instantiation_block, _settings);
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

/**
 * @brief Given some information about a fn call, construct a
 * printable string.
 * @param _name The name of the function
 * @param _args The types of the arguments
 * @returns A string representing the fn call
 */
std::string fn_call_str(const std::string &_name,
                        const std::vector<Type> &_args) {
  debug_print();
  std::string call_text = _name + "(";
  bool first = true;
  for (const auto &arg_type : _args) {
    if (first) {
      first = false;
    } else {
      call_text += ", ";
    }
    call_text += "_: " + arg_type.oak_repr();
  }
  call_text += ")";
  return call_text;
}

// Resolves a function call through any means necessary. If
// it cannot be resolved, an error is thrown.
Type Parser::resolve_fn_call(const std::string &_name,
                             const std::vector<Type> &_args,
                             FnInfo &_into,
                             std::vector<int> &_derefs,
                             Settings &_settings,
                             const bool &_allow_template) {
  std::vector<FnInfo> candidates;
  std::list<uint> exact_matches, cast_matches, ref_matches;
  std::list<std::vector<int>> ref_match_deref_counts;

  debug_print();
  try {
    _derefs.clear();
    for (const auto &_ : _args) {
      _derefs.push_back(0);
    }

    // Attempt existing instances
    candidates.assign(functions[_name].begin(),
                      functions[_name].end());
    for (uint i = 0; i < candidates.size(); ++i) {
      bool exact = true, ref = true, cast = true;
      const auto instance_args = candidates.at(i).t.fn_args();

      if (instance_args.size() != _args.size()) {
        continue;
      }

      std::vector<int> num_derefs;
      num_derefs.reserve(instance_args.size());
      for (uint i = 0; i < instance_args.size(); ++i) {
        if (exact &&
            !_args[i].exact_match(instance_args[i].second)) {
          exact = false;
        }

        int num_arg_derefs = 0;
        if (ref && !_args[i].ref_match(instance_args[i].second,
                                       num_arg_derefs)) {
          ref = false;
        }
        num_derefs.push_back(num_arg_derefs);

        if (cast &&
            !_args[i].cast_match(instance_args[i].second)) {
          cast = false;
        }
      }

      if (exact) {
        exact_matches.push_back(i);
      } else if (ref) {
        ref_matches.push_back(i);
        ref_match_deref_counts.push_back(num_derefs);
      } else if (cast) {
        cast_matches.push_back(i);
      }
    }

    if (exact_matches.empty()) {
      if (ref_matches.empty()) {
        if (!cast_matches.empty()) {
          // Use casting matches
          if (cast_matches.size() != 1) {
            throw std::runtime_error(
                "Multiple castable matches were "
                "found for function call '" +
                fn_call_str(_name, _args) + "'");
          } else {
            _into = candidates.at(cast_matches.front());
            return candidates.at(cast_matches.front())
                .t.fn_return_type();
          }
        }
      } else {
        // Use ref matches
        if (ref_matches.size() != 1) {
          throw std::runtime_error(
              "Multiple reference matches were "
              "found for function call '" +
              fn_call_str(_name, _args) + "'");
        } else {
          _into = candidates.at(ref_matches.front());
          _derefs = ref_match_deref_counts.front();
          return candidates.at(ref_matches.front())
              .t.fn_return_type();
        }
      }
    } else {
      // Use exact matches

      // Count number of signature-only matches
      uint num_sigs = 0;
      for (const auto &item : exact_matches) {
        if (candidates.at(item).tags.contains("casual") &&
            candidates.at(item).tags.at("casual") == "true") {
          ++num_sigs;
        }
      }

      if (!(num_sigs == exact_matches.size() ||
            num_sigs + 1 == exact_matches.size())) {
        throw std::runtime_error("Multiple exact matches were "
                                 "found for function call '" +
                                 fn_call_str(_name, _args) +
                                 "'");
      } else {
        _into = candidates.at(exact_matches.front());
        return candidates.at(exact_matches.front())
            .t.fn_return_type();
      }
    }

    if (_allow_template) {
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
          for (const auto &tok : l.lex(arg.oak_repr(), "NULL",
                                       junk_line, junk_col)) {
            signature.push_back(tok.text);
          }
        }
        signature.push_back(")");
        // Note: No return type!

        // If there exist some substitutions such that some
        // template exactly matches the signature, do that
        const std::list<
            std::pair<std::list<std::list<std::string>>, uint>>
            ts = find_substitutions(_name, signature);
        for (const auto &p : ts) {
          if (templates.at(_name)
                  .at(p.second)
                  .attempt_instantiation(*this, p.first,
                                         _settings)) {
            // Don't allow templates this time!
            return resolve_fn_call(_name, _args, _into, _derefs,
                                   _settings, false);
          }
        }
      } catch (std::runtime_error &_e) {
        throw std::runtime_error(
            "Error during template checking "
            "requested by function call '" +
            fn_call_str(_name, _args) + "':\n" + _e.what());
      } catch (...) {
        throw std::runtime_error(
            "Unknown error during template checking "
            "requested by function call '" +
            fn_call_str(_name, _args) + "'");
      }
    }

    // Throw error if it couldn't be resolved
    throw std::runtime_error("No existing candidate nor "
                             "providing template could be "
                             "found for function call '" +
                             fn_call_str(_name, _args) + "'");
  } catch (std::runtime_error &) {
    _settings.ostream << "Candidates:\n";
    for (uint i = 0; i < candidates.size(); ++i) {
      _settings.ostream << candidates.at(i).tags["file"] << ":"
                        << candidates.at(i).tags["line"] << "> "
                        << candidates.at(i).t.oak_repr(_name);

      if (std::find(exact_matches.begin(), exact_matches.end(),
                    i) != exact_matches.end()) {
        _settings.ostream << " exact";
      }
      if (std::find(cast_matches.begin(), cast_matches.end(),
                    i) != cast_matches.end()) {
        _settings.ostream << " cast-matchable";
      }
      if (std::find(ref_matches.begin(), ref_matches.end(),
                    i) != ref_matches.end()) {
        _settings.ostream << " ref-matchable";
      }

      _settings.ostream << '\n';
    }
    throw;
  }
}

std::list<std::pair<std::list<std::list<std::string>>, uint>>
Parser::find_substitutions(
    const std::string &_name,
    const std::list<std::string> &_signature) const {
  debug_print();

  std::list<std::pair<std::list<std::list<std::string>>, uint>>
      out;

  if (templates.contains(_name)) {
    for (uint i = 0; i < templates.at(_name).size(); ++i) {
      // Check instance

      // For as long as we haven't finished the template
      // If literal on both sides that matches, advance
      // Else if template has generic, log what that template
      // needs to be
      const auto templ_info = templates.at(_name).at(i);

      std::map<std::string, uint> generic_indices;
      std::vector<std::list<std::string>> substitutions;
      for (const auto &item : templ_info.generics) {
        generic_indices[item] = substitutions.size();
        substitutions.push_back({});
      }

      auto desired_it = _signature.begin();
      auto templ_it = templ_info.provides.begin();
      bool do_add = true;

      while (desired_it != _signature.end() &&
             templ_it != templ_info.provides.end()) {
        if (generic_indices.contains(*templ_it)) {
          do {
            substitutions.at(generic_indices.at(*templ_it))
                .push_back(*desired_it);
            ++desired_it;
          } while (desired_it != _signature.end() &&
                   *desired_it != *std::next(templ_it));
          ++templ_it;
        } else if (*desired_it == *templ_it) {
          ++desired_it;
          ++templ_it;
        } else {
          do_add = false;
          break;
        }
      }

      if (do_add) {
        std::list<std::list<std::string>> to_append;
        for (uint j = 0; j < substitutions.size(); ++j) {
          to_append.push_back(substitutions.at(j));
        }
        out.push_back({to_append, i});
      }
    }
  }

  return out;
}

// Throws an error on invalid type (EG undefined struct
// name)
void Parser::validate_type(const Type &_t) const {
  debug_print();
  for (const auto &node : _t.nodes) {
    if (node.type == Type::TypeNode::LITERAL) {
      if (Type::is_built_in_type(node.literal_name)) {
        continue;
      }

      if (!global_types.contains(node.literal_name)) {
        throw std::runtime_error("Atomic type '" +
                                 node.literal_name +
                                 "' does not exist.");
      }
    }
  }
}

// Fetch a symbol
std::optional<std::variant<Parser::StructInfo, Parser::EnumInfo,
                           std::list<Parser::FnInfo>>>
Parser::fetch_symbol(const std::string &_name) const noexcept {
  debug_print();
  std::optional<
      std::variant<StructInfo, EnumInfo, std::list<FnInfo>>>
      out;
  if (global_types.contains(_name)) {
    // Some syntactic fluff on the "variant" type
    if (std::holds_alternative<StructInfo>(
            global_types.at(_name))) {
      out = std::get<StructInfo>(global_types.at(_name));
    } else {
      out = std::get<EnumInfo>(global_types.at(_name));
    }
  } else if (functions.contains(_name)) {
    out = functions.at(_name);
  }
  return out;
}
