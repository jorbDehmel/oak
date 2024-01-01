#include <oak/std_oak_header.h>

bool Eq_FN_PTR_void_JOIN_u128_MAPS_bool(void *l, u128 r)
{
    if (r == 0)
    {
        return l == 0;
    }
    return false;
}

bool Eq_FN_u128_JOIN_PTR_void_MAPS_bool(u128 l, void *r)
{
    if (l == 0)
    {
        return r == 0;
    }
    return false;
}

bool Neq_FN_PTR_void_JOIN_u128_MAPS_bool(void *l, u128 r)
{
    if (r != 0)
    {
        return true;
    }
    return l != 0;
}

bool Neq_FN_u128_JOIN_PTR_void_MAPS_bool(u128 l, void *r)
{
    if (l != 0)
    {
        return true;
    }
    return r != 0;
}

bool Eq_FN_PTR_void_JOIN_PTR_void_MAPS_bool(void *l, void *r)
{
    return l == r;
}

bool Neq_FN_PTR_void_JOIN_PTR_void_MAPS_bool(void *l, void *r)
{
    return l != r;
}

bool is_null_FN_PTR_void_MAPS_bool(void *what)
{
    return what == 0;
}

bool is_not_null_FN_PTR_void_MAPS_bool(void *what)
{
    return what != 0;
}
