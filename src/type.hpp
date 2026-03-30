/**
 * @file
 * @brief Defines the `Type` class.
 */

#pragma once

#include "ast_node.hpp"
#include <map>
#include <string>
#include <vector>

/**
 * @class Type
 * @brief A class representing a type that an entry in a symbol
 * table might have. This CAN be a function or a function
 * pointer!
 */
class Type {
public:
  /// Default constructor
  Type(const ASTNode &_type_ast = {}) : type_ast(_type_ast) {
  }

  /// A higher number is more precise. The goal is not to lose
  /// any precision in our casts. These are EG i32, i64, int
  const static std::map<std::string, uint> int_literals;

  /// A higher number is more precise. The goal is not to lose
  /// any precision in our casts. These are EG u32, u64, uint
  const static std::map<std::string, uint> uint_literals;

  /// A higher number is more precise. The goal is not to lose
  /// any precision in our casts. These are EG f32, f64, float
  const static std::map<std::string, uint> float_literals;

  /// Returns true iff the given type is atomic (EG i32, bool).
  /// If the type is an array or pointer, this is always false.
  static bool is_built_in_type(const Type &_what) noexcept;

  /// Returns true iff the given type is atomic (EG i32, bool).
  /// If the type is an array or pointer, this is always false.
  static bool
  is_built_in_type(const std::string &_what) noexcept;

  /// Return this type in Oak notation
  /// O(n)
  std::string oak_repr(const std::string &_var_name = "") const;

  /// Return this type in C notation. If this is a function,
  /// mangle it.
  /// O(n)
  std::string c_repr(const std::string &_var_name = "",
                     const bool &_no_mangle = false) const;

  /// If this is a function, mangle it.
  /// O(n)
  std::string mangle(const std::string &_var_name = "") const;

  /// Returns true iff the other matches this at every node
  /// O(n)
  bool exact_match(const Type &_other) const;

  /// Returns true iff this type can be cast to match the other
  /// O(n)
  bool cast_match(const Type &_other) const;

  /// Returns true iff the other matches this after only legal
  /// reference handling. We are allowed to deref ourselves any
  /// number of times, but we are only allowed to add one ref.
  /// No casting is allowed here!
  /// O(n)
  bool ref_match(const Type &_other, int &_num_deref) const;

  /// Helper that ignores the _num_deref arg, only giving a bool
  inline bool ref_match(const Type &_other) const {
    int junk = 0;
    return ref_match(_other, junk);
  }

  /// Returns the struct name of this type for parse-time
  /// lookup. Errors if not a direct instance of a struct or
  /// enum
  std::string struct_name() const;

  /// Returns true iff the first node is of type FUNCTION
  /// O(1)
  bool is_fn() const noexcept;

  /// Returns true iff the type is a ptr (NOT an unsized array)
  bool is_ptr() const noexcept;

  /// Returns true iff the type is a array (sized or unsized)
  bool is_arr() const noexcept;

  /// Returns true iff the type is a sized array
  bool is_sized_arr() const noexcept;

  /// Returns true iff the type is an unsized array
  bool is_unsized_arr() const noexcept;

  /// Throws if not a sized array
  ASTNode sized_arr_size() const;

  /// Returns true iff the first node is of type POINTER and the
  /// second node is of type FUNCTION
  /// O(1)
  bool is_fn_ptr() const noexcept;

  /// Gets the arguments, given that this is a function
  /// O(n)
  std::vector<std::pair<std::string, Type>> fn_args() const;

  /// Gets the fn return type, given that this is a function
  /// O(n)
  Type fn_return_type() const;

  /// Returns a COPY of this type if it were to be dereferenced
  /// once (throwing if this is not a dereferenceable type)
  Type deref() const;

  /// Same as deref, but allows arrays (sized or unsized)
  Type deref_allow_arrays() const;

  /// Returns a COPY of this type if it were to be referenced
  /// once
  Type ref() const;

  /// Internal type representation
  ASTNode type_ast;

  /// Cast to the underlying AST
  inline operator ASTNode() const noexcept {
    return type_ast;
  }
};
