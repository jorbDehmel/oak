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

void seed_rand()
{
    srand((unsigned int)time(NULL));
}

void seed_rand(i32 with)
{
    srand(with);
}

str c_time(i64 when)
{
    return ctime(&when);
}
