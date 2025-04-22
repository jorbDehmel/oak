/**
 * @file
 * @brief Basic interfacial I/O file for std oak
 * @author Jordan Dehmel
 * @year 2025
 * @license MIT
 */

#include "std_oak_header.h"
#include <stdio.h>

// print(_: []i8) -> void
void print_FN_ARR_i8_MAPS_void(i8 *_to_print) {
  printf("%s", _to_print);
}

// print(_: bool) -> void
void print_FN_bool_MAPS_void(bool _to_print) {
  printf("%s", _to_print ? "true" : "false");
}

// print(_: ^void) -> void
void print_FN_PTR_void_MAPS_void(void *_to_print) {
  printf("%p", _to_print);
}

// print(_: i8) -> void
void print_FN_i8_MAPS_void(i8 _to_print) {
  printf("%c", _to_print);
}

// print(_: u8) -> void
void print_FN_u8_MAPS_void(u8 _to_print) {
  printf("%c", _to_print);
}

// print(_: i16) -> void
void print_FN_i16_MAPS_void(i16 _to_print) {
  printf("%hi", _to_print);
}

// print(_: u16) -> void
void print_FN_u16_MAPS_void(u16 _to_print) {
  printf("%hu", _to_print);
}

// print(_: i32) -> void
void print_FN_i32_MAPS_void(i32 _to_print) {
  printf("%i", _to_print);
}

// print(_: u32) -> void
void print_FN_u32_MAPS_void(u32 _to_print) {
  printf("%u", _to_print);
}

// print(_: i64) -> void
void print_FN_i64_MAPS_void(i64 _to_print) {
  printf("%li", _to_print);
}

// print(_: u64) -> void
void print_FN_u64_MAPS_void(u64 _to_print) {
  printf("%lu", _to_print);
}

// print(_: f32) -> void
void print_FN_f32_MAPS_void(f32 _to_print) {
  printf("%f", _to_print);
}

// print(_: f64) -> void
void print_FN_f64_MAPS_void(f64 _to_print) {
  printf("%f", _to_print);
}

// print(_: i128) -> void
void print_FN_i128_MAPS_void(i128 _to_print) {
  printf("%lli", _to_print);
}

// print(_: u128) -> void
void print_FN_u128_MAPS_void(u128 _to_print) {
  printf("%llu", _to_print);
}

// print(_: f128) -> void
void print_FN_f128_MAPS_void(f128 _to_print) {
  printf("%Lf", _to_print);
}

// print(_: int) -> void
void print_FN_int_MAPS_void(int _to_print) {
  printf("%d", _to_print);
}

// print(_: uint) -> void
void print_FN_uint_MAPS_void(uint _to_print) {
  printf("%u", _to_print);
}

// let getch() -> i8
i8 getch_FN_MAPS_i8() {
  return getchar();
}

// get_i32() -> i32
i32 get_i32_FN_MAPS_i32() {
  i32 out = 0;
  scanf("%d", &out);
  return out;
}

// get_f64() -> f64
f64 get_f64_FN_MAPS_f64() {
  f64 out = 0.0;
  scanf("%lf", &out);
  return out;
}

// flush() -> void
void flush_FN_MAPS_void() {
  fflush(stdout);
}

// err(_: []i8) -> void
void err_FN_ARR_i8_MAPS_void(i8 *msg) {
  fprintf(stderr, "%s", (char *)msg);
}

// let endl() -> void;
void endl_FN_MAPS_void() {
#if (defined(WIN32) || defined(WINNT))
  printf("\r\n");
#else
  printf("\n");
#endif
}
