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

    test_repr("main", "i32 main(_: i32, __: i8 **)",
              {"(", "_", ":", "i32", ",", "_", ":", "^", "^",
               "i8", ")", "->", "i32"});
  }

  { // Test different equality modes
    ;
  }

  { // Test fn operations
    Type t;
    for (const auto &i : {"(", "_", ":", "i32", ",", "__", ":",
                          "bool", ",", "___", ":", "^", "[",
                          "]", "i8", ",", ")", "->", "void"}) {
      t.process_next(i);
    }
    std::cout << t.oak_repr() << '\n';
    assert(t.oak_repr() == "(i32, bool, ^[]i8) -> void");

    assert(t.is_fn());
    const auto args = t.fn_args();

    assert(args.size() == 3);
    assert(args.at("_").oak_repr() == "i32");
    assert(args.at("__").oak_repr() == "bool");
    assert(args.at("___").oak_repr() == "^[]i8");

    const auto ret = t.fn_return_type();
    assert(ret.oak_repr() == "void");
  }

  std::cout << "All type unit tests passed!\n";

  return 0;
}
