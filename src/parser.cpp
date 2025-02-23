/**
 * @file parser.cpp
 */

#include "parser.hpp"
#include "debug.hpp"
#include "lexer.hpp"
#include <functional>
#include <linux/limits.h>
#include <stdexcept>
#include <string>
#include <variant>

/**
 * @brief Safely increment an iterator. If it is the end before
 * OR AFTER incrementation, throws an error.
 * @param _it The iterator to increment
 * @param _end The end position from the iterand
 */
void incr(std::list<Lexer::Token>::const_iterator &_it,
          const std::list<Lexer::Token>::const_iterator &_end) {
  if (_it == _end) {
    throw std::runtime_error(
        "Cannot increment iterator past end of iterand.");
  }

  ++_it;
  if (_it == _end) {
    throw std::runtime_error("Attempted to move past EOF.");
  }
}

// Parse a global scope
void Parser::parse_global(
    const std::list<Lexer::Token> &_file_contents) {
  debug_print();
  // Iterate and delegate. No macros remain.
  auto pos = _file_contents.begin();
  const auto end = _file_contents.end();

  while (pos != _file_contents.end()) {
    try {
      if (*pos == "let") {
        incr(pos, end);
        std::set<std::string> names = {*pos};
        incr(pos, end);
        if (*pos == ":") {
          // Struct, enum, or invalid global definition
          incr(pos, end);
          if (*pos == "struct") {
            incr(pos, end);
            parse_struct(names, pos, end);
            ++pos; // Don't use incr here
          } else if (*pos == "enum") {
            incr(pos, end);
            parse_enum(names, pos, end);
            ++pos; // Don't use incr here
          } else {
            throw std::runtime_error(
                "Global scope 'let' error: Expected 'struct' "
                "or "
                "'enum', saw '" +
                pos->text + "'");
          }
        } else if (*pos == "(") {
          // Function
          parse_function(names, pos, end);
          ++pos;
        } else {
          throw std::runtime_error(
              "Global scope 'let' error: "
              "Expected '(' or ':', saw '" +
              pos->text + "'");
        }
      } else if (*pos == ";") {
        ++pos;
      } else {
        throw std::runtime_error(
            "Global scope parse error: Unexpected token '" +
            pos->text + "'");
      }
    } catch (std::runtime_error &e) {
      if (pos == _file_contents.end()) {
        throw e;
      }
      throw std::runtime_error("At " + pos->file.string() +
                               ":" + std::to_string(pos->line) +
                               "." + std::to_string(pos->col) +
                               "\n" + e.what());
    } catch (...) {
      if (pos == _file_contents.end()) {
        throw;
      }
      throw std::runtime_error("At " + pos->file.string() +
                               ":" + std::to_string(pos->line) +
                               "." + std::to_string(pos->col) +
                               "\nUnknown error");
    }
  }
}

// Resets the state of the translation unit
void Parser::reset() {
  debug_print();
}

