/*
Jordan Dehmel, 2023
jdehmel@outlook.com

For .o linking w/ Acorn STD package
*/

#include "oak/std_oak_header.h"

i32 s_to_i_FN_str_MAPS_i32(str what)
{
    bool negative = false;
    i32 out = 0;

    for (int i = 0; what[i] != '\0'; i++)
    {
        out *= 10;

        switch (what[i])
        {
        case '-':
            negative = true;
            break;
        case '0':
            break;
        case '1':
            out += 1;
            break;
        case '2':
            out += 2;
            break;
        case '3':
            out += 3;
            break;
        case '4':
            out += 4;
            break;
        case '5':
            out += 5;
            break;
        case '6':
            out += 6;
            break;
        case '7':
            out += 7;
            break;
        case '8':
            out += 8;
            break;
        case '9':
            out += 9;
            break;
        }
    }

    if (negative)
    {
        return -1 * out;
    }
    else
    {
        return out;
    }
}

i32 s_to_i_FN_PTR_i8_MAPS_i32(i8 *what)
{
    return s_to_i_FN_str_MAPS_i32(what);
}

i32 s_to_i_FN_ARR_i8_MAPS_i32(i8 *what)
{
    return s_to_i_FN_str_MAPS_i32(what);
}

// Cheater's way out tbh
bool s_to_b_FN_str_MAPS_bool(str what)
{
    return (what[0] == 't') || (what[0] == 'T');
}

str b_to_s_FN_bool_MAPS_str(bool what)
{
    return (what ? "true" : "false");
}

f64 i_to_f_FN_i32_MAPS_f64(i32 what)
{
    return (f64)what;
}

i32 f_to_i_FN_f64_MAPS_i32(f64 what)
{
    return (i32)what;
}

i32 to_i32_FN_u8_MAPS_i32(u8 what)
{
    return what;
}

i32 to_i32_FN_i8_MAPS_i32(i8 what)
{
    return what;
}

i32 to_i32_FN_u16_MAPS_i32(u16 what)
{
    return what;
}

i32 to_i32_FN_i16_MAPS_i32(i16 what)
{
    return what;
}

i32 to_i32_FN_u32_MAPS_i32(u32 what)
{
    return what;
}

i32 to_i32_FN_u64_MAPS_i32(u64 what)
{
    return what;
}

i32 to_i32_FN_i64_MAPS_i32(i64 what)
{
    return what;
}

i32 to_i32_FN_u128_MAPS_i32(u128 what)
{
    return what;
}

i32 to_i32_FN_i128_MAPS_i32(i128 what)
{
    return what;
}

u8 to_u8_FN_i32_MAPS_u8(i32 what)
{
    return what;
}

i8 to_i8_FN_i32_MAPS_i8(i32 what)
{
    return what;
}

u16 to_u16_FN_i32_MAPS_u16(i32 what)
{
    return what;
}

i16 to_i16_FN_i32_MAPS_i16(i32 what)
{
    return what;
}

u32 to_u32_FN_i32_MAPS_u32(i32 what)
{
    return what;
}

u64 to_u64_FN_i32_MAPS_u64(i32 what)
{
    return what;
}

i64 to_i64_FN_i32_MAPS_i64(i32 what)
{
    return what;
}

u128 to_u128_FN_i32_MAPS_u128(i32 what)
{
    return what;
}

i128 to_i128_FN_i32_MAPS_i128(i32 what)
{
    return what;
}

f64 to_f64_FN_f32_MAPS_f64(f32 what)
{
    return what;
}

f64 to_f64_FN_f128_MAPS_f64(f128 what)
{
    return what;
}

i8 Get_FN_PTR_str_JOIN_u128_MAPS_i8(str *self, u128 index)
{
    return (*self)[index];
}

i8 Get_FN_str_JOIN_u128_MAPS_i8(str self, u128 index)
{
    return self[index];
}

i8 to_i8_FN_str_MAPS_i8(str what)
{
    return what[0];
}

// Absolute value casts
u8 to_u8_FN_i8_MAPS_u8(i8 what)
{
    if (what < 0)
    {
        return -what;
    }
    else
    {
        return what;
    }
}

u16 to_u16_FN_i16_MAPS_u16(i16 what)
{
    if (what < 0)
    {
        return -what;
    }
    else
    {
        return what;
    }
}

u64 to_u64_FN_i64_MAPS_u64(i64 what)
{
    if (what < 0)
    {
        return -what;
    }
    else
    {
        return what;
    }
}

u128 to_u128_FN_i128_MAPS_u128(i128 what)
{
    if (what < 0)
    {
        return -what;
    }
    else
    {
        return what;
    }
}

bool Eq_FN_str_JOIN_i8_MAPS_bool(str what, i8 against)
{
    return what[0] == against;
}

bool Eq_FN_i8_JOIN_str_MAPS_bool(i8 what, str against)
{
    return against[0] == what;
}
