/*
let ns : struct
{
    __a, __b, __c, __d : u128,
}

let New(self : ^ns)
    ->void;
let Del(self : ^ns)->void;
let Copy(self : ^ns, other : ns)->void;

let count(to_count : ns)->i128;

let Sub(a : ns, b : ns)->ns;
let Add(a : ns, b : ns)->ns;
*/

#include <chrono>
#include "std_oak_header.hpp"

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
