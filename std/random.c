#include "std_oak_header.h"
#include <stdlib.h>
#include <time.h>

// let srand() -> void
void srand_FN_MAPS_void() {
  srand(time(NULL));
}

// srand(seed: uint) -> void
void srand_FN_uint_MAPS_void(uint seed) {
  srand(seed);
}

// rand() -> int
int rand_FN_MAPS_int() {
  return rand();
}
