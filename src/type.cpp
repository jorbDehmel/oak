/**
 * @file type.cpp
 * @brief Implements the `Type` class.
 */

#include "type.hpp"
#include <cwctype>
#include <map>
#include <set>
#include <stdexcept>

// A higher number is more precise. The goal is not to lose
// any precision in our casts.
const std::map<std::string, uint> Type::int_literals = {
    {"i8", 1},  {"i16", 2},   {"i32", 4},
    {"i64", 8}, {"i128", 16}, {"int", sizeof(int)}};
const std::map<std::string, uint> Type::uint_literals = {
    {"u8", 1},  {"u16", 2},   {"u32", 4},
    {"u64", 8}, {"u128", 16}, {"uint", sizeof(uint)}};
const std::map<std::string, uint> Type::float_literals = {
    {"f32", 4},
    {"f64", 8},
    {"f128", 16},
    {"float", sizeof(double)}};

/// Process one token. This should be treated as consumptive.
void Type::process_next(const std::string &_symbol) {
  if (_symbol == "^") {
    append_ptr();
  } else if (_symbol == ",") {
    append_join();
  } else if (_symbol == "->") {
    append_maps();
  } else if (_symbol == "(") {
    append_fn();
  } else if (_symbol == "[") {
    enclosure.push(_symbol);
  }

  // Begining of 2-token "maps"
  else if (_symbol == ")") {
    while (!enclosure.empty() && enclosure.top() == "*") {
      enclosure.pop();
    }
    if (enclosure.empty() || enclosure.top() != "(") {
      throw std::runtime_error("Unexpected ')'.");
    }
    enclosure.pop();
  }

  // Unclosed array
  else if (!enclosure.empty() && enclosure.top() == "[") {
    if (_symbol == "]") {
      append_arr();
      enclosure.pop();
    } else {
      uint64_t size = 0;
      try {
        size = std::stoull(_symbol);
      } catch (...) {
        throw std::runtime_error(
            "Array size must be a compile-time integer: "
            "Instead, saw " +
            _symbol + ".");
      }
      if (size == 0) {
        throw std::runtime_error("Array size must be nonzero.");
      }
      append_sized_arr(size);
      enclosure.top().push_back('*');
    }
  }

  // Overclosed array
  else if (_symbol == "]") {
    if (enclosure.empty() || enclosure.top() != "[*") {
      throw std::runtime_error("Unexpected '" + _symbol + "'.");
    }
    enclosure.pop();
  }

  // Type or arg name
  else if (_symbol != ";") {
    if (!enclosure.empty() && enclosure.top() == "*") {
      if (!nodes.empty()) {
        if (nodes.back().following_arg_name == "") {
          nodes.back().following_arg_name = _symbol;
        } else if (_symbol != ":") {
          throw std::runtime_error(
              "Expected ':'. Arguments must take the form "
              "'name: type' (even in implicit declarations).");
        }
      }
      enclosure.pop();
    } else {
      append_literal(_symbol);
    }
  }
}

/// Returns true iff the first node is of type FUNCTION
bool Type::is_fn() const noexcept {
  return (nodes.size() >= 1 &&
          nodes.front().type == TypeNode::FUNCTION);
}

/// Gets the arguments, given that this is a function
std::vector<std::pair<std::string, Type>>
Type::fn_args() const {
  if (!is_fn()) {
    throw std::runtime_error(
        "Cannot get arguments of non-function type '" +
        oak_repr() + "'.");
  } else if (!is_valid_type) {
    throw std::runtime_error(
        "Cannot get args of invalid type.");
  }

  uint64_t depth = 0;
  std::vector<std::pair<std::string, Type>> out;
  std::set<std::string> used_names;
  Type t;
  std::string argname;

  for (const auto &node : nodes) {
    if (node.type == TypeNode::FUNCTION) {
      ++depth;
      if (depth == 1) {
        argname = node.following_arg_name;
        continue;
      }
    } else if (node.type == TypeNode::MAPS) {
      --depth;
      if (depth == 0) {
        while (used_names.contains(argname)) {
          argname = "_" + argname;
        }
        if (!t.nodes.empty()) {
          t.is_valid_type = is_valid_type;
          out.push_back({argname, t});
          used_names.insert(argname);
        }
        break;
      }
    }

    if (depth == 0) {
      continue;
    } else {
      if (depth == 1 && node.type == TypeNode::JOIN) {
        while (used_names.contains(argname)) {
          argname = "_" + argname;
        }
        if (!t.nodes.empty()) {
          t.is_valid_type = is_valid_type;
          out.push_back({argname, t});
          used_names.insert(argname);
        }
        argname = node.following_arg_name;
        t = Type{};
      } else {
        t.nodes.push_back(node);
      }
    }
  }

  return out;
}

