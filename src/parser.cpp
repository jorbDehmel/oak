/**
 * @file parser.cpp
 */

#include "parser.hpp"
#include <linux/limits.h>
#include <stdexcept>
#include <variant>

/**
 * @brief Safely increment an iterator. If it is the end before
 * OR AFTER incrementation, throws an error.
 * @param _it The iterator to increment
 * @param _end The end position from the iterand
 */
void incr(auto _it, auto _end) {
  if (_it == _end) {
    throw std::runtime_error(
        "Cannot increment iterator past end of iterand.");
  }
  ++_it;
  if (_it == _end) {
    throw std::runtime_error("Unexpected EOF during parsing!");
  }
}

// Parse a global scope
void Parser::parse_global(
    const std::list<Lexer::Token> &_file_contents) {
  // Iterate and delegate. No macros remain.
  auto pos = _file_contents.begin();
  const auto end = _file_contents.end();

  while (pos != _file_contents.end()) {
    if (*pos == "let") {
      incr(pos, end);
      std::set<std::string> names = {*pos};
      incr(pos, end);
      if (*pos == ":") {
        // Struct, enum, or invalid global definition
        incr(pos, end);

        if (*pos == "struct") {
          parse_struct(names, pos, end);
        } else if (*pos == "enum") {
          parse_enum(names, pos, end);
        } else {
          throw std::runtime_error(
              "Global scope 'let' error: Expected 'struct' or "
              "'enum', saw '" +
              pos->text + "'.");
        }
      } else if (*pos == "(") {
        // Function
        parse_function(names, pos, end);
      } else {
        throw std::runtime_error("Global scope 'let' error: "
                                 "Expected '(' or ':', saw '" +
                                 pos->text + "'.");
      }
    } else {
      throw std::runtime_error(
          "Global scope parse error: Unexpected token '" +
          pos->text + "'.");
    }
  }
}

// Resets the state of the translation unit
void Parser::reset() {}

// Constructs the equivalent C program at the given path
void Parser::reconstruct(const std::filesystem::path &_where) {}

// Parse a single function declaration
// Assumes we have just seen "let NAME (" and are pointing to
// the next token.
void Parser::parse_function(
    const std::set<std::string> &_names,
    std::list<Lexer::Token>::const_iterator &_cur_pos,
    const std::list<Lexer::Token>::const_iterator &_end) {
  // Finish parsing typeb
  Type t;
  do {
    t.process_next(*_cur_pos);
    incr(_cur_pos, _end);
  } while (!t.valid());

  // Either signature or implementation
  if (*_cur_pos == ";") {
    // Signature
    for (const auto &name : _names) {
      // Mark as existing w/o definition
    }
  } else if (*_cur_pos != "{") {
    // Neither signature nor implementation: error
    throw std::runtime_error("Function-definition 'let' error: "
                             "Expected '{' or ';', saw '" +
                             _cur_pos->text + "'.");
  }

  // Implementation
  Node to_add;
}

// Parse a single struct declaration
// Assumes we have just seen "let NAME : struct" and are
// pointing to the next token.
void Parser::parse_struct(
    const std::set<std::string> &_names,
    std::list<Lexer::Token>::const_iterator &_cur_pos,
    const std::list<Lexer::Token>::const_iterator &_end) {}

// Parse a single enum declaration
// Assumes we have just seen "let NAME : enum" and are
// pointing to the next token.
void Parser::parse_enum(
    const std::set<std::string> &_names,
    std::list<Lexer::Token>::const_iterator &_cur_pos,
    const std::list<Lexer::Token>::const_iterator &_end) {}

// Assumes we are pointing to the first token in the statement
Node Parser::parse_statement(
    std::list<Lexer::Token>::const_iterator &_cur_pos,
    const std::list<Lexer::Token>::const_iterator &_end) {
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
          _cur_pos->text + "'.");
    }
    incr(_cur_pos, _end);

    // Get type
    Type t;
    do {
      t.process_next(*_cur_pos);
      incr(_cur_pos, _end);
    } while (!t.valid());

    // Add all to symbol table
    for (const auto &name : names) {
    }

    Node out;
    out.node_type = Node::NONE;
    return out;
  } else if (*_cur_pos == "{") {
    // Scope
    Node out;
    out.node_type = Node::STMT;

    // Add a scope frame to the scope stack
    locals.push({});

    incr(_cur_pos, _end);
    while (*_cur_pos == "}") {
      out.children.push_back(parse_statement(_cur_pos, _end));
      incr(_cur_pos, _end);
    }

    // Remove that scope frame
    locals.pop();

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
    ++_cur_pos;
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
  } else {
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

    out.type = resolve_fn_call(out.token.value().text, args);
    return out;
  }
}

// Parses a single object (resolvable variable or function
// call return value). Assumes we are pointing ot the first
// token of the object. Non-global (inside statements)
Node Parser::parse_object(
    std::list<Lexer::Token>::const_iterator &_cur_pos,
    const std::list<Lexer::Token>::const_iterator &_end) {
  // Check for local variables
}

// Resolves a function call through any means necessary. If
// it cannot be resolved, an error is thrown.
Type Parser::resolve_fn_call(const std::string &_name,
                             const std::vector<Type> &_args) {}

// Throws an error on invalid type (EG undefined struct name)
void Parser::validate_type(const Type &_t) const {
  for (const auto &node : _t.nodes) {
    if (node.type == Type::TypeNode::LITERAL) {
      if (!definitions.contains(node.literal_name) ||
          std::holds_alternative<std::list<FnInfo>>(
              definitions.at(node.literal_name))) {
        throw std::runtime_error("Atomic type '" +
                                 node.literal_name +
                                 "' does not exist.");
      }
    }
  }
}
