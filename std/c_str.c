#include <oak/std/std_oak_header.h>
#include <string.h>

// strcpy(dest: ^[]i8, src: []i8) -> []i8
i8 *strcpy_FN_PTR_ARR_i8_JOIN_ARR_i8_MAPS_ARR_i8(i8 **dest,
                                                 i8 *src) {
  return (i8 *)strcpy((char *)*dest, (char *)src);
}
// strlen(what: []i8) -> uint
uint strlen_FN_ARR_i8_MAPS_uint(i8 *what) {
  return strlen((char *)what);
}
