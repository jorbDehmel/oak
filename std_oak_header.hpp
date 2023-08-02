
/*
Oak translated document heading
Jordan Dehmel, jdehmel@outlook.com, 2023
This document was translated from .oak

This header protected by the GPLv3.
Unless otherwise specified, so is the
remainder of this translated document,
as well as its .oak source.
*/

#ifndef OAK_HEAD
#define OAK_HEAD

#if (defined(_WIN32) || defined(_WIN64))
#define WINDOWS
#endif

// Signed integer types
typedef char i8;
typedef short int i16;
typedef int i32;
typedef long int i64;
typedef long long int i128;

// Unsigned integer types
typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long int u64;
typedef unsigned long long int u128;

// Floating point types
typedef float f32;
typedef double f64;
typedef long double f128;

// Misc
typedef const char *const str;

#endif
