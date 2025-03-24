/*
Kernel panic interface for Oak
*/

#include "oak/std/std_oak_header.h"
#include <stdlib.h>

// panic() -> void
void panic_FN_MAPS_void() {
  exit(1);
}
