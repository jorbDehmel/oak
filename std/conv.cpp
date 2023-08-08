/*
Jordan Dehmel, 2023
jdehmel@outlook.com

For .o linking w/ Acorn STD package
*/

#include "std_oak_header.hpp"

i32 s_to_i(str what)
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

// Cheater's way out tbh
bool s_to_b(str what)
{
    return (what[0] == 't') || (what[0] == 'T');
}

str b_to_s(bool what)
{
    return (what ? "true" : "false");
}

f64 s_to_f(str what)
{
    throw -1;
}

f64 i_to_f(i32 what)
{
    return (f64)what;
}

i32 f_to_i(f64 what)
{
    return (i32)what;
}

i32 to_i32(u8 what)
{
    return what;
}

i32 to_i32(i8 what)
{
    return what;
}

i32 to_i32(u16 what)
{
    return what;
}

i32 to_i32(i16 what)
{
    return what;
}

i32 to_i32(u64 what)
{
    return what;
}

i32 to_i32(i64 what)
{
    return what;
}

i32 to_i32(u128 what)
{
    return what;
}

i32 to_i32(i128 what)
{
    return what;
}

u8 to_u8(i32 what)
{
    return what;
}

i8 to_i8(i32 what)
{
    return what;
}

u16 to_u16(i32 what)
{
    return what;
}

i16 to_i16(i32 what)
{
    return what;
}

u64 to_u64(i32 what)
{
    return what;
}

i64 to_i64(i32 what)
{
    return what;
}

u128 to_u128(i32 what)
{
    return what;
}

i128 to_i128(i32 what)
{
    return what;
}

i8 Get(str *self, u128 index)
{
    return (*self)[index];
}

i8 Get(str self, u128 index)
{
    return self[index];
}
