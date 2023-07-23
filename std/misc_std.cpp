#include <cstdlib>

#include "time.h"
#include "std_oak_header.hpp"

i64 time()
{
    return time(NULL);
}

i32 rand(i32 low, i32 high)
{
    return (rand() % (high - low) + low);
}

void srand()
{
    srand((unsigned int)time(NULL));
}

void srand(i32 with)
{
    srand(with);
}
