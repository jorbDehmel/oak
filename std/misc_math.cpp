/*
Jordan Dehmel, 2023
jdehmel@outlook.com

For .o linking w/ Acorn STD package
*/

#include "std_oak_header.hpp"

void New(bool *self)
{
    *self = false;
    return;
}

bool Copy(bool *self, bool other)
{
    (*self) = other;
    return (*self);
}

i8 Get(str self, u128 index)
{
    return self[index];
}

bool Eq(str self, str other)
{
    int l = 0, r = 0;

    do
    {
        if (self[l] != other[r])
        {
            return false;
        }

        l++;
        r++;
    } while (self[l] != '\0' && other[r] != '\0');

    return true;
}

bool Neq(str self, str other)
{
    return !Eq(self, other);
}

bool Eq(bool self, bool other)
{
    return self == other;
}

bool Neq(bool self, bool other)
{
    return self != other;
}

bool Andd(bool self, bool other)
{
    return self && other;
}

bool Orr(bool self, bool other)
{
    return self || other;
}

bool Not(bool self)
{
    return !self;
}

bool AndEq(bool *self, bool other)
{
    *self &= other;
    return *self;
}

bool OrEq(bool *self, bool other)
{
    *self |= other;
    return *self;
}
