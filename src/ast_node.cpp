#include "ast_node.hpp"
#include <stdexcept>
#include <variant>

void ASTNodes::reconstruct(const ASTNodes::Node &_what,
                           std::ostream &_where) {
  if (std::holds_alternative<ASTNodes::If>(_what)) {
    const auto d = std::get<ASTNodes::If>(_what);

    _where << "if (";
    reconstruct(d.condition.get(), _where);
    _where << ")";
    reconstruct(d.then_body.get(), _where);
    if (d.else_body.has_value()) {
      _where << "else ";
      reconstruct(d.else_body.get(), _where);
    }
  }

  else if (std::holds_alternative<ASTNodes::While>(_what)) {
    const auto d = std::get<ASTNodes::While>(_what);
    _where << "while (";
    reconstruct(d.condition.get(), _where);
    _where << ")";
    reconstruct(d.body.get(), _where);
  }

  else if (std::holds_alternative<ASTNodes::Match>(_what)) {
    const auto d = std::get<ASTNodes::Match>(_what);

    _where << "switch ((";
    reconstruct(d.upon.get(), _where);
    _where << ").__info) {\n";
    for (const auto &branch : d.branches) {
      if (std::holds_alternative<ASTNodes::Statement>(
              branch.get())) {
        // 'else'
        const auto branch_d =
            std::get<ASTNodes::Statement>(branch.get());
        _where << "default: {";
        reconstruct(branch_d, _where);
        _where << "} break;\n";
      } else {
        // 'case'
        const auto branch_d =
            std::get<ASTNodes::Case>(branch.get());
        _where << "case " << d.enum_name << "_OPT_"
               << branch_d.case_name << ": {\n"
               << branch_d.type.c_repr(branch_d.passed_name)
               << " = " << (d.is_mutable ? "&" : "") << "(";
        reconstruct(d.upon.get(), _where);
        _where << ").__data." << branch_d.case_name << "; {";
        reconstruct(branch_d.body.get(), _where);
        _where << "}} break;\n";
      }
    }
    _where << "}\n";
  }

  else if (std::holds_alternative<ASTNodes::RawCFormat>(
               _what)) {
    const auto d = std::get<ASTNodes::RawCFormat>(_what);
    // Literal C format string
    uint cur_child = 0;
    for (const char &c : d.format_string) {
      if (c == '%' && cur_child < d.args.size()) {
        // Format case
        reconstruct(d.args.at(cur_child).get(), _where);
        ++cur_child;
      } else {
        // Literal C
        _where << c;
      }
    }
  }

  else if (std::holds_alternative<ASTNodes::Object>(_what)) {
    const auto d = std::get<ASTNodes::Object>(_what);
    // Literal or variable
    _where << d.raw_text;
  }

  else if (std::holds_alternative<ASTNodes::Call>(_what)) {
    const auto d = std::get<ASTNodes::Call>(_what);
    _where << d.mangled_c_fn_name << "(";
    bool first = true;
    for (const auto &arg : d.args) {
      if (first) {
        first = false;
      } else {
        _where << ", ";
      }

      if (arg.derefs > 0) {
        for (int i = 0; i < arg.derefs; ++i) {
          _where << "*";
        }
      } else {
        for (int i = 0; i > arg.derefs; --i) {
          _where << "&";
        }
      }
      reconstruct(arg.name.get(), _where);
    }
    _where << ")";
  }

  else if (std::holds_alternative<ASTNodes::Declaration>(
               _what)) {
    const auto d = std::get<ASTNodes::Declaration>(_what);

    // Variable declarations
    for (const auto &name : d.names) {
      _where << d.type.c_repr(name) << ";\n";
    }

    // `New` calls
    for (const auto &call : d.new_calls) {
      reconstruct(call.get(), _where);
      _where << "\n";
    }
  }

  else if (std::holds_alternative<ASTNodes::Statement>(_what)) {
    const auto d = std::get<ASTNodes::Statement>(_what);
    if (d.children.size() > 1) {
      // Scope
      _where << "{\n";
      for (const auto &child : d.children) {
        reconstruct(child.get(), _where);
        _where << ";\n";
      }
      _where << "}\n";
    } else if (d.children.size() == 1) {
      // Simple statement
      reconstruct(d.children.front().get(), _where);
      _where << ";\n";
    }
  }

  else if (std::holds_alternative<ASTNodes::Return>(_what)) {
    const auto d = std::get<ASTNodes::Return>(_what);
    _where << "return ";
    if (d.value.has_value()) {
      reconstruct(d.value.get(), _where);
    }
    _where << ";\n";
  }

  else if (std::holds_alternative<ASTNodes::ArrAccess>(_what)) {
    const auto d = std::get<ASTNodes::ArrAccess>(_what);
    // Array access: 2 children (var and index)
    _where << "(";
    reconstruct(d.upon.get(), _where);
    _where << "[";
    reconstruct(d.index.get(), _where);
    _where << "])";
  }
}

Type ASTNodes::type(const ASTNodes::Node &_what) {
  if (std::holds_alternative<ASTNodes::If>(_what) ||
      std::holds_alternative<ASTNodes::While>(_what) ||
      std::holds_alternative<ASTNodes::Match>(_what) ||
      std::holds_alternative<ASTNodes::Case>(_what) ||
      std::holds_alternative<ASTNodes::Declaration>(_what) ||
      std::holds_alternative<ASTNodes::Statement>(_what) ||
      std::holds_alternative<ASTNodes::Return>(_what)) {
    // No type
    throw std::runtime_error(
        "Expected typed object, but found untyped statement, "
        "declaration, or conditional.");
  }

  else if (std::holds_alternative<ASTNodes::Object>(_what)) {
    return std::get<ASTNodes::Object>(_what).type;
  } else if (std::holds_alternative<ASTNodes::Call>(_what)) {
    // RETURN TYPE ONLY
    return std::get<ASTNodes::Call>(_what).return_type;
  } else if (std::holds_alternative<ASTNodes::ArrAccess>(
                 _what)) {
    return std::get<ASTNodes::ArrAccess>(_what).return_type;
  } else if (std::holds_alternative<ASTNodes::RawCFormat>(
                 _what)) {
    if (std::get<ASTNodes::RawCFormat>(_what)
            .type.has_value()) {
      return std::get<ASTNodes::RawCFormat>(_what).type.value();
    }
    throw std::runtime_error("RawCFormat AST node is missing "
                             "type when one is expected");
  }

  else {
    // Me-proofing for when I add another variant and forget
    // to change this
    throw std::runtime_error(__FILE__ ":" +
                             std::to_string(__LINE__) +
                             " Unreachable state reached!");
  }
}