// Constructs the equivalent C program at the given path
void Parser::reconstruct(std::ostream &_where) const noexcept {
  debug_print();

  // Include std header
  _where << "#include \"oak/std/std_oak_header.h\"\n";

  // Struct and enum signatures
  for (const auto &g : globals) {
    if (std::holds_alternative<StructInfo>(g.second)) {
      _where << "struct " << g.first << ";\n";
    } else {
      _where << "enum " << g.first << ";\n";
    }
  }

  // Function signatures
  for (const auto &p : functions) {
    const auto name = p.first;
    for (const auto &info : p.second) {
      _where << info.t.c_repr(name, name == "main") << ";\n";
    }
  }

  // Struct and enum definitions
  for (const auto &g : globals) {
    if (std::holds_alternative<StructInfo>(g.second)) {
      const auto info = std::get<StructInfo>(g.second);
      _where << "struct " << g.first << " {\n";
      for (const auto &item : info.member_order) {
        info.members.at(item).c_repr(item);
        _where << ";\n";
      }
      _where << "};\n";
    } else {
      const EnumInfo info = std::get<EnumInfo>(g.second);
      _where << "enum " << g.first << "{enum{\n";
      for (const auto &item : info.option_order) {
        _where << g.first << "_OPT_" << item << ",";
      }
      _where << "}__info;union{\n";
      for (const auto &item : info.option_order) {
        info.options.at(item).c_repr(item);
        _where << ";";
      }
      _where << "}__data;};\n";
    }
  }

  const std::function<void(const Node &)> reconstruct_node =
      [&](const Node &stmt) -> void {
    bool first;
    switch (stmt.node_type) {
    case Node::IF:
      _where << "if (";
      reconstruct_node(stmt.children.front());
      _where << ")";
      reconstruct_node(*std::next(stmt.children.begin()));
      if (stmt.children.size() == 3) {
        _where << "else ";
        reconstruct_node(*std::next(stmt.children.begin(), 2));
      }
      break;
    case Node::WHILE:
      _where << "while (";
      reconstruct_node(stmt.children.front());
      _where << ")";
      reconstruct_node(*std::next(stmt.children.begin()));
      break;
    case Node::MATCH:
      throw std::runtime_error(__FUNCTION__);
      break;
    case Node::CASE:
      throw std::runtime_error(__FUNCTION__);
      break;
    case Node::OBJECT:
      // Literal or variable
      _where << stmt.c_name.value();
      break;
    case Node::CALL:
      _where << stmt.c_name.value() << "(";
      first = true;
      for (const auto &item : stmt.children) {
        if (first) {
          first = false;
        } else {
          _where << ", ";
        }
        reconstruct_node(item);
      }
      _where << ")";
      break;
    case Node::NONE:
      _where << ";\n";
      break;
    case Node::STMT:
      if (*stmt.token == "return") {
        // Return statement
        _where << "return ";
        if (!stmt.children.empty()) {
          reconstruct_node(stmt.children.front());
        }
        _where << ";\n";
      } else {
        // Scope
        _where << "{\n";
        for (const auto &child : stmt.children) {
          reconstruct_node(child);
          _where << ";\n";
        }
        _where << "}\n";
      }
      break;
    }
  };

  // Function definitions
  for (const auto &p : functions) {
    const auto name = p.first;
    for (const auto &info : p.second) {
      if (info.tags.contains("signature")) {
        continue;
      }
      _where << info.t.c_repr(name, name == "main");
      reconstruct_node(info.n);
    }
  }
}

/// Dump to the given stream
void Parser::dump(std::ostream &_where) const noexcept {
  debug_print();
  throw std::runtime_error(__FUNCTION__);
}

// Parse a single function declaration
// Assumes we have just seen "let NAME (" and are pointing to
// the next token.
void Parser::parse_function(
    const std::set<std::string> &_names,
    std::list<Lexer::Token>::const_iterator &_cur_pos,
    const std::list<Lexer::Token>::const_iterator &_end) {
  debug_print();
  // Finish parsing type
  Type t = parse_type(_cur_pos, _end);
  incr(_cur_pos, _end);

  // Either signature or implementation
  FnInfo to_add;
  to_add.t = t;

  if (*_cur_pos == ";") {
    // Signature
    to_add.tags = {{"signature", "true"}};
  } else {
    // Implementation
    to_add.n = parse_statement(_cur_pos, _end);
  }

  for (const auto &name : _names) {
    functions[name].push_back(to_add);
  }
}

// Parses a struct/enum's guts
std::list<std::pair<std::string, Type>> Parser::parse_members(
    std::list<Lexer::Token>::const_iterator &_cur_pos,
    const std::list<Lexer::Token>::const_iterator &_end) {
  debug_print();
  // Where to write output
  std::list<std::pair<std::string, Type>> out;

  // For keeping track of used names
  std::set<std::string> all_names;

  incr(_cur_pos, _end);

  while (*_cur_pos != "}") {
    // One or more comma-separated names
    std::list<std::string> current_names;

    // Mandatory name
    current_names.push_back(*_cur_pos);
    incr(_cur_pos, _end);

    // Optional names
    while (*_cur_pos == ",") {
      incr(_cur_pos, _end);
      current_names.push_back(*_cur_pos);
      incr(_cur_pos, _end);
    }

    // Colon
    if (*_cur_pos != ":") {
      throw std::runtime_error("Invalid struct/enum body: "
                               "Expected ':' or '.' but saw '" +
                               _cur_pos->text + "'");
    }
    incr(_cur_pos, _end);

    // Type
    Type t = parse_type(_cur_pos, _end);
    incr(_cur_pos, _end);

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
    while (*_cur_pos == ",") {
      incr(_cur_pos, _end);
    }
  }

  // Leave pointing to ending brace
  return out;
}

