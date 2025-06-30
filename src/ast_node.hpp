/**
 * @file
 * @brief Abstract Syntax Tree nodes used in program
 * reconstruction
 */

#pragma once

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

/// An optional box: Contains either nothing or one dynamically
/// allocated value owned by this instance. This is similar to
/// std::unique_ptr or std::indirect (C++26), but is custom for
/// trait satisfaction reasons.
template <typename T> class OptBox {
public:
  /// Construct with no contents
  OptBox() : data(nullptr) {
  }

  /// Construct with contents
  OptBox(const T &_other) : data(new T(_other)) {
  }

  /// DEEP COPY another optional box
  OptBox(const OptBox<T> &_other)
      : data(_other.has_value() ? new T(_other.get())
                                : nullptr) {
  }

  /// Copy the value, making that copy the value held by this
  /// box
  OptBox<T> &operator=(const T &_other) noexcept {
    if (!has_value()) {
      data = new T;
    }
    *data = _other;
    return *this;
  }

  /// DEEP COPY from another box
  OptBox<T> &operator=(const OptBox<T> &_other) {
    if (_other.has_value()) {
      if (!has_value()) {
        data = new T;
      }
      *data = _other.get();
    }
    return *this;
  }

  /// Free any owned memory
  ~OptBox() {
    if (has_value()) {
      delete data;
    }
  }

  /// Returns true iff this box has been assigned a value
  inline bool has_value() const noexcept {
    return data != nullptr;
  }

  /// Returns the value owned, throwing if none exists
  T &get() const {
    if (!has_value()) {
      throw std::runtime_error(
          "Cannot get value from empty OptBox");
    }
    return *data;
  }

private:
  /// The internal data, or nullptr if empty
  T *data;
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
    OptBox<Node> name;

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
  std::vector<OptBox<Node>> children;
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
  OptBox<Node> body;
};

/// A Rust-style match
struct Match {
  /// The enum instance we are looking at
  OptBox<Node> upon;

  /// The enum root name (of which is option is a part)
  std::string enum_name;

  /// The tines of the match statement
  std::list<OptBox<Node>> branches;

  /// If true, all the cases will be references
  bool is_mutable = false;
};

/// A while statement
struct While {
  /// The condition
  OptBox<Node> condition;

  /// What is executed within the loop
  OptBox<Node> body;
};

/// An if-then-else statement where the else block is optional
struct If {
  /// The condition
  OptBox<Node> condition;

  /// What is executed if the condition is true
  OptBox<Node> then_body;

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
  std::list<OptBox<Node>> new_calls;
};

/// A low-level array access object
struct ArrAccess {
  /// The thing being accessed
  OptBox<Node> upon;

  /// The index into `upon`
  OptBox<Node> index;

  /// The type returned: Almost always the type of which `upon`
  /// is an array
  Type return_type;
};

/// A format string in C
struct RawCFormat {
  /// The format string, where any '%' is replaced  by the next
  /// format argument
  std::string format_string;

  /// The arguments to be inserted at '%'
  std::vector<OptBox<Node>> args;

  /// If provided, the return type
  std::optional<Type> type;
};

/// Recursively reconstruct AST in C
void reconstruct(const Node &_what, std::ostream &_where);

/// Extracts the type from any node, throwing if it is untyped
Type type(const Node &_what);

} // namespace ASTNodes
