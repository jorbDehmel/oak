/*
Jordan Dehmel, 2023
GPLv3

Standard io object files with std_io.oak
This file must be compiled, as symbols are
provided during linking for cross-language
compilation.
*/

#include "oak/std_oak_header.hpp"
#include <stdio.h>

void print(str what)
{
    printf("%s", what);
    return;
}

void print(i32 what)
{
    printf("%d", what);
    return;
}

void print(f64 what)
{
    printf("%f", what);
    return;
}

void print(i8 what)
{
    printf("%c", what);
    return;
}

void print(i8 *what)
{
    printf("%s", what);
    return;
}

void flush()
{
    fflush(stdout);
    fflush(stderr);
    return;
}

void print_err(str what)
{
    fprintf(stderr, "%s", what);
    return;
}

char get_char()
{
    char out = getchar();
    return out;
}
