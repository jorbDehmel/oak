/**
 * @file
 * @brief Defines the `Type` class.
 */

#pragma once

#include <cstdint>
#include <initializer_list>
#include <list>
#include <map>
#include <stack>
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
  /// A higher number is more precise. The goal is not to lose
  /// any precision in our casts. These are EG i32, i64, int
  const static std::map<std::string, uint> int_literals;

  /// A higher number is more precise. The goal is not to lose
  /// any precision in our casts. These are EG u32, u64, uint
  const static std::map<std::string, uint> uint_literals;

  /// A higher number is more precise. The goal is not to lose
  /// any precision in our casts. These are EG f32, f64, float
  const static std::map<std::string, uint> float_literals;

  /// Default constructor
  Type() = default;

  /**
   * @brief Parse some series of tokens as a type
   */
  Type(const std::initializer_list<std::string> &_tokens) {
    for (const auto &t : _tokens) {
      process_next(t);
    }
  }

  friend class Parser;

  /// Process one token. This should be treated as consumptive.
  /// O(1)
  void process_next(const std::string &_symbol);

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
    int junk;
    return ref_match(_other, junk);
  }

  /// Returns the struct name of this type for parse-time
  /// lookup. Errors if not a direct instance of a struct or
  /// enum
  std::string struct_name() const;

  /// Returns whether or not this type is valid to instantiate
  /// O(n)
  bool valid() const noexcept;

  /// Returns true iff the first node is of type FUNCTION
  /// O(1)
  bool is_fn() const noexcept;

  /// Gets the arguments, given that this is a function
  /// O(n)
  std::vector<std::pair<std::string, Type>> fn_args() const;

  /// Gets the fn return type, given that this is a function
  /// O(n)
  Type fn_return_type() const;

protected:
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

  /// Appends the entire other type (EG fn arg)
  void append_type(const Type &_other);

  /// Returns a COPY of this type if it were to be dereferenced
  /// once
  Type deref() const;

  /**
   * @struct TypeNode
   * @brief A single node in a type
   */
  struct TypeNode {
    /**
     * @enum TypeTag
     * @brief The type of this typenode (EG pointer, literal,
     * array)
     */
    enum TypeTag {
      POINTER,
      UNSIZED_ARRAY,
      SIZED_ARRAY,
      LITERAL,
      FUNCTION,
      JOIN,
      MAPS,
    };

    /// The type of this typenode
    TypeTag type = LITERAL;

    /// Used only in literal nodes
    std::string literal_name = "";

    /// Used only in sized array nodes
    uint64_t sized_array_size = 0;

    /// Used in "function" and "join" nodes to list argument
    /// names
    std::string following_arg_name = "";
  };

  /// Used for parsing types from token streams
  std::stack<std::string> enclosure;

  /// Internal type representation
  std::list<TypeNode> nodes;
};
