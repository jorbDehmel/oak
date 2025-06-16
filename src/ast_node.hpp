#pragma once
#include "type.hpp"
#include <optional>
#include <ostream>
#include <string>
#include <variant>

namespace ASTNodes {

using DataType =
    std::variant<struct If, struct While, struct Match,
                 struct Case, struct Object, struct Call,
                 struct Declaration, struct ArrAccess,
                 struct Statement, struct RawCFormat,
                 struct Return>;

///
struct Statement {
  ///
  std::vector<DataType> children;
};

///
struct Object {
  ///
  std::string raw_text;
};

///
struct If {
  ///
  Object condition;

  ///
  Statement then_body;

  ///
  std::optional<Statement> else_body;
};

///
struct While {
  ///
  Object condition;

  ///
  Statement body;
};

///
struct Match {
  ///
  Object upon;

  using Else = Statement;

  ///
  std::list<std::variant<Case, Else>> branches;

  ///
  bool is_mutable;
};

///
struct Case {
  ///
  std::string case_name;

  ///
  std::string enum_name;

  ///
  std::string passed_name;

  ///
  Statement body;

  ///
  Type type;
};

/// A (resolved) call
struct Call {
  ///
  struct Arg {
    std::string name;
    Type type;
    int derefs;
  };

  ///
  std::string mangled_c_fn_name;

  ///
  std::list<Arg> args;

  ///
  Type return_type;
};

///
struct Declaration {
  ///
  Type type;

  ///
  std::list<std::string> names;

  /// Constructors
  std::list<Call> new_calls;
};

///
struct ArrAccess {
  ///
  Object upon;

  ///
  Object index;
};

///
struct RawCFormat {
  ///
  std::string format_string;

  ///
  std::vector<DataType> args;
};

///
struct Return {
  /// If provided, the value
  std::optional<Object> value;
};

/// Recursively reconstruct in C
void reconstruct(const DataType &_what, std::ostream &_where);

} // namespace ASTNodes
