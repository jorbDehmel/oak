/*
 */

#include "oak/std/std_oak_header.h"
#include <stdlib.h>

// srand(seed: uint) -> void
void srand_FN_uint_MAPS_void(uint seed) {
  srand(seed);
}

// rand() -> int
int rand_FN_MAPS_int() {
  return rand();
}
