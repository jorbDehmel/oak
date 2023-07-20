/*
Jordan Dehmel, 2023
GPLv3

Standard io object files with std_io.oak
This file must be compiled, as symbols are
provided during linking for cross-language
compilation.
*/

#include <iostream>
using namespace std;

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

typedef const char *const str;

void print(str what)
{
    cout << what;
    return;
}

void print(i32 what)
{
    cout << what;
    return;
}

void print(f64 what)
{
    cout << what;
    return;
}

void print_err(str what)
{
    cerr << what;
    return;
}

void println_err(str what)
{
    cerr << what << '\n';
    return;
}

char get_char()
{
    char out;
    cin >> out;
    return out;
}
