/*
C version of the standard Oak string struct.
*/

#include "/usr/include/oak/std_oak_header.h"

/*
This must be kept up in order to not break interfacial files!
*/
struct string
{
    u128 capacity;       // The max value currently storable
    u128 current_length; // The current length of the string
    i8 *data;            // The underlying heap memory
};

// let New(self: ^string) -> void
void New_FN_PTR_string_MAPS_void(struct string* self);

// let Copy(self: ^string, from: str) -> string
struct string Copy_FN_PTR_string_JOIN_str_MAPS_string(
    struct string* self, str from);

// let Del(self: ^string) -> void
void Del_FN_PTR_string_MAPS_void(struct string* self);
