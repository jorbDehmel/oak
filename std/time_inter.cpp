#include <chrono>
#include "oak/std_oak_header.h"

struct ns
{
    std::chrono::_V2::high_resolution_clock::time_point raw;
};

extern "C"
{
    void New_FN_PTR_ns_MAPS_void(ns *self);
    void Copy_FN_PTR_ns_JOIN_ns_MAPS_void(ns *self, ns other);
    void now_FN_PTR_ns_MAPS_void(ns *self);
    i128 count_FN_PTR_ns_JOIN_PTR_ns_MAPS_i128(ns *start, ns *end);
    i64 time_FN_MAPS_i64(void);
    str c_time_FN_i64_MAPS_str(i64 when);
    str c_time_FN_i32_MAPS_str(i32 when);
}

void New_FN_PTR_ns_MAPS_void(ns *self)
{
    for (int i = 0; i < sizeof(self->raw); i++)
    {
        *(char *)(&self->raw) = '\0';
    }
}

void Copy_FN_PTR_ns_JOIN_ns_MAPS_void(ns *self, ns other)
{
    self->raw = other.raw;
}

void now_FN_PTR_ns_MAPS_void(ns *self)
{
    self->raw = std::chrono::high_resolution_clock::now();
}

i128 count_FN_PTR_ns_JOIN_PTR_ns_MAPS_i128(ns *start, ns *end)
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end->raw - start->raw).count();
}

i64 time_FN_MAPS_i64(void)
{
    return time(NULL);
}

str c_time_FN_i64_MAPS_str(i64 when)
{
    return ctime(&when);
}

str c_time_FN_i32_MAPS_str(i32 when)
{
    i64 copy = when;
    return ctime(&copy);
}