// Return the type spec at the specified location
Type Parser::parse_type(
    std::list<Lexer::Token>::const_iterator &_cur_pos,
    const std::list<Lexer::Token>::const_iterator &_end) {
  debug_print();
  Type out;
  out.process_next(*_cur_pos);
  while (!out.valid()) {
    incr(_cur_pos, _end);
    out.process_next(*_cur_pos);
  }
  return out;
}

// Parse a single struct declaration
// Assumes we have just seen "let NAME : struct" and are
// pointing to the next token.
void Parser::parse_struct(
    const std::set<std::string> &_names,
    std::list<Lexer::Token>::const_iterator &_cur_pos,
    const std::list<Lexer::Token>::const_iterator &_end) {
  debug_print();

  // The struct info to populate
  StructInfo to_add;

  to_add.tags["file"] = _cur_pos->file;
  to_add.tags["line"] = _cur_pos->line;
  to_add.tags["col"] = _cur_pos->col;

  // Casual def
  if (*_cur_pos == ";") {
    to_add.tags["casual"] = "true";
  }

  // Declaration
  else if (*_cur_pos == "{") {
    const auto members = parse_members(_cur_pos, _end);
  }

  // Invalid
  else {
    throw std::runtime_error("Invalid struct declaration: "
                             "Expected ';' or '{' but saw '" +
                             _cur_pos->text + "'");
  }

  // Add these entries
  for (const auto &name : _names) {
    if (globals.contains(name) && // Disallow overwriting
        !(std::holds_alternative<StructInfo>(
              globals.at(name)) && // Except for
                                   // structs
          std::get<StructInfo>(globals.at(name))
                  .tags["casual"] ==
              "true") // That are only casually defined
    ) {

      // Construct the existing type as a str
      std::string existing_type_str;

      if (std::holds_alternative<StructInfo>(
              globals.at(name))) {
        existing_type_str = "struct";
      } else if (std::holds_alternative<EnumInfo>(
                     globals.at(name))) {
        existing_type_str = "enum";
      }

      // Throw appropriate error message
      throw std::runtime_error("Cannot replace " +
                               existing_type_str + " '" + name +
                               "' w/ struct of same name");
    }
    globals[name] = to_add;
  }
}

// Parse a single enum declaration
// Assumes we have just seen "let NAME : enum" and are
// pointing to the next token.
void Parser::parse_enum(
    const std::set<std::string> &_names,
    std::list<Lexer::Token>::const_iterator &_cur_pos,
    const std::list<Lexer::Token>::const_iterator &_end) {
  debug_print();

  // The enum info to populate
  EnumInfo to_add;

  to_add.tags["file"] = _cur_pos->file;
  to_add.tags["line"] = _cur_pos->line;
  to_add.tags["col"] = _cur_pos->col;

  // Casual def
  if (*_cur_pos == ";") {
    to_add.tags["casual"] = "true";
  }

  // Declaration
  else if (*_cur_pos == "{") {
    const auto members = parse_members(_cur_pos, _end);
  }

  // Invalid
  else {
    throw std::runtime_error("Invalid enum declaration: "
                             "Expected ';' or '{' but saw '" +
                             _cur_pos->text + "'");
  }

  // Add these entries
  for (const auto &name : _names) {
    if (globals.contains(name) && // Disallow overwriting
        !(std::holds_alternative<EnumInfo>(
              globals.at(name)) && // Except for
                                   // enums
          std::get<EnumInfo>(globals.at(name)).tags["casual"] ==
              "true") // That are only casually defined
    ) {

      // Construct the existing type as a str
      std::string existing_type_str;

      if (std::holds_alternative<StructInfo>(
              globals.at(name))) {
        existing_type_str = "struct";
      } else if (std::holds_alternative<EnumInfo>(
                     globals.at(name))) {
        existing_type_str = "enum";
      }

      // Throw appropriate error message
      throw std::runtime_error("Cannot replace " +
                               existing_type_str + " '" + name +
                               "' w/ enum of same name");
    }
    globals[name] = to_add;
  }
}

