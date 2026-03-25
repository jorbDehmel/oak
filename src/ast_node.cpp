#include "ast_node.hpp"
#include "type.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>

void reconstruct(const ASTNode &_what, std::ostream &_where) {
  if (_what.text == "if") {
    assert(_what.children.size() == 2 ||
           _what.children.size() == 3);
    const auto condition = _what.children.at(0);
    const auto then_block = _what.children.at(1);

    _where << "if (";
    reconstruct(condition, _where);
    _where << ")";
    reconstruct(then_block, _where);

    if (_what.children.size() == 3) {
      const auto else_block = _what.children.at(2);
      _where << "else ";
      reconstruct(else_block, _where);
    }
  }

  else if (_what.text == "while") {
    assert(_what.children.size() == 2);
    const auto condition = _what.children.at(0);
    const auto body = _what.children.at(1);

    _where << "while (";
    reconstruct(condition, _where);
    _where << ")";
    reconstruct(body, _where);
  }

  else if (_what.text == "match") {
    assert(_what.children.size() == 4);
    const ASTNode upon = _what.children.at(0);
    const ASTNode branches = _what.children.at(1);
    const std::string enum_name = _what.children.at(2).text;
    const bool is_mutable =
        (_what.children.at(3).text == "true");

    _where << "switch ((";
    reconstruct(upon, _where);
    _where << ").__info) {\n";

    for (const ASTNode &branch : branches.children) {
      if (branch.text == "else") {
        assert(branch.children.size() == 1);
        _where << "default: {";
        reconstruct(branch.children.front(), _where);
        _where << "} break;\n";
      } else {
        // {case_name, passed_name, passed_type, statement}

        const std::string case_name =
            branch.children.at(0).text;
        const std::string passed_name =
            branch.children.at(1).text;
        const Type passed_type = branch.children.at(2);
        const ASTNode body = branch.children.at(3);

        _where << "case " << enum_name << "_OPT_" << case_name
               << ": {\n"
               << passed_type.c_repr(passed_name) << " = "
               << (is_mutable ? "&" : "") << "(";
        reconstruct(upon, _where);
        _where << ").__data." << case_name << "; {";
        reconstruct(body, _where);
        _where << "}} break;\n";
      }
    }
    _where << "}\n";
  }

  else if (_what.text == "raw_c_format") {
    // Literal C format string
    assert(_what.children.size() == 3);
    assert(_what.children.front().children.empty());
    const std::string format_string = _what.children.at(0).text;
    const ASTNode type = _what.children.at(1);
    const ASTNode args = _what.children.at(2);
    uint cur_child = 0;
    for (const char &c : format_string) {
      if (c == '%' && cur_child < args.children.size()) {
        // Format case
        reconstruct(args.children.at(cur_child), _where);
        ++cur_child;
      } else {
        // Literal C code
        _where << c;
      }
    }
  }

  else if (_what.text == "object") {
    // Literal or variable
    _where << _what.children.back().text;
  }

  else if (_what.text == "@") {
    // mangled_name, return_type, {args}
    // each arg is {arg_tree, derefs}
    assert(_what.children.size() == 3);
    const std::string mangled_name =
        _what.children.front().text;
    const Type ret_type = _what.children.at(1);
    const ASTNode args = _what.children.at(2);

    _where << mangled_name << "(";
    bool first = true;
    for (const auto &arg_info : args.children) {
      const ASTNode arg = arg_info.children.front();
      const int derefs =
          std::stoi(arg_info.children.back().text);

      if (first) {
        first = false;
      } else {
        _where << ", ";
      }

      if (derefs > 0) {
        for (int i = 0; i < derefs; ++i) {
          _where << "*";
        }
      } else {
        for (int i = 0; i > derefs; --i) {
          _where << "&";
        }
      }
      reconstruct(arg, _where);
    }
    _where << ")";
  }

  else if (_what.text == "let") {
    assert(_what.children.size() == 3);
    const auto names = _what.children.at(0);
    const auto new_calls = _what.children.at(1);
    const auto type = Type(_what.children.at(2));

    // Variable declarations
    for (const auto &name : names.children) {
      _where << type.c_repr(name.text) << ";\n";
    }

    // `New` calls
    for (const auto &call : new_calls.children) {
      reconstruct(call, _where);
      _where << "\n";
    }
  }

  else if (_what.text == "null") {
    return;
  }

  else if (_what.text == "statement") {
    if (_what.children.size() > 1) {
      // Scope
      _where << "{\n";
      for (const auto &child : _what.children) {
        reconstruct(child, _where);
      }
      _where << "}\n";
    } else if (_what.children.size() == 1) {
      // Simple statement
      reconstruct(_what.children.front(), _where);
      _where << ";\n";
    }
  }

  else if (_what.text == "return") {
    _where << "return ";
    if (!_what.children.empty()) {
      reconstruct(_what.children.front(), _where);
    }
    _where << ";\n";
  }

  else if (_what.text == "[]") {
    // Array access: 2 children (var and index)
    assert(_what.children.size() == 2);
    _where << "(";
    reconstruct(_what.children.at(0), _where);
    _where << "[";
    reconstruct(_what.children.at(1), _where);
    _where << "])";
  }
}

ASTNode type(const ASTNode &_what) {
  if (_what.text == "object") {
    return _what.children.front();
  } else if (_what.text == "@") {
    // Application, so only gives ret type
    return _what.children.at(1);
  } else if (_what.text == "[]") {
    return _what.children.back();
  } else if (_what.text == "raw_c_format") {
    return _what.children.at(1);
  }

  else {
    // No type
    throw std::runtime_error(
        "Expected typed object, but found untyped statement, "
        "declaration, or conditional.");
  }
}
