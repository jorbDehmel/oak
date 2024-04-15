////////////////////////////////////////////////////////////////

/*
Jordan Dehmel, 2023
GPLv3

Standard io object files with std_io.oak
This file must be compiled, as symbols are
provided during linking for cross-language
compilation.
*/

////////////////////////////////////////////////////////////////

#include "/usr/include/oak/std_oak_header.h"
#include <stdio.h>

////////////////////////////////////////////////////////////////

void print_FN_str_MAPS_void(str what)
{
    printf("%s", what);
}

void print_FN_ARR_i8_MAPS_void(i8 what[])
{
    printf("%s", what);
}

////////////////////////////////////////////////////////////////

void print_FN_i8_MAPS_void(i8 what)
{
    printf("%c", what);
}

void print_FN_u8_MAPS_void(u8 what)
{
    printf("%c", what);
}

void print_FN_i16_MAPS_void(i16 what)
{
    printf("%hu", what);
}

void print_FN_u16_MAPS_void(u16 what)
{
    printf("%hi", what);
}

void print_FN_i32_MAPS_void(i32 what)
{
    printf("%i", what);
}

void print_FN_u32_MAPS_void(u32 what)
{
    printf("%u", what);
}

void print_FN_i64_MAPS_void(i64 what)
{
    printf("%li", what);
}

void print_FN_u64_MAPS_void(u64 what)
{
    printf("%lu", what);
}

void print_FN_i128_MAPS_void(i128 what)
{
    printf("%lli", what);
}

void print_FN_u128_MAPS_void(u128 what)
{
    printf("%llu", what);
}

void print_FN_f32_MAPS_void(f32 what)
{
    printf("%f", what);
}

void print_FN_f64_MAPS_void(f64 what)
{
    printf("%f", what);
}

////////////////////////////////////////////////////////////////

void flush_FN_MAPS_void(void)
{
    fflush(stdout);
    fflush(stderr);
}

void print_err_FN_str_MAPS_void(str what)
{
    fprintf(stderr, "%s", what);
}

////////////////////////////////////////////////////////////////

char get_char_FN_MAPS_char(void)
{
    char out = getchar();
    return out;
}

// Get a i128 from cin
i128 get_i128_FN_MAPS_i128(void)
{
    i128 out;
    scanf("%lld", &out);
    return out;
}

// Get a f64 from cin
f64 get_f64_FN_MAPS_f64(void)
{
    f64 out;
    scanf("%lf", &out);
    return out;
}

////////////////////////////////////////////////////////////////
