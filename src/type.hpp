/**
 * @file type.hpp
 * @brief Defines the `Type` class.
 */

#pragma once

#include <list>
#include <map>
#include <string>
#include <variant>

/**
 * @class Type
 * @brief A class representing a type that an entry in a symbol
 * table might have. This CAN be a function or a function
 * pointer!
 */
class Type {
public:
  /// Return this type in Oak notation
  std::string oak_repr(const std::string &_var_name = "") const;

  /// Return this type in C notation. If this is a function,
  /// mangle it.
  std::string c_repr(const std::string &_var_name = "") const;

  /// Returns true iff the other matches this at every node
  bool exact_match(const Type &_other) const;

  /// Returns true iff this type can be cast to match the other
  bool cast_match(const Type &_other) const;

  /// Returns true iff the other matches this after only legal
  /// reference handling
  bool ref_match(const Type &_other) const;

  /// Returns whether or not this type is valid to instantiate
  bool valid() const;

  /// Appends a pointer node to this type
  void append_ptr();

  /// Appends an unsized array node to this type
  void append_arr();

  /// Appends a size array node to this type
  void append_sized_arr(const uint64_t &_size);

  /// Appends a literal node ot this type WITHOUT checking its
  /// existence or size.
  void append_literal(const std::string &_name);

  /// Appends a function open node to this type
  void append_fn();

  /// Appends a join node to this type
  void append_join();

  /// Appends a function close node ("maps") to this type
  void append_maps();

protected:
  struct TypeNode {
    enum {
      POINTER,
      UNSIZED_ARRAY,
      SIZED_ARRAY,
      LITERAL,
      FUNCTION,
      JOIN,
      MAPS,
    } type;
    std::string literal_name;
    uint64_t sized_array_size;
  };
  std::list<TypeNode> nodes;
  uint fn_depth = 0;
  bool is_valid_type = false;
};