// Assumes we are pointing to the first token in the statement
Node Parser::parse_statement(
    std::list<Lexer::Token>::const_iterator &_cur_pos,
    const std::list<Lexer::Token>::const_iterator &_end) {
  debug_print();
  // A statement can be a function call, a (possibly compound)
  // if statement, a match statement, nothing, a variable
  // declaration, or a while statement

  if (*_cur_pos == ";") {
    // Unit statement
    Node out;
    out.node_type = Node::NONE;
    return out;
  } else if (*_cur_pos == "let") {
    // Variable declaration
    // Collect names
    std::set<std::string> names;

    do {
      // Fluff
      incr(_cur_pos, _end);

      // Name
      names.insert(*_cur_pos);
      incr(_cur_pos, _end);
    } while (*_cur_pos == ",");

    if (*_cur_pos != ":") {
      throw std::runtime_error(
          "Expected ':' after 'let' statement. Instead saw '" +
          _cur_pos->text + "'");
    }
    incr(_cur_pos, _end);

    // Get type
    Type t = parse_type(_cur_pos, _end);
    validate_type(t);

    // Add all to symbol table
    for (const auto &name : names) {
      locals.back()[name] = t;
    }

    Node out;
    out.node_type = Node::NONE;
    return out;
  } else if (*_cur_pos == "{") {
    // Scope
    Node out;
    out.node_type = Node::STMT;

    // Add a scope frame to the scope stack
    locals.push_back({});

    incr(_cur_pos, _end);
    while (*_cur_pos != "}") {
      out.children.push_back(parse_statement(_cur_pos, _end));
      incr(_cur_pos, _end);
    }

    // Remove that scope frame
    locals.pop_back();

    return out;
  } else if (*_cur_pos == "if") {
    // If statement
    incr(_cur_pos, _end);
    if (*_cur_pos != "(") {
      throw std::runtime_error(
          "Missing parenthesis in 'if' statement.");
    }

    // Condition is a single boolean object
    Node condition = parse_object(_cur_pos, _end);
    if (!condition.type.value().cast_match(Type({"bool"}))) {
      throw std::runtime_error("Statement condition type '" +
                               condition.type->oak_repr() +
                               "' is not castable to bool.");
    }

    // Closing parenthesis
    incr(_cur_pos, _end);
    if (*_cur_pos != ")") {
      throw std::runtime_error(
          "Missing ending parenthesis in 'if' statement.");
    }
    incr(_cur_pos, _end);

    // Body
    Node body = parse_statement(_cur_pos, _end);

    Node out;
    out.node_type = Node::IF;
    out.children = {condition, body};

    // Optional else clause
    ++_cur_pos; // Can't use incr here!
    if (_cur_pos != _end && *_cur_pos == "else") {
      // Else clause
      incr(_cur_pos, _end);
      out.children.push_back(parse_statement(_cur_pos, _end));
    } else {
      --_cur_pos;
    }

    return out;
  } else if (*_cur_pos == "while") {
    // While loop
    incr(_cur_pos, _end);
    if (*_cur_pos != "(") {
      throw std::runtime_error(
          "Missing parenthesis in 'while' statement.");
    }

    // Condition is a single boolean object
    Node condition = parse_object(_cur_pos, _end);
    if (!condition.type.value().cast_match(Type({"bool"}))) {
      throw std::runtime_error("Statement condition type '" +
                               condition.type->oak_repr() +
                               "' is not castable to bool.");
    }

    // Closing parenthesis
    incr(_cur_pos, _end);
    if (*_cur_pos != ")") {
      throw std::runtime_error(
          "Missing ending parenthesis in 'while' statement.");
    }
    incr(_cur_pos, _end);

    // Body
    Node body = parse_statement(_cur_pos, _end);

    Node out;
    out.node_type = Node::WHILE;
    out.children = {condition, body};
    return out;
  } else if (*_cur_pos == "match") {
    // Match statement
    /*
    match (NAME) {
        case (a: foo) {}
        case (b: fizz) {}
        else {}
    }
    */
    throw std::runtime_error(
        "Match statements are unimplemented");
  } else if (*_cur_pos == "return") {
    // Return statement
    Node out;
    out.token = *_cur_pos;
    incr(_cur_pos, _end);
    out.node_type = Node::STMT;
    out.children.push_back(parse_object(_cur_pos, _end));
    return out;
  } else {
    // Function call
    return parse_function_call(_cur_pos, _end);
  }
}

