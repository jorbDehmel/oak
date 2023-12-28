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
    switch (what)
    {
    case 0:
        printf("NUL");
        break;
    case 1:
        printf("SOH");
        break;
    case 2:
        printf("STX");
        break;
    case 3:
        printf("ETX");
        break;
    case 4:
        printf("EOT");
        break;
    case 5:
        printf("ENQ");
        break;
    case 6:
        printf("ACK");
        break;
    case 7:
        printf("BEL");
        break;
    case 8:
        printf("BS ");
        break;
    case 9:
        printf("TAB");
        break;
    case 10:
        printf("LF ");
        break;
    case 11:
        printf("VT ");
        break;
    case 12:
        printf("FF ");
        break;
    case 13:
        printf("CR ");
        break;
    case 14:
        printf("SO ");
        break;
    case 15:
        printf("SI ");
        break;
    case 16:
        printf("DLE");
        break;
    case 17:
        printf("DC1");
        break;
    case 18:
        printf("DC2");
        break;
    case 19:
        printf("DC3");
        break;
    case 20:
        printf("DC4");
        break;
    case 21:
        printf("NAK");
        break;
    case 22:
        printf("SYN");
        break;
    case 23:
        printf("ETB");
        break;
    case 24:
        printf("CAN");
        break;
    case 25:
        printf("EM ");
        break;
    case 26:
        printf("SUB");
        break;
    case 27:
        printf("ESC");
        break;
    case 28:
        printf("FS ");
        break;
    case 29:
        printf("GS ");
        break;
    case 30:
        printf("RS ");
        break;
    case 31:
        printf("US ");
        break;
    case 32:
        printf("SPC");
        break;
    case 127:
        printf("DEL");
        break;
    case 255:
        printf("255");
        break;

    default:
        printf("%c  ", what);
        break;
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

    return;   
}
