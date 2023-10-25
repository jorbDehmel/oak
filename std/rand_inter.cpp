#include <cstdlib>
#include <ctime>

#include "oak/std_oak_header.hpp"

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
