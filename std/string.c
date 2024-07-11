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
