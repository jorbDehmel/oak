// let string.c_str()->str;

#include "/usr/include/oak/std_oak_header.h"

struct string
{
    i8 *data;
    u128 size;
};

// let strlen(what: str) -> u128;

u128 strlen_FN_str_MAPS_u128(str what)
{
    return strlen(what);
}

str c_str_FN_PTR_string_MAPS_str(struct string *self)
{
    return self->data;
}
