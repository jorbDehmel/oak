/**
 * @file type.cpp
 * @brief Implements the `Type` class.
 */

#include "type.hpp"
#include <map>
#include <stdexcept>

/// Return this type in Oak notation
std::string Type::oak_repr(const std::string &_var_name) const {
  std::string out;

  if (!_var_name.empty()) {
    out = _var_name;
    if (nodes.empty() ||
        nodes.front().type != TypeNode::FUNCTION) {
      out += ": ";
    }
  }

  for (const auto &node : nodes) {
    switch (node.type) {
    case TypeNode::POINTER:
      out += "^";
      break;
    case TypeNode::UNSIZED_ARRAY:
      out += "[]";
      break;
    case TypeNode::SIZED_ARRAY:
      out += "[" + std::to_string(node.sized_array_size) + "]";
      break;
    case TypeNode::LITERAL:
      out += node.literal_name;
      break;
    case TypeNode::FUNCTION:
      out += "(";
      break;
    case TypeNode::JOIN:
      out += ", ";
      break;
    case TypeNode::MAPS:
      out += ") -> ";
      break;
    }
  }
  if (!is_valid_type) {
    out = "<INVALID TYPE> " + out;
  }
  return out;
}

/// Return this type in C notation, mangling if a raw function
/// (not function pointers though)
std::string Type::c_repr(const std::string &_var_name) const {
  // Mangle if needed
  std::string real_name = _var_name;
  if (nodes.front().type == TypeNode::FUNCTION) {
    for (const auto &node : nodes) {
      switch (node.type) {
      case TypeNode::POINTER:
        real_name += "_PTR";
        break;
      case TypeNode::UNSIZED_ARRAY:
      case TypeNode::SIZED_ARRAY:
        real_name += "_ARR";
        break;
      case TypeNode::LITERAL:
        real_name += "_" + node.literal_name;
        break;
      case TypeNode::FUNCTION:
        real_name += "_FN";
        break;
      case TypeNode::JOIN:
        real_name += "_JOIN";
        break;
      case TypeNode::MAPS:
        real_name += "_MAPS";
        break;
      }
    }
  }

  std::string prefix, suffix;

  // Do actual type construction
  throw std::runtime_error(__FILE_NAME__ ":" +
                           std::to_string(__LINE__) +
                           "> Unimplemented!");

  return prefix += _var_name + suffix;
}

/// Returns true iff the other matches this at every node
bool Type::exact_match(const Type &_other) const {
  if (nodes.size() != _other.nodes.size()) {
    return false;
  }
  auto mine = nodes.begin();
  auto theirs = _other.nodes.begin();
  while (mine != nodes.end() && theirs != _other.nodes.end()) {
    if (mine->type != theirs->type) {
      return false;
    } else if (mine->type == TypeNode::LITERAL &&
               mine->literal_name != theirs->literal_name) {
      return false;
    } else if (mine->type == TypeNode::SIZED_ARRAY &&
               mine->sized_array_size !=
                   theirs->sized_array_size) {
      return false;
    }
    ++mine, ++theirs;
  }
  return true;
}