/// Parse a function call
Node Parser::parse_function_call(
    std::list<Lexer::Token>::const_iterator &_cur_pos,
    const std::list<Lexer::Token>::const_iterator &_end) {
  debug_print();
  // Function call
  Node out;
  out.token = *_cur_pos;
  out.node_type = Node::CALL;

  incr(_cur_pos, _end);
  if (*_cur_pos != "(") {
    throw std::runtime_error("Expected function call!");
  }
  incr(_cur_pos, _end);

  std::vector<Type> args;
  while (*_cur_pos != ")") {
    if (*_cur_pos != ",") {
      out.children.push_back(parse_object(_cur_pos, _end));
      args.push_back(out.children.back().type.value());
    }
    incr(_cur_pos, _end);
  }
  incr(_cur_pos, _end);
  if (*_cur_pos != ";") {
    throw std::runtime_error(
        "Missing semicolon after function call.");
  }

  // Return type only
  out.type = resolve_fn_call(out.token.value().text, args);

  // Build c-name
  Type full_type;
  full_type.append_fn();

  bool first = true;
  for (const auto &arg : args) {
    if (first) {
      first = false;
    } else {
      full_type.append_join();
    }

    full_type.append_type(arg);
  }
  full_type.append_maps();
  full_type.append_type(out.type.value());

  out.c_name = full_type.mangle(out.token.value().text);

  return out;
}

/// Resolve the given variable
Type Parser::resolve_variable(const Lexer::Token &_name) {
  debug_print();

  for (auto frame = locals.rbegin(); frame != locals.rend();
       ++frame) {
    if (frame->contains(_name)) {
      return frame->at(_name);
    }
  }
  throw std::runtime_error("Variable '" + _name.text +
                           "' does not exist.");
}

