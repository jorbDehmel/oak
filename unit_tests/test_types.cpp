/**
 * @file test_types.cpp
 * @brief Tests the Oak typing system
 */

#include "../src/type.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>

void test_repr(const std::string &_name,
               const std::string &_c_type,
               const std::list<std::string> &_oak) {
  Type t;
  std::string c;
  for (const auto &i : _oak) {
    t.process_next(i);
  }

  c = t.c_repr(_name);

  if (_c_type != c) {
    std::cout << "Expected C-type | " << _c_type << '\n'
              << "Observed Oak    | " << t.oak_repr(_name)
              << '\n'
              << "Observed C-type | " << c << '\n';
    throw std::runtime_error("Mismatch");
  }
}

int main() {
  { // Test Oak repr and validity checks
    Type t;
    assert(!t.valid());
    t.process_next("^");
    assert(!t.valid());
    t.process_next("[");
    t.process_next("123");
    t.process_next("]");
    assert(!t.valid());
    t.process_next("^");
    assert(!t.valid());
    t.process_next("(");
    assert(!t.valid());
    t.process_next(")");
    t.process_next("->");
    assert(!t.valid());
    t.process_next("void");
    assert(t.valid());

    assert(t.oak_repr("var_name") ==
           "var_name: ^[123]^() -> void");
  }

  { // Test C representation
    test_repr("foo", "i32* foo", {"^", "i32"});
    test_repr("foo", "i32 foo[]", {"[", "]", "i32"});
    test_repr("foo", "bool* foo[64]",
              {"^", "[", "64", "]", "bool"});

    // Non-main fn
    test_repr(
        "not_main",
        "i32 not_main_FN_i32_JOIN_PTR_PTR_i8_MAPS_i32(i32 c, "
        "i8** v)",
        {"(", "c", ":", "i32", ",", "v", ":", "^", "^", "i8",
         ")", "->", "i32"});

    // Fn ptr
    test_repr("fn_ptr", "void (*fn_ptr)(i32 _, i32 (*__)())",
              {"^", "(", "_", ":", "i32", ",", "_", ":", "^",
               "(", ")", "->", "i32", ")", "->", "void"});
  }

  { // Test different equality modes
    assert(Type({"i32"}).exact_match(Type({"i32"})));
    assert(!Type({"i32"}).exact_match(Type({"i64"})));

    assert(Type({"i32"}).cast_match(Type({"i32"})));
    assert(Type({"i32"}).cast_match(Type({"i64"})));
    assert(!Type({"i64"}).cast_match(Type({"i32"})));

    // t.ref_match();
    assert(Type({"i32"}).ref_match(Type({"^", "i32"})));
    assert(Type({"^", "^", "i32"}).ref_match(Type({"i32"})));
    assert(!Type({"i32"}).ref_match(Type({"^", "^", "i32"})));

    assert(!Type({"^", "i32"})
                .exact_match(Type({"[", "]", "i32"})));
    assert(!Type({"^", "i32"})
                .cast_match(Type({"[", "]", "i32"})));
    assert(
        !Type({"^", "i32"}).ref_match(Type({"[", "]", "i32"})));
  }

  { // Test fn operations
    Type t;
    for (const auto &i :
         {"(", "_", ":", "i32", ",", "_", ":", "bool", ",", "_",
          ":", "^", "[", "]", "i8", ",", ")", "->", "void"}) {
      t.process_next(i);
    }
    assert(t.oak_repr() ==
           "(_: i32, _: bool, _: ^[]i8) -> void");

    assert(t.is_fn());
    const auto args = t.fn_args();

    assert(args.size() == 3);

    assert(args[0].first == "_");
    assert(args[1].first == "__");
    assert(args[2].first == "___");

    assert(args[0].second.oak_repr() == "i32");
    assert(args[1].second.oak_repr() == "bool");
    assert(args[2].second.oak_repr() == "^[]i8");

    const auto ret = t.fn_return_type();
    assert(ret.oak_repr() == "void");

    assert(ret.c_repr() == "void");
  }

  std::cout << "All type unit tests passed!\n";

  return 0;
}
