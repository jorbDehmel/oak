/**
 * @file
 * @brief Implements the `Type` class.
 */

#include "type.hpp"
#include <cassert>
#include <csignal>
#include <map>
#include <set>
#include <stdexcept>
#include <sys/types.h>

bool Type::is_ptr() const noexcept {
  return type_ast.text == "^";
}

bool Type::is_arr() const noexcept {
  return type_ast.text == "[]";
}

bool Type::is_sized_arr() const noexcept {
  return is_arr() && type_ast.children.size() == 2;
}

bool Type::is_unsized_arr() const noexcept {
  return is_arr() && type_ast.children.size() == 1;
}

ASTNode Type::sized_arr_size() const {
  if (!is_sized_arr()) {
    throw std::runtime_error(
        "Cannot get array size of non-sized-array type");
  }
  assert(type_ast.children.at(0).text == "object");
  return type_ast.children.at(0).children.at(1);
}

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

bool Type::is_built_in_type(const Type &_what) noexcept {
  if (!_what.type_ast.children.empty()) {
    return false;
  }
  const auto t = _what.type_ast.text;
  return is_built_in_type(t);
}

bool Type::is_built_in_type(const std::string &_what) noexcept {
  return int_literals.contains(_what) ||
         uint_literals.contains(_what) ||
         float_literals.contains(_what) || _what == "bool" ||
         _what == "void";
}

bool Type::is_fn() const noexcept {
  return type_ast.text == "->";
}

std::vector<std::pair<std::string, Type>>
Type::fn_args() const {
  if (!is_fn()) {
    throw std::runtime_error(
        "Cannot get arguments of non-function type '" +
        oak_repr() + "'.");
  }

  std::vector<std::pair<std::string, Type>> out;
  std::set<std::string> used_names;
  for (const auto &arg : type_ast.children.front().children) {
    assert(arg.children.size() == 2);
    const ASTNode type = arg.children.back();
    std::string name = arg.children.front().text;

    while (used_names.contains(name)) {
      name = "_" + name;
    }

    out.push_back({name, type});
    used_names.insert(name);
  }
  return out;
}

Type Type::fn_return_type() const {
  if (!is_fn()) {
    throw std::runtime_error(
        "Cannot get return type of non-function type '" +
        oak_repr() + "'.");
  }
  return type_ast.children.back();
}

std::string Type::oak_repr(const std::string &_var_name) const {
  if (!_var_name.empty()) {
    return _var_name + ": " + oak_repr();
  } else if (is_ptr()) {
    return "^" + deref().oak_repr();
  } else if (is_arr()) {
    if (is_sized_arr()) {
      return "[" + sized_arr_size().text + "]" +
             deref_allow_arrays().oak_repr();
    } else {
      return "[]" + deref_allow_arrays().oak_repr();
    }
  } else if (is_fn()) {
    std::string out = "(";
    const auto args = fn_args();
    const auto ret_type = fn_return_type();
    bool first = true;
    for (const auto &arg : args) {
      if (first) {
        first = false;
      } else {
        out += ", ";
      }
      out += arg.second.oak_repr(arg.first);
    }
    out += ") -> " + ret_type.oak_repr();
    return out;
  } else {
    return type_ast.text;
  }
}

