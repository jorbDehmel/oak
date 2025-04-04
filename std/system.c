/*
Interfaces with the std `c` system call
*/

#include "oak/std/std_oak_header.h"
#include <stdlib.h>
#include <unistd.h>

// system(cmd: []i8) -> i32
i32 system_FN_ARR_i8_MAPS_i32(i8 *cmd) {
  return system((char *)cmd);
}

// get_pid() -> int
int get_pid_FN_MAPS_int() {
  return getpid();
}

// fork() -> int
int fork_FN_MAPS_int() {
  return fork();
}

// get_uid() -> int
int get_uid_FN_MAPS_int() {
  return getuid();
}

// get_euid() -> int
int get_euid_FN_MAPS_int() {
  return geteuid();
}
