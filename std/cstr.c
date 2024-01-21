#include "/usr/include/oak/std_oak_header.h"

u128 strlen_FN_ARR_i8_MAPS_u128(i8 what[])
{
    return strlen(what);
}

u128 strlen_FN_str_MAPS_u128(str what)
{
    return strlen(what);
}

void strcpy_FN_ARR_i8_JOIN_ARR_i8_MAPS_void(i8 to[], i8 from[])
{
    strcpy(to, from);   
}

void strncpy_FN_ARR_i8_JOIN_ARR_i8_JOIN_u128_MAPS_void(i8 to[], i8 from[], u128 n)
{
    strncpy(to, from, n);
}

void strcpy_FN_ARR_i8_JOIN_str_MAPS_void(i8 to[], str from)
{
    strcpy(to, from);   
}

void strncpy_FN_ARR_i8_JOIN_str_JOIN_u128_MAPS_void(i8 to[], str from, u128 n)
{
    strncpy(to, from, n);
}

i32 strcmp_FN_ARR_i8_JOIN_str_MAPS_i32(i8 l[], str r)
{
    return strcmp(l, r);
}

i32 strncmp_FN_ARR_i8_JOIN_str_JOIN_u128_MAPS_i32(i8 l[], str r, u128 n)
{
    return strncmp(l, r, n);
}

i32 strcmp_FN_str_JOIN_str_MAPS_i32(str l, str r)
{
    return strcmp(l, r);
}

i32 strncmp_FN_str_JOIN_str_JOIN_u128_MAPS_i32(str l, str r, u128 n)
{
    return strncmp(l, r, n);
}

i32 strcmp_FN_str_JOIN_ARR_i8_MAPS_i32(str l, i8 r[])
{
    return strcmp(l, r);
}

i32 strncmp_FN_str_JOIN_ARR_i8_JOIN_u128_MAPS_i32(str l, i8 r[], u128 n)
{
    return strncmp(l, r, n);
}

i32 strcmp_FN_ARR_i8_JOIN_ARR_i8_MAPS_i32(i8 l[], i8 r[])
{
    return strcmp(l, r);
}

i32 strncmp_FN_ARR_i8_JOIN_ARR_i8_JOIN_u128_MAPS_i32(i8 l[], i8 r[], u128 n)
{
    return strncmp(l, r, n);
}

