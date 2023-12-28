#include <oak/std_oak_header.h>
#include <stdio.h>

void bit_print_internal_FN_PTR_void_JOIN_u128_MAPS_void(void *what, u128 size)
{
    printf("0b");

    for (unsigned long long i = 0; i < size * 8; i++)
    {
        if (((unsigned char *)what)[i / 8] & (1 << (7 - (i % 8))))
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

void hex_print_internal_FN_PTR_void_JOIN_u128_MAPS_void(void *what, u128 size)
{
    printf("0x");

    // Iterate over bytes
    unsigned char temp;
    for (unsigned long long i = 0; i < size; i++)
    {
        // Print high-order nibble
        temp = ((unsigned char *)what)[i] >> 4;
        if (temp < 10)
        {
            temp += '0';
        }
        else
        {
            temp += 'A' - 10;
        }
        printf("%c", temp);

        // Print low-order nibble
        temp = ((unsigned char *)what)[i] & 15;
        if (temp < 10)
        {
            temp += '0';
        }
        else
        {
            temp += 'A' - 10;
        }
        printf("%c", temp);
    }

    return;
}

void print_char_ascii(u8 what)
{
    if (what < 33 || what == 127 || what == 255)
    {
        char temp = what >> 4;
        if (temp < 10)
        {
            temp += '0';
        }
        else
        {
            temp += 'A' - 10;
        }
        printf("%c", temp);

        // Print low-order nibble
        temp = what & 15;
        if (temp < 10)
        {
            temp += '0';
        }
        else
        {
            temp += 'A' - 10;
        }
        printf("%c", temp);
    }
    else
    {
        printf("%c ", what);
    }
}

void char_print_internal_FN_PTR_void_JOIN_u128_MAPS_void(void *what, u128 size)
{
    // Iterate over bytes
    for (unsigned long long i = 0; i < size; i++)
    {
        if ((i % 16 == 0) && i != 0)
        {
            printf("\n");
        }

        print_char_ascii(((char *)what)[i]);
        printf(" ");
    }

    for (int i = size; i % 16 != 0; i++)
    {
        printf("__ ");
    }

    return;
}

void hex_dump_internal_FN_PTR_void_JOIN_u128_MAPS_void(void *what, u128 size)
{
    // Iterate over bytes
    unsigned char temp;
    for (unsigned long long i = 0; i < size; i++)
    {
        if ((i % 16 == 0) && i != 0)
        {
            printf("\n");
        }

        // Print high-order nibble
        temp = ((unsigned char *)what)[i] >> 4;
        if (temp < 10)
        {
            temp += '0';
        }
        else
        {
            temp += 'A' - 10;
        }
        printf("%c", temp);

        // Print low-order nibble
        temp = ((unsigned char *)what)[i] & 15;
        if (temp < 10)
        {
            temp += '0';
        }
        else
        {
            temp += 'A' - 10;
        }
        printf("%c ", temp);
    }

    for (int i = size; i % 16 != 0; i++)
    {
        printf("__ ");
    }

    return;   
}
