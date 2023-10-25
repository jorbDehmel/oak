#include <chrono>
#include "oak/std_oak_header.hpp"

struct ns
{
    std::chrono::_V2::high_resolution_clock::time_point raw;
};

void New(ns *self)
{
    for (int i = 0; i < sizeof(self->raw); i++)
    {
        *(char *)(&self->raw) = '\0';
    }
}

void Copy(ns *self, ns other)
{
    self->raw = other.raw;
}

void now(ns *self)
{
    self->raw = std::chrono::high_resolution_clock::now();
}

i128 count(ns *start, ns *end)
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end->raw - start->raw).count();
}

i64 time()
{
    return time(NULL);
}

str c_time(i64 when)
{
    return ctime(&when);
}

str c_time(i32 when)
{
    i64 copy = when;
    return ctime(&copy);
}
