/*
C interfacial file
*/

#include "std_oak_header.h"
#include <math.h>

// pow(base: f64, exp: f64) -> f64
f64 pow_FN_f64_JOIN_f64_MAPS_f64(f64 base, f64 exp) {
  return pow(base, exp);
}

// exp(power: f64) -> f64
f64 exp_FN_f64_MAPS_f64(f64 power) {
  return exp(power);
}

// sqrt(x: f64) -> f64
f64 sqrt_FN_f64_MAPS_f64(f64 x) {
  return sqrt(x);
}

// sin(rad: f64) -> f64
f64 sin_FN_f64_MAPS_f64(f64 rad) {
  return sin(rad);
}

// cos(rad: f64) -> f64
f64 cos_FN_f64_MAPS_f64(f64 rad) {
  return cos(rad);
}

// tan(rad: f64) -> f64
f64 tan_FN_f64_MAPS_f64(f64 rad) {
  return tan(rad);
}

// sinh(x: f64) -> f64
f64 sinh_FN_f64_MAPS_f64(f64 x) {
  return sinh(x);
}

// cosh(x: f64) -> f64
f64 cosh_FN_f64_MAPS_f64(f64 x) {
  return cosh(x);
}

// tanh(x: f64) -> f64
f64 tanh_FN_f64_MAPS_f64(f64 x) {
  return tanh(x);
}

// log(x: f64) -> f64
f64 log_FN_f64_MAPS_f64(f64 x) {
  return log(x);
}

// log2(x: f64) -> f64
f64 log2_FN_f64_MAPS_f64(f64 x) {
  return log2(x);
}

// log10(x: f64) -> f64
f64 log10_FN_f64_MAPS_f64(f64 x) {
  return log10(x);
}

// cbrt(x: f64) -> f64
f64 cbrt_FN_f64_MAPS_f64(f64 x) {
  return cbrt(x);
}

// hypot(a: f64, b: f64) -> f64
f64 hypot_FN_f64_JOIN_f64_MAPS_f64(f64 a, f64 b) {
  return hypot(a, b);
}

// ceil(x: f64) -> f64
f64 ceil_FN_f64_MAPS_f64(f64 x) {
  return ceil(x);
}

// floor(x: f64) -> f64
f64 floor_FN_f64_MAPS_f64(f64 x) {
  return floor(x);
}

// round(x: f64) -> f64
f64 round_FN_f64_MAPS_f64(f64 x) {
  return round(x);
}

// round(x: f64, decimal_places: uint) -> f64
f64 round_FN_f64_JOIN_uint_MAPS_f64(f64 x,
                                    uint decimal_places) {
  return (f64)round(x * pow(10, decimal_places)) /
         (f64)pow(10, decimal_places);
}

// min(l: f64, r: f64) -> f64
f64 min_FN_f64_JOIN_f64_MAPS_f64(f64 l, f64 r) {
  return fmin(l, r);
}

// max(l: f64, r: f64) -> f64
f64 max_FN_f64_JOIN_f64_MAPS_f64(f64 l, f64 r) {
  return fmax(l, r);
}

// abs(x: f64) -> f64
f64 abs_FN_f64_MAPS_f64(f64 x) {
  return fabs(x);
}