/// Returns true iff this type can be cast to match the other
bool Type::cast_match(const Type &_other) const {
  // A higher number is more precise. The goal is not to lose
  // any precision in our casts.
  const static std::map<std::string, uint> int_literals = {
      {"i8", 1},  {"i16", 2},   {"i32", 4},
      {"i64", 8}, {"i128", 16}, {"int", sizeof(int)}};
  const static std::map<std::string, uint> uint_literals = {
      {"u8", 1},  {"u16", 2},   {"u32", 4},
      {"u64", 8}, {"u128", 16}, {"uint", sizeof(uint)}};
  const static std::map<std::string, uint> float_literals = {
      {"f32", 4},
      {"f64", 8},
      {"f128", 16},
      {"float", sizeof(double)}};

  if (nodes.size() != _other.nodes.size()) {
    return false;
  }
  auto mine = nodes.begin();
  auto theirs = _other.nodes.begin();
  while (mine != nodes.end() && theirs != _other.nodes.end()) {
    if (mine->type != theirs->type) {
      return false;
    } else if (mine->type == TypeNode::LITERAL &&
               mine->literal_name != theirs->literal_name) {

      // Possible casting case
      const auto &my_type = mine->literal_name;
      const auto &their_type = theirs->literal_name;
      if (int_literals.contains(my_type) &&
          int_literals.contains(their_type)) {
        if (int_literals.at(my_type) >
            int_literals.at(their_type)) {
          return false;
        }
      } else if (uint_literals.contains(my_type) &&
                 uint_literals.contains(their_type)) {
        if (uint_literals.at(my_type) >
            uint_literals.at(their_type)) {
          return false;
        }
      } else if (float_literals.contains(my_type) &&
                 float_literals.contains(their_type)) {
        if (float_literals.at(my_type) >
            float_literals.at(their_type)) {
          return false;
        }
      } else {
        // Do not share literal type genre, cannot be cast
        return false;
      }
    } else if (mine->type == TypeNode::SIZED_ARRAY &&
               mine->sized_array_size !=
                   theirs->sized_array_size) {
      return false;
    }
    ++mine, ++theirs;
  }
  return true;
}

/// Returns true iff the other matches this after only legal
/// reference handling
bool Type::ref_match(const Type &_other) const {
  throw std::runtime_error(__FILE_NAME__ ":" +
                           std::to_string(__LINE__) +
                           "> Unimplemented!");
}

/// Returns whether or not this type is valid to instantiate
bool Type::valid() const { return is_valid_type; }

/// Appends a pointer node to this type
void Type::append_ptr() {
  if (is_valid_type) {
    throw std::runtime_error(
        "Cannot append 'pointer' to an already concrete type: "
        "Did you mean to put the caret before the type?");
  }
  nodes.push_back({TypeNode::POINTER});
}

/// Appends an unsized array node to this type
void Type::append_arr() {
  if (is_valid_type) {
    throw std::runtime_error(
        "Cannot append 'array' to an already concrete type: "
        "Did you mean to put brackets before the type?");
  }
  nodes.push_back({TypeNode::UNSIZED_ARRAY});
}

/// Appends a size array node to this type
void Type::append_sized_arr(const uint64_t &_size) {
  if (is_valid_type) {
    throw std::runtime_error(
        "Cannot append 'sized array' to an "
        "already concrete type: Did you mean to put brackets "
        "before the type?");
  } else if (_size == 0) {
    throw std::runtime_error(
        "Sized array cannot be of size zero!");
  }
  nodes.push_back({TypeNode::SIZED_ARRAY, {}, _size});
}

/// Appends a literal node ot this type WITHOUT checking its
/// existence or size.
void Type::append_literal(const std::string &_name) {
  if (is_valid_type) {
    throw std::runtime_error("Cannot append 'literal' to an "
                             "already concrete type.");
  }
  nodes.push_back({TypeNode::LITERAL, _name});
  if (fn_depth == 0) {
    is_valid_type = true;
  }
}

/// Appends a function open node to this type
void Type::append_fn() {
  if (is_valid_type) {
    throw std::runtime_error(
        "Cannot append 'function open' to an "
        "already concrete type.");
  }
  nodes.push_back({TypeNode::FUNCTION});
  ++fn_depth;
}

/// Appends a join node to this type
void Type::append_join() {
  if (is_valid_type) {
    throw std::runtime_error(
        "Cannot append 'join' to an already concrete type.");
  } else if (fn_depth == 0) {
    throw std::runtime_error(
        "Cannot append 'join' to a non-function type.");
  }
  nodes.push_back({TypeNode::JOIN});
}

/// Appends a function close node ("maps") to this type
void Type::append_maps() {
  if (is_valid_type) {
    throw std::runtime_error(
        "Cannot append 'maps' to an already concrete type.");
  } else if (fn_depth == 0) {
    throw std::runtime_error(
        "Cannot append 'maps' to a non-function type.");
  }
  if (nodes.back().type == TypeNode::JOIN) {
    nodes.pop_back();
  }
  nodes.push_back({TypeNode::MAPS});
  --fn_depth;
}
