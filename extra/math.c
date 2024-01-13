// link!("extra/math.o");

// // Returns a^b
// let pow(a: f64, b: f64) -> f64;

// // Returns e^a
// let exp(a: f64) -> f64;

#include "/usr/include/oak/std_oak_header.h"
#include <math.h>

f64 extra_pow_FN_f64_JOIN_f64_MAPS_f64(f64 a, f64 b)
{
    return pow(a, b);
}

f64 extra_exp_FN_f64_MAPS_f64(f64 a)
{
    return exp(a);
}
