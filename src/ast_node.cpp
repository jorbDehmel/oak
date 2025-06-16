#include "ast_node.hpp"
#include <variant>

void ASTNodes::reconstruct(const ASTNodes::DataType &_what,
                           std::ostream &_where) {

  if (std::holds_alternative<ASTNodes::If>(_what)) {
    const auto d = std::get<ASTNodes::If>(_what);

    _where << "if (";
    reconstruct(d.condition, _where);
    _where << ")";
    reconstruct(d.then_body, _where);
    if (d.else_body.has_value()) {
      _where << "else ";
      reconstruct(d.else_body.value(), _where);
    }
  }

  else if (std::holds_alternative<ASTNodes::While>(_what)) {
    const auto d = std::get<ASTNodes::While>(_what);
    _where << "while (";
    reconstruct(d.condition, _where);
    _where << ")";
    reconstruct(d.body, _where);
  }

  else if (std::holds_alternative<ASTNodes::Match>(_what)) {
    const auto d = std::get<ASTNodes::Match>(_what);

    _where << "switch ((";
    reconstruct(d.upon, _where);
    _where << ").__info){\n";
    for (const auto &branch : d.branches) {
      if (std::holds_alternative<ASTNodes::Match::Else>(
              branch)) {
        // 'else'
        const auto branch_d =
            std::get<ASTNodes::Match::Else>(branch);
        _where << "default: {";
        reconstruct(branch_d, _where);
        _where << "} break;\n";
      } else {
        // 'case'
        const auto branch_d = std::get<ASTNodes::Case>(branch);
        _where << "case " << branch_d.enum_name << "_OPT_"
               << branch_d.case_name << ": {\n"
               << branch_d.type.c_repr(branch_d.passed_name)
               << " = " << (d.is_mutable ? "&" : "") << "(";
        reconstruct(d.upon, _where);
        _where << ").__data." << branch_d.case_name << "; {";
        reconstruct(branch_d.body, _where);
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
        reconstruct(d.args.at(cur_child), _where);
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
      _where << arg.name;
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
      reconstruct(call, _where);
      _where << ";\n";
    }
  }

  else if (std::holds_alternative<ASTNodes::Statement>(_what)) {
    const auto d = std::get<ASTNodes::Statement>(_what);
    if (!d.children.empty()) {
      // Scope
      _where << "{\n";
      for (const auto &child : d.children) {
        reconstruct(child, _where);
        _where << ";\n";
      }
      _where << "}\n";
    }
  }

  else if (std::holds_alternative<ASTNodes::Return>(_what)) {
    const auto d = std::get<ASTNodes::Return>(_what);
    _where << "return ";
    if (d.value.has_value()) {
      reconstruct(d.value.value(), _where);
    }
  }

  else if (std::holds_alternative<ASTNodes::ArrAccess>(_what)) {
    const auto d = std::get<ASTNodes::ArrAccess>(_what);
    // Array access: 2 children (var and index)
    _where << "(";
    reconstruct(d.upon, _where);
    _where << "[";
    reconstruct(d.index, _where);
    _where << "])";
  }
}