// Parses a single object (resolvable variable or function
// call return value). Assumes we are pointing ot the first
// token of the object. Non-global (inside statements)
Node Parser::parse_object(
    std::list<Lexer::Token>::const_iterator &_cur_pos,
    const std::list<Lexer::Token>::const_iterator &_end) {
  debug_print();
  /*
  object = name | function_call | object . name ;
  */

  // Open parenthesis: Function call
  if (std::next(_cur_pos) != _end &&
      *std::next(_cur_pos) == "(") {
    return parse_function_call(_cur_pos, _end);
  }

  auto cur = *_cur_pos;
  const auto literal_type = Lexer::get_literal_type(cur);
  if (literal_type.has_value()) {
    // Literal
    Node out;
    out.node_type = Node::OBJECT;
    out.c_name = cur;
    out.type = literal_type.value();
    return out;
  } else {
    // Name
    Node out;
    out.token = *_cur_pos;

    auto name = *_cur_pos;
    Type t = resolve_variable(name);

    // Member access
    while (std::next(_cur_pos) != _end &&
           *std::next(_cur_pos) == ".") {

      incr(_cur_pos, _end); // pointing at .
      incr(_cur_pos, _end); // pointing at member name
      const auto member_name = _cur_pos->text;

      const auto info = globals.at(t.struct_name());

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

      name.text += "." + member_name;
    }

    out.node_type = Node::OBJECT;
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
  const auto will_provide =
      replace(provides, generics, _substitutions);
  if (will_provide.size() != _desired.size()) {
    return false;
  }
  auto l = will_provide.begin();
  auto r = _desired.end();
  while (l != will_provide.end()) {
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
std::list<Lexer::Token> Parser::TemplateInfo::replace(
    const std::list<Lexer::Token> &_to_augment,
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
  std::list<Lexer::Token> out;
  for (const auto &t : _to_augment) {
    if (substitution_map.contains(t)) {
      for (const auto &replacement : substitution_map.at(t)) {
        out.push_back({t, replacement});
      }
    } else {
      out.push_back(t);
    }
  }
  return out;
}

/// Run the given parser as necessary on this template.
/// This first checks for existing instances. If one exists,
/// returns true. If none exist, it replaces and parses the
/// validate block. If that works, it replaces and parses the
/// instantiate block. If the instantiate block fails, it
/// raises an error. If not, the instance is logged and we
/// return without error. Returns true on full success, false
/// on failure w/o error
bool Parser::TemplateInfo::attempt_instantiation(
    Parser &_p,
    const std::list<std::list<std::string>> &_substitutions) {
  debug_print();
  // Check for existing instances
  if (existing_instances.contains(_substitutions)) {
    return true;
  }

  // Build validation block
  const auto replaced_validation_block =
      replace(validate, generics, _substitutions);

  // Run validation block
  Parser backup = _p;
  try {
    _p.parse_global(replaced_validation_block);
  } catch (...) {
    _p = backup;
    return false;
  }

  // Replace the instantiation block
  const auto replaced_instantiation_block =
      replace(instantiate, generics, _substitutions);

  // Run instantiation block
  _p.parse_global(replaced_instantiation_block);

  // Log any success
  existing_instances.insert(_substitutions);

  return true;
}

// Resolves a function call through any means necessary. If
// it cannot be resolved, an error is thrown.
Type Parser::resolve_fn_call(const std::string &_name,
                             const std::vector<Type> &_args) {
  debug_print();
  const static auto fn_call_str = [&]() -> std::string {
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
  };

  // Do any templates
  try {
    std::list<std::pair<std::list<std::list<std::string>>,
                        std::list<TemplateInfo>::iterator>>
        candidates;
    find_substitutions(_name, _args, candidates);
    for (const auto &t : candidates) {
      t.second->attempt_instantiation(*this, t.first);
    }
  } catch (std::runtime_error &_e) {
    throw std::runtime_error("Error during template checking "
                             "requested by function call '" +
                             fn_call_str() + "':\n" +
                             _e.what());
  } catch (...) {
    throw std::runtime_error(
        "Unknown error during template checking "
        "requested by function call '" +
        fn_call_str() + "'");
  }

  // Attempt existing instances
  std::list<FnInfo> exact_matches, cast_matches, ref_matches;
  for (const auto &instance : functions[_name]) {
    bool exact = true, ref = true, cast = true;
    const auto instance_args = instance.t.fn_args();

    if (instance_args.size() != _args.size()) {
      continue;
    }

    for (uint i = 0; i < instance_args.size(); ++i) {
      if (exact &&
          !_args[i].exact_match(instance_args[i].second)) {
        exact = false;
      }
      if (ref && !_args[i].ref_match(instance_args[i].second)) {
        ref = false;
      }
      if (cast &&
          !_args[i].cast_match(instance_args[i].second)) {
        cast = false;
      }
    }

    if (exact) {
      exact_matches.push_back(instance);
    } else if (ref) {
      ref_matches.push_back(instance);
    } else if (cast) {
      cast_matches.push_back(instance);
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
              fn_call_str() + "'");
        } else {
          return cast_matches.front().t.fn_return_type();
        }
      }
    } else {
      // Use ref matches
      if (ref_matches.size() != 1) {
        throw std::runtime_error(
            "Multiple reference matches were "
            "found for function call '" +
            fn_call_str() + "'");
      } else {
        return ref_matches.front().t.fn_return_type();
      }
    }
  } else {
    // Use exact matches
    if (exact_matches.size() != 1) {
      throw std::runtime_error("Multiple exact matches were "
                               "found for function call '" +
                               fn_call_str() + "'");
    } else {
      return exact_matches.front().t.fn_return_type();
    }
  }

  // Throw error if it couldn't be resolved
  throw std::runtime_error("No existing candidate nor "
                           "providing template could be "
                           "found for function call '" +
                           fn_call_str() + "'");
}

/// Finds all possible template instantiations to match the
/// given function call information
void Parser ::find_substitutions(
    const std::string &_name,
    const std::vector<Type> &_arg_types,
    std::list<std::pair<std::list<std::list<std::string>>,
                        std::list<TemplateInfo>::iterator>>
        &_candidates) const {
  debug_print();
  for (const auto &cand : templates) {
    throw std::runtime_error("UNIMPLEMENTED: " +
                             std::string(__FUNCTION__));
  }
}

// Throws an error on invalid type (EG undefined struct
// name)
void Parser::validate_type(const Type &_t) const {
  debug_print();
  for (const auto &node : _t.nodes) {
    if (node.type == Type::TypeNode::LITERAL) {
      if (Type::int_literals.contains(node.literal_name)) {
        continue;
      } else if (Type::uint_literals.contains(
                     node.literal_name)) {
        continue;
      } else if (Type::float_literals.contains(
                     node.literal_name)) {
        continue;
      }

      if (!globals.contains(node.literal_name)) {
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
  if (globals.contains(_name)) {
    // Some syntactic fluff on the "variant" type
    if (std::holds_alternative<StructInfo>(globals.at(_name))) {
      out = std::get<StructInfo>(globals.at(_name));
    } else {
      out = std::get<EnumInfo>(globals.at(_name));
    }
  } else if (functions.contains(_name)) {
    out = functions.at(_name);
  }
  return out;
}
