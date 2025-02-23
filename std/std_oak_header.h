/**
 * @file std_oak_header.h
 * @brief The file to be included in all Oak c files
 * @author J Dehmel, 2025-present, MIT license
 */

#ifndef STD_OAK_HEADER_H
#define STD_OAK_HEADER_H
#define OAK

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

// Type defs
typedef uint8_t u8;
typedef int8_t i8;

typedef uint16_t u16;
typedef int16_t i16;

typedef uint32_t u32;
typedef int32_t i32;
typedef float f32;

typedef uint64_t u64;
typedef int64_t i64;
typedef double f64;

typedef unsigned int uint;

// Experimental definitions: Not usually supported at their true
// sizes.
typedef long double f128;
typedef unsigned long long u128;
typedef long long i128;

// Validity assertions
static_assert(sizeof(u8) == 1, "Invalid compile-time sizes!");
static_assert(sizeof(i8) == 1, "Invalid compile-time sizes!");
static_assert(sizeof(u16) == 2, "Invalid compile-time sizes!");
static_assert(sizeof(i16) == 2, "Invalid compile-time sizes!");
static_assert(sizeof(u32) == 4, "Invalid compile-time sizes!");
static_assert(sizeof(i32) == 4, "Invalid compile-time sizes!");
static_assert(sizeof(f32) == 4, "Invalid compile-time sizes!");
static_assert(sizeof(u64) == 8, "Invalid compile-time sizes!");
static_assert(sizeof(i64) == 8, "Invalid compile-time sizes!");
static_assert(sizeof(f64) == 8, "Invalid compile-time sizes!");

#ifdef __cplusplus
}
#endif

#endif
