/*
C interfacial file
*/

#include "oak/std/std_oak_header.h"
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
