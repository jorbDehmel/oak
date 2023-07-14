
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

#include <map>
#include <stdio.h>
using namespace std;

#define impl(onto, name, what) \
    onto.__methods[name] = (void *)(&what)
#define call(from, name, type) \
    ((type)from.__methods[name])(from)

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
typedef float f16;
typedef double f32;
typedef long double f64;

// Misc
typedef const char *const str;

// Base class for all live structs
struct __live_base
{
    map<str, void *> __methods;
};

#endif