/// Gets the fn return type, given that this is a function
Type Type::fn_return_type() const {
  if (!is_fn()) {
    throw std::runtime_error(
        "Cannot get return type of non-function type '" +
        oak_repr() + "'.");
  } else if (!is_valid_type) {
    throw std::runtime_error(
        "Cannot get return type of invalid type.");
  }

  Type out = *this;
  int count = 0;
  while (!out.nodes.empty()) {
    if (out.nodes.front().type == TypeNode::FUNCTION) {
      ++count;
    } else if (out.nodes.front().type == TypeNode::MAPS) {
      --count;
      if (count == 0) {
        out.nodes.pop_front();
        break;
      }
    }
    out.nodes.pop_front();
  }
  return out;
}

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
    if (!node.following_arg_name.empty()) {
      out += node.following_arg_name + ": ";
    }
  }
  if (!is_valid_type) {
    out = "<INVALID TYPE> " + out;
  }
  return out;
}

/// Return this type in C notation, mangling if a raw function
/// (not function pointers though)
std::string Type::c_repr(const std::string &_var_name,
                         const bool &_no_mangle) const {
  // Dispatch based on type: Function pointers get one method,
  // regular types get another.
  std::string repr;
  if (nodes.size() >= 2 &&
      nodes.front().type == TypeNode::POINTER &&
      std::next(nodes.begin())->type == TypeNode::FUNCTION) {
    // Function pointer
    repr = deref().c_repr("(*" + _var_name + ")", true);
  } else if (nodes.size() >= 1 &&
             nodes.front().type == TypeNode::FUNCTION) {
    // Regular function
    // Mangle
    std::string name = _var_name;
    if (!_no_mangle) {
      for (const auto &node : nodes) {
        switch (node.type) {
        case TypeNode::POINTER:
          name += "_PTR";
          break;
        case TypeNode::UNSIZED_ARRAY:
        case TypeNode::SIZED_ARRAY:
          name += "_ARR";
          break;
        case TypeNode::LITERAL:
          name += "_" + node.literal_name;
          break;
        case TypeNode::FUNCTION:
          name += "_FN";
          break;
        case TypeNode::JOIN:
          name += "_JOIN";
          break;
        case TypeNode::MAPS:
          name += "_MAPS";
          break;
        }
      }
    }

    const auto ret_type = fn_return_type();
    const auto args = fn_args();

    // Real stuff
    std::string out = ret_type.c_repr(name) + "(";
    bool first = true;
    for (const auto &arg : args) {
      if (first) {
        first = false;
      } else {
        out += ", ";
      }
      out += arg.second.c_repr(arg.first);
    }
    out += ")";
    repr = out;
  } else {
    // Normal type
    std::string prefix, suffix;

    for (const auto &node : nodes) {
      switch (node.type) {
      case TypeNode::POINTER:
        prefix += "*";
        break;
      case TypeNode::UNSIZED_ARRAY:
        suffix += "[]";
        break;
      case TypeNode::SIZED_ARRAY:
        suffix +=
            "[" + std::to_string(node.sized_array_size) + "]";
        break;
      case TypeNode::LITERAL:
        if (!int_literals.contains(node.literal_name) &&
            !uint_literals.contains(node.literal_name) &&
            !float_literals.contains(node.literal_name) &&
            node.literal_name != "bool" &&
            node.literal_name != "void") {
          prefix = "struct " + node.literal_name + prefix;
        } else {
          prefix = node.literal_name + prefix;
        }
        break;

      default:
        break;
      }
    }

    repr = prefix + " " + _var_name + suffix;
  }

  if (!is_valid_type) {
    repr = "<INVALID TYPE> " + repr;
  }

  while (std::isspace(repr.back())) {
    repr.pop_back();
  }

  return repr;
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
/// reference handling. We are allowed to deref ourselves any
/// number of times, but we are only allowed to add one ref.
/// No casting is allowed here!
bool Type::ref_match(const Type &_other,
                     int &_num_deref) const {
  Type me = *this;
  Type it = _other;
  uint num_me_derefs = 0;
  uint num_it_derefs = 0;

  // Fully deref both sides
  while (!me.nodes.empty() &&
         me.nodes.front().type == TypeNode::POINTER) {
    me = me.deref();
    ++num_me_derefs;
  }
  while (!it.nodes.empty() &&
         it.nodes.front().type == TypeNode::POINTER) {
    it = it.deref();
    ++num_it_derefs;
  }

  // Net number of derefs on this to get to other
  _num_deref = num_me_derefs - num_it_derefs;

  // CCheck for legality
  if (!me.exact_match(it)) {
    return false;
  } else if (_num_deref < -1) {
    return false;
  } else {
    return true;
  }
}

/// Returns whether or not this type is valid to instantiate
bool Type::valid() const noexcept {
  return is_valid_type;
}

/// Appends a pointer node to this type
void Type::append_ptr() {
  if (is_valid_type) {
    throw std::runtime_error(
        "Cannot append 'pointer' to an already concrete "
        "type '" +
        oak_repr() +
        "': Did you mean to put the caret before the type?");
  }
  nodes.push_back({TypeNode::POINTER});
}

/// Appends an unsized array node to this type
void Type::append_arr() {
  if (is_valid_type) {
    throw std::runtime_error(
        "Cannot append 'array' to an already concrete type '" +
        oak_repr() +
        "': Did you mean to put brackets before the type?");
  }
  nodes.push_back({TypeNode::UNSIZED_ARRAY});
}

/// Appends a size array node to this type
void Type::append_sized_arr(const uint64_t &_size) {
  if (is_valid_type) {
    throw std::runtime_error(
        "Cannot append 'sized array' to an "
        "already concrete type '" +
        oak_repr() +
        "': Did you mean to put brackets "
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
                             "already concrete type '" +
                             oak_repr() + "'");
  }
  nodes.push_back({TypeNode::LITERAL, _name});
  if (is_valid_type) {
    is_valid_type = false;
  } else if (enclosure.empty()) {
    is_valid_type = true;
  }
}

/// Appends a function open node to this type
void Type::append_fn() {
  enclosure.push("(");

  // Denote that the next two tokens should be ignored
  // since they will be 'name :'
  enclosure.push("*");
  enclosure.push("*");

  if (is_valid_type) {
    throw std::runtime_error(
        "Cannot append 'function open' to an "
        "already concrete type '" +
        oak_repr() + "'");
  }
  nodes.push_back({TypeNode::FUNCTION});
}

/// Appends a join node to this type
void Type::append_join() {
  while (!enclosure.empty() && enclosure.top() == "*") {
    enclosure.pop();
  }

  if (is_valid_type) {
    throw std::runtime_error(
        "Cannot append 'join' to an already concrete type '" +
        oak_repr() + "'");
  } else if (enclosure.empty() || enclosure.top() != "(") {
    throw std::runtime_error(
        "Cannot append 'join' to a non-function type '" +
        oak_repr() + "'");
  }
  nodes.push_back({TypeNode::JOIN});

  // Denote that the next two tokens should be ignored
  // since they will be 'name :'
  enclosure.push("*");
  enclosure.push("*");
}

/// Appends a function close node ("maps") to this type
void Type::append_maps() {
  if (is_valid_type) {
    throw std::runtime_error(
        "Cannot append 'maps' to an already concrete type '" +
        oak_repr() + "'");
  }
  if (nodes.back().type == TypeNode::JOIN) {
    nodes.pop_back();
  }
  nodes.push_back({TypeNode::MAPS});
}

/// Returns a COPY of this type if it were to be dereferenced
/// once
Type Type::deref() const {
  if (nodes.empty() ||
      nodes.front().type != TypeNode::POINTER) {
    throw std::runtime_error("Cannot deref non-pointer type '" +
                             oak_repr() + "'");
  }

  Type out = *this;
  out.nodes.pop_front();
  return out;
}

/// Returns the struct name of this type for parse-time
/// lookup. Errors if not a direct instance of a struct
std::string Type::struct_name() const {
  if (!is_valid_type) {
    throw std::runtime_error(
        "Cannot get struct name of invalid type.");
  } else if (nodes.empty() ||
             nodes.front().type == TypeNode::LITERAL) {
    throw std::runtime_error(
        "Cannot get struct name of non-terminal type '" +
        oak_repr() + "'");
  }
  return nodes.front().literal_name;
}
