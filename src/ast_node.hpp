/**
 * @file
 * @brief Abstract Syntax Tree nodes used in program
 * reconstruction
 */

#pragma once

#include "debug.hpp"
#include "type.hpp"
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <variant>

/// A smattering of node types for AST parsing
namespace ASTNodes {

/// A union of all AST node types
using Node =
    std::variant<class Object, class Call, class Return,
                 class Declaration, class ArrAccess,
                 class Statement, class RawCFormat, class If,
                 class While, class Match, class Case>;

/// Custom implementation that is just an indirection
template <typename T> struct Box {
  Box() {
    _data = new T;
  }

  Box(const T &_other) {
    _data = new T(_other);
  }

  Box(const Box<T> &_other) {
    _data = new T(_other.get());
  }

  Box<T> &operator=(const T &_other) {
    *_data = _other;
    return *this;
  }

  ~Box() {
    db_assert(_data != nullptr);
    delete _data;
  }

  T &get() const noexcept {
    db_assert(_data != nullptr);
    return *_data;
  }

private:
  T *_data;
};

/// Custom implementation that is just an indirection
template <typename T> struct OptBox {
  OptBox() {
    _data = nullptr;
  }

  OptBox(const T &_other) {
    _data = new T(_other);
  }

  OptBox(const OptBox<T> &_other) {
    if (_other.has_value()) {
      _data = new T(_other.get());
    }
  }

  OptBox<T> &operator=(const T &_other) {
    if (!has_value()) {
      _data = new T;
    }
    *_data = _other;
    return *this;
  }

  ~OptBox() {
    if (has_value()) {
      delete _data;
    }
  }

  bool has_value() const noexcept {
    return _data != nullptr;
  }

  T &get() const {
    if (!has_value()) {
      throw std::runtime_error(
          "Cannot get value from empty OptBox");
    }
    return *_data;
  }

private:
  T *_data;
};

/// A resolved object in the form of a string
struct Object {
  /// The C identifier of the object
  std::string raw_text;

  /// The type of this object
  Type type;
};

/// A return statement
struct Return {
  /// If provided, the value
  OptBox<Node> value;
};

/// A (resolved) function call
struct Call {
  /// A single argument in the call
  struct Arg {
    /// The identifier of this arg
    Box<Node> name;

    /// The type of this arg
    Type type;

    /// The number of times (possibly negative) that this arg
    /// needs to be dereferenced in C
    int derefs;
  };

  /// The C identifier
  std::string mangled_c_fn_name;

  /// The arguments
  std::list<Arg> args;

  /// The type returned by the fn call
  Type return_type;
};

/// Zero or more sequential statements
struct Statement {
  /// The children, in order
  std::vector<Box<Node>> children;
};

/// A branch of a match
struct Case {
  /// The enum option name
  std::string case_name;

  /// The name given to the option value
  std::string passed_name;

  /// The type of the option value
  Type type;

  /// The body of the case
  Box<Node> body;
};

/// A Rust-style match
struct Match {
  /// The enum instance we are looking at
  Box<Node> upon;

  /// The enum root name (of which is option is a part)
  std::string enum_name;

  /// The tines of the match statement
  std::list<Box<Node>> branches;

  /// If true, all the cases will be references
  bool is_mutable = false;
};

/// A while statement
struct While {
  /// The condition
  Box<Node> condition;

  /// What is executed within the loop
  Box<Node> body;
};

/// An if-then-else statement where the else block is optional
struct If {
  /// The condition
  Box<Node> condition;

  /// What is executed if the condition is true
  Box<Node> then_body;

  /// If provided, the else block
  OptBox<Node> else_body;
};

/// Declares one or more named variables and provides their
/// constructors
struct Declaration {
  /// The type shared by all of them
  Type type;

  /// The names of the variables
  std::list<std::string> names;

  /// Constructors
  std::list<Box<Node>> new_calls;
};

/// A low-level array access object
struct ArrAccess {
  /// The thing being accessed
  Box<Node> upon;

  /// The index into `upon`
  Box<Node> index;

  ///
  Type return_type;
};

/// A format string in C
struct RawCFormat {
  /// The format string, where any '%' is replaced  by the next
  /// format argument
  std::string format_string;

  /// The arguments to be inserted at '%'
  std::vector<Box<Node>> args;

  /// If provided, the return type
  std::optional<Type> type;
};

/// Recursively reconstruct AST in C
void reconstruct(const Node &_what, std::ostream &_where);

///
Type type(const Node &_what);

} // namespace ASTNodes
