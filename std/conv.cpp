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
