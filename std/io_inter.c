/*
Jordan Dehmel, 2023
GPLv3

Standard io object files with std_io.oak
This file must be compiled, as symbols are
provided during linking for cross-language
compilation.
*/

#include "oak/std_oak_header.h"
#include <stdio.h>

void print_FN_str_MAPS_void(str what)
{
    printf("%s", what);
    return;
}

void print_FN_ARR_i8_MAPS_void(i8 what[])
{
    printf("%s", what);
    return;
}

void print_FN_i32_MAPS_void(i32 what)
{
    printf("%d", what);
    return;
}

void print_FN_f64_MAPS_void(f64 what)
{
    printf("%f", what);
    return;
}

void print_FN_i8_MAPS_void(i8 what)
{
    printf("%c", what);
    return;
}

void flush_FN_MAPS_void(void)
{
    fflush(stdout);
    fflush(stderr);
    return;
}

void print_err_FN_str_MAPS_void(str what)
{
    fprintf(stderr, "%s", what);
    return;
}

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
