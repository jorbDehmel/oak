#include <oak/std_oak_header.h>
#include <stdio.h>

void bit_print_internal_FN_PTR_void_JOIN_u128_MAPS_void(void *what, u128 size)
{
    printf("0b");

    for (unsigned long long i = 0; i < size * 8; i++)
    {
        if (((char *)what)[i / 8] & (1 << (7 - (i % 8))))
        {
            printf("1");
        }
        else
        {
            printf("0");
        }
    }

    return;
}
