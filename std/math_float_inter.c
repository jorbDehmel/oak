/*
Jordan Dehmel, 2023
jdehmel@outlook.com

For .o linking w/ Acorn STD package
*/

#include "oak/std_oak_header.h"

// Floats
f32 Add_FN_f32_JOIN_f32_MAPS_f32(f32 self, f32 other)
{
    return self + other;
}
f32 Sub_FN_f32_JOIN_f32_MAPS_f32(f32 self, f32 other)
{
    return self - other;
}
f32 Mult_FN_f32_JOIN_f32_MAPS_f32(f32 self, f32 other)
{
    return self * other;
}
f32 Div_FN_f32_JOIN_f32_MAPS_f32(f32 self, f32 other)
{
    return self / other;
}
bool Eq_FN_f32_JOIN_f32_MAPS_bool(f32 self, f32 other)
{
    return self == other;
}
bool Neq_FN_f32_JOIN_f32_MAPS_bool(f32 self, f32 other)
{
    return self != other;
}
bool Less_FN_f32_JOIN_f32_MAPS_bool(f32 self, f32 other)
{
    return self < other;
}
bool Great_FN_f32_JOIN_f32_MAPS_bool(f32 self, f32 other)
{
    return self > other;
}
bool Leq_FN_f32_JOIN_f32_MAPS_bool(f32 self, f32 other)
{
    return self <= other;
}
bool Greq_FN_f32_JOIN_f32_MAPS_bool(f32 self, f32 other)
{
    return self >= other;
}
void New_FN_PTR_f32_MAPS_void(f32 *what)
{
    *what = 0;
    return;
}
f32 Copy_FN_PTR_f32_JOIN_f32_MAPS_f32(f32 *self, f32 other)
{
    *self = other;
    return *self;
}
f32 AddEq_FN_PTR_f32_JOIN_f32_MAPS_f32(f32 *self, f32 other)
{
    *self += other;
    return *self;
}
f32 SubEq_FN_PTR_f32_JOIN_f32_MAPS_f32(f32 *self, f32 other)
{
    *self -= other;
    return *self;
}
f32 MultEq_FN_PTR_f32_JOIN_f32_MAPS_f32(f32 *self, f32 other)
{
    *self *= other;
    return *self;
}
f32 DivEq_FN_PTR_f32_JOIN_f32_MAPS_f32(f32 *self, f32 other)
{
    *self /= other;
    return *self;
}

// Doubles
f64 Add_FN_f64_JOIN_f64_MAPS_f64(f64 self, f64 other)
{
    return self + other;
}
f64 Sub_FN_f64_JOIN_f64_MAPS_f64(f64 self, f64 other)
{
    return self - other;
}
f64 Mult_FN_f64_JOIN_f64_MAPS_f64(f64 self, f64 other)
{
    return self * other;
}
f64 Div_FN_f64_JOIN_f64_MAPS_f64(f64 self, f64 other)
{
    return self / other;
}
bool Eq_FN_f64_JOIN_f64_MAPS_bool(f64 self, f64 other)
{
    return self == other;
}
bool Neq_FN_f64_JOIN_f64_MAPS_bool(f64 self, f64 other)
{
    return self != other;
}
bool Less_FN_f64_JOIN_f64_MAPS_bool(f64 self, f64 other)
{
    return self < other;
}
bool Great_FN_f64_JOIN_f64_MAPS_bool(f64 self, f64 other)
{
    return self > other;
}
bool Leq_FN_f64_JOIN_f64_MAPS_bool(f64 self, f64 other)
{
    return self <= other;
}
bool Greq_FN_f64_JOIN_f64_MAPS_bool(f64 self, f64 other)
{
    return self >= other;
}
void New_FN_PTR_f64_MAPS_void(f64 *what)
{
    *what = 0;
    return;
}
f64 Copy_FN_PTR_f64_JOIN_f64_MAPS_f64(f64 *self, f64 other)
{
    *self = other;
    return *self;
}
f64 AddEq_FN_PTR_f64_JOIN_f64_MAPS_f64(f64 *self, f64 other)
{
    *self += other;
    return *self;
}
f64 SubEq_FN_PTR_f64_JOIN_f64_MAPS_f64(f64 *self, f64 other)
{
    *self -= other;
    return *self;
}
f64 MultEq_FN_PTR_f64_JOIN_f64_MAPS_f64(f64 *self, f64 other)
{
    *self *= other;
    return *self;
}
f64 DivEq_FN_PTR_f64_JOIN_f64_MAPS_f64(f64 *self, f64 other)
{
    *self /= other;
    return *self;
}

// Long doubles / Quads
f128 Add_FN_f128_JOIN_f128_MAPS_f128(f128 self, f128 other)
{
    return self + other;
}
f128 Sub_FN_f128_JOIN_f128_MAPS_f128(f128 self, f128 other)
{
    return self - other;
}
f128 Mult_FN_f128_JOIN_f128_MAPS_f128(f128 self, f128 other)
{
    return self * other;
}
f128 Div_FN_f128_JOIN_f128_MAPS_f128(f128 self, f128 other)
{
    return self / other;
}
bool Eq_FN_f128_JOIN_f128_MAPS_bool(f128 self, f128 other)
{
    return self == other;
}
bool Neq_FN_f128_JOIN_f128_MAPS_bool(f128 self, f128 other)
{
    return self != other;
}
bool Less_FN_f128_JOIN_f128_MAPS_bool(f128 self, f128 other)
{
    return self < other;
}
bool Great_FN_f128_JOIN_f128_MAPS_bool(f128 self, f128 other)
{
    return self > other;
}
bool Leq_FN_f128_JOIN_f128_MAPS_bool(f128 self, f128 other)
{
    return self <= other;
}
bool Greq_FN_f128_JOIN_f128_MAPS_bool(f128 self, f128 other)
{
    return self >= other;
}
void New_FN_PTR_f128_MAPS_void(f128 *what)
{
    *what = 0;
    return;
}
f128 Copy_FN_PTR_f128_JOIN_f128_MAPS_f128(f128 *self, f128 other)
{
    *self = other;
    return *self;
}
f128 AddEq_FN_PTR_f128_JOIN_f128_MAPS_f128(f128 *self, f128 other)
{
    *self += other;
    return *self;
}
f128 SubEq_FN_PTR_f128_JOIN_f128_MAPS_f128(f128 *self, f128 other)
{
    *self -= other;
    return *self;
}
f128 MultEq_FN_PTR_f128_JOIN_f128_MAPS_f128(f128 *self, f128 other)
{
    *self *= other;
    return *self;
}
f128 DivEq_FN_PTR_f128_JOIN_f128_MAPS_f128(f128 *self, f128 other)
{
    *self /= other;
    return *self;
}
