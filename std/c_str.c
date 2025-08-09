#include <std_oak_header.h>
#include <stdlib.h>
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

// stoi(what: []i8) -> int
int stoi_FN_ARR_i8_MAPS_int(i8 *what) {
  return atoi((char *)what);
}

// stou(what: []i8) -> uint
uint stou_FN_ARR_i8_MAPS_uint(i8 *what) {
  return strtoul((char *)what, NULL, 10);
}

// stof(what: []i8) -> float
float stof_FN_ARR_i8_MAPS_float(i8 *what) {
  return atof((char *)what);
}

// strcmp(lhs: []i8, rhs: []i8) -> int
int strcmp_FN_ARR_i8_JOIN_ARR_i8_MAPS_int(i8 *lhs, i8 *rhs) {
  return strcmp((char *)lhs, (char *)rhs);
}
