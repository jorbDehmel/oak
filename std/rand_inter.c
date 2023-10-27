#include <stdlib.h>
#include <time.h>

#include "oak/std_oak_header.h"

i32 rand_FN_i32_JOIN_i32_MAPS_i32(i32 low, i32 high)
{
    return (rand() % (high - low) + low);
}

void seed_rand_FN_MAPS_void(void)
{
    srand((unsigned int)time(NULL));
}

void seed_rand_FN_i32_MAPS_void(i32 with)
{
    srand(with);
}
