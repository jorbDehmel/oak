/*
C resources for the Oak cereal library.
*/

#include "oak/std_oak_header.h"

void cereal_hex_write_FN_PTR_void_JOIN_PTR_void_JOIN_u128_MAPS_void(
    void *into, void *from, u128 n)
{
    // Create hex representation
    const u128 buffer_size = 2 * n;
    u128 i;
    u8 value, *arr = from;
    char buffer[buffer_size];

    for (i = 0; i < n; i++)
    {
        // High order nybble first
        value = arr[i] >> 4;
        buffer[2 * i] = value < 10 ? '0' + value : ('A' - 10) + value;

        // Low order nybble next
        value = arr[i] & 15;
        buffer[2 * i + 1] = value < 10 ? '0' + value : ('A' - 10) + value;
    }

    // Copy in
    memcpy(into, buffer, buffer_size);
}

void cereal_hex_read_FN_PTR_void_JOIN_PTR_void_JOIN_u128_MAPS_void(
    void *from, void *into, u128 n)
{
    memset(into, 0, n);

    u128 i;
    u8 value, *out_arr = into, *inp_arr = from;

    for (i = 0; i < n; i++)
    {
        // High order nybble first
        value = inp_arr[2 * i];
        value = value <= '9' ? value - '0' : 10 + value - 'A';
        out_arr[i] |= value << 4;

        // Low order nybble next
        value = inp_arr[2 * i + 1];
        value = value <= '9' ? value - '0' : 10 + value - 'A';
        out_arr[i] |= value & 15;
    }
}
