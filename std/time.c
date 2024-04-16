#include "/usr/include/oak/std_oak_header.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>

struct ns
{
    struct timespec raw;
};

void New_FN_PTR_ns_MAPS_void(struct ns *self)
{
    memset(&self->raw, 0, sizeof(self->raw));
}

struct ns *Copy_FN_PTR_ns_JOIN_ns_MAPS_PTR_ns(struct ns *self, struct ns other)
{
    self->raw = other.raw;
    return self;
}

void now_FN_PTR_ns_MAPS_void(struct ns *self)
{
    // This compiles fine, even if VSCode flags it as an error.
    clock_gettime(CLOCK_REALTIME, &self->raw);
}

i128 count_FN_PTR_ns_JOIN_PTR_ns_MAPS_i128(struct ns *start, struct ns *end)
{
    return end->raw.tv_nsec - start->raw.tv_nsec;
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
