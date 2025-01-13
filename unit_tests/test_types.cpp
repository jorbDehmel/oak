#include "../src/type.hpp"
#include <cassert>
#include <iostream>

int main() {
  { // Test Oak repr and validity checks
    Type t;
    assert(!t.valid());
    t.append_ptr();
    assert(!t.valid());
    t.append_sized_arr(123);
    assert(!t.valid());
    t.append_ptr();
    assert(!t.valid());
    t.append_fn();
    assert(!t.valid());
    t.append_maps();
    assert(!t.valid());
    t.append_literal("void");
    assert(t.valid());

    assert(t.oak_repr("var_name") ==
           "var_name: ^[123]^() -> void");
  }

  { // Test C representation
    ;
  }

  { // Test different equality modes
    ;
  }

  std::cout << "All type unit tests passed!\n";

  return 0;
}
