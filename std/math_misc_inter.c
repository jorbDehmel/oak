/*
Jordan Dehmel, 2023
jdehmel@outlook.com

For .o linking w/ Acorn STD package
*/

#include "/usr/include/oak/std_oak_header.h"

void New_FN_PTR_bool_MAPS_void(bool *self)
{
    *self = false;
    return;
}

bool Copy_FN_PTR_bool_JOIN_bool_MAPS_bool(bool *self, bool other)
{
    (*self) = other;
    return (*self);
}

bool Eq_FN_str_JOIN_str_MAPS_bool(str self, str other)
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

bool Neq_FN_str_JOIN_str_MAPS_bool(str self, str other)
{
    return !Eq_FN_str_JOIN_str_MAPS_bool(self, other);
}

bool Eq_FN_bool_JOIN_bool_MAPS_bool(bool self, bool other)
{
    return self == other;
}

bool Neq_FN_bool_JOIN_bool_MAPS_bool(bool self, bool other)
{
    return self != other;
}

bool Andd_FN_bool_JOIN_bool_MAPS_bool(bool self, bool other)
{
    return self && other;
}

bool Orr_FN_bool_JOIN_bool_MAPS_bool(bool self, bool other)
{
    return self || other;
}

bool Not_FN_bool_MAPS_bool(bool self)
{
    return !self;
}

bool AndEq_FN_PTR_bool_JOIN_bool_MAPS_bool(bool *self, bool other)
{
    *self &= other;
    return *self;
}

bool OrEq_FN_PTR_bool_JOIN_bool_MAPS_bool(bool *self, bool other)
{
    *self |= other;
    return *self;
}
