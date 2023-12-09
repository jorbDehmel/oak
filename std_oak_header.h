/*
Oak translated document heading
Jordan Dehmel, jdehmel@outlook.com, 2023
This header protected by the GPLv3.
*/

#ifndef OAK_HEAD
#define OAK_HEAD

#include <assert.h>
#include <memory.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// Signed integer types
typedef char i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef long long i128; // Not always supported

// Unsigned integer types
typedef unsigned char u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef unsigned long long u128; // Not always supported

// Floating point types
typedef float f32;
typedef double f64;
typedef long double f128; // Not usually supported

// Misc
typedef const char *const str;

#endif
