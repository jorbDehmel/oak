/*
Interfaces with the std `c` system call
*/

#include "oak/std/std_oak_header.h"
#include <stdlib.h>

// system(cmd: ^i8) -> i32
i32 system_FN_PTR_i8_MAPS_i32(i8 *cmd) {
  system(cmd);
}