std::string Type::c_repr(const std::string &_var_name,
                         const bool &_no_mangle) const {
  if (is_fn_ptr()) {
    // Function pointer
    return Type(type_ast.children.front())
        .c_repr("(*" + _var_name + ")", true);
  } else if (is_fn()) {
    // Regular function
    // Mangle
    std::string name = _var_name;
    if (!_no_mangle) {
      name = mangle(_var_name);
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
    return out;
  } else if (is_sized_arr()) {
    if (_var_name.empty()) {
      throw std::runtime_error(
          "Type '" + oak_repr() +
          "' cannot be cast to C (illegal sized array)");
    }

    return Type(type_ast.children.back()).c_repr("") + " " +
           _var_name + "[" + type_ast.children.front().text +
           "]";
  } else if (!_var_name.empty()) {
    return c_repr() + _var_name;
  } else {
    // Less weird types
    if (is_ptr() || is_unsized_arr()) {
      return Type(type_ast.children.front()).c_repr() + "*";
    } else {
      if (is_built_in_type(type_ast.text)) {
        return type_ast.text + " ";
      } else {
        return "struct " + type_ast.text + " ";
      }
    }
  }
}

std::string Type::mangle(const std::string &_var_name) const {

  if (!_var_name.empty()) {
    return _var_name + "_" + mangle("");
  } else if (is_ptr()) {
    return "PTR_" + deref().mangle();
  } else if (is_arr()) {
    if (is_sized_arr()) {
      return "SIZED_ARR_" + sized_arr_size().text + "_" +
             deref_allow_arrays().mangle();
    } else {
      return "ARR_" + deref_allow_arrays().mangle();
    }
  } else if (is_fn()) {
    std::string out = "FN";
    const auto args = fn_args();
    const auto ret_type = fn_return_type();
    bool first = true;
    for (const auto &arg : args) {
      out += "_";
      if (first) {
        first = false;
      } else {
        out += "JOIN_";
      }
      out += arg.second.mangle();
    }
    out += "_MAPS_" + ret_type.mangle();
    return out;
  } else {
    return type_ast.text;
  }
}

bool Type::exact_match(const Type &_other) const {
  if (type_ast.text != _other.type_ast.text) {
    return false;
  } else if (type_ast.children.size() !=
             _other.type_ast.children.size()) {
    return false;
  }

  if (type_ast.text == "arg") {
    // name, type
    return Type(type_ast.children.back())
        .exact_match(_other.type_ast.children.back());
  } else {
    for (uint i = 0; i < type_ast.children.size(); ++i) {
      if (!Type(type_ast.children.at(i))
               .exact_match(_other.type_ast.children.at(i))) {
        return false;
      }
    }
    return true;
  }
}

bool Type::cast_match(const Type &_other) const {
  // Special case: Void pointer casting
  // NOTE: This may cause some issues on C-side
  if (_other.is_ptr() &&
      exact_match(ASTNode("^", {ASTNode("void")}))) {
    return true;
  } else if (is_ptr() && _other.exact_match(
                             ASTNode("^", {ASTNode("void")}))) {
    return true;
  } else if (is_arr() || is_fn_ptr() || is_fn() || is_ptr()) {
    return exact_match(_other);
  }

  // Possible casting
  const auto &my_type = type_ast.text;
  const auto &their_type = _other.type_ast.text;
  if (my_type == their_type) {
    return true;
  } else if (int_literals.contains(my_type) &&
             int_literals.contains(their_type)) {
    if (int_literals.at(my_type) >
        int_literals.at(their_type)) {
      return false;
    }
    return true;
  } else if (uint_literals.contains(my_type) &&
             uint_literals.contains(their_type)) {
    if (uint_literals.at(my_type) >
        uint_literals.at(their_type)) {
      return false;
    }
    return true;
  } else if (float_literals.contains(my_type) &&
             float_literals.contains(their_type)) {
    if (float_literals.at(my_type) >
        float_literals.at(their_type)) {
      return false;
    }
    return true;
  }
  return false;
}

bool Type::ref_match(const Type &_other,
                     int &_num_deref) const {

  Type me = *this;
  Type it = _other;
  uint num_me_derefs = 0;
  uint num_it_derefs = 0;

  // Fully deref both sides
  while (me.is_ptr()) {
    me = me.deref();
    ++num_me_derefs;
  }
  while (it.is_ptr()) {
    it = it.deref();
    ++num_it_derefs;
  }

  // Net number of derefs on this to get to other
  _num_deref = num_me_derefs - num_it_derefs;

  // Check for legality
  if (!me.exact_match(it)) {
    return false;
  }

  return (-1 == _num_deref || _num_deref == 0);
}

Type Type::deref() const {
  if (!is_ptr()) {
    throw std::runtime_error("Cannot deref non-pointer type '" +
                             oak_repr() + "'");
  }
  return type_ast.children.front();
}

Type Type::deref_allow_arrays() const {
  if (!is_ptr() && !is_arr()) {
    throw std::runtime_error(
        "Cannot deref non-deref-able type '" + oak_repr() +
        "'");
  }
  return type_ast.children.front();
}

Type Type::ref() const {
  return Type(ASTNode("^", {type_ast}));
}

std::string Type::struct_name() const {
  if (!type_ast.children.empty()) {
    throw std::runtime_error(
        "Cannot get struct/enum name of non-terminal type '" +
        oak_repr() + "'");
  }
  return type_ast.text;
}

bool Type::is_fn_ptr() const noexcept {
  return is_ptr() && Type(type_ast.children.front()).is_fn();
}
