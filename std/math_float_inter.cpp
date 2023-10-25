/*
Jordan Dehmel, 2023
jdehmel@outlook.com

For .o linking w/ Acorn STD package
*/

#include "oak/std_oak_header.hpp"

// Floats
f32 Add(f32 self, f32 other)
{
    return self + other;
}
f32 Sub(f32 self, f32 other)
{
    return self - other;
}
f32 Mult(f32 self, f32 other)
{
    return self * other;
}
f32 Div(f32 self, f32 other)
{
    return self / other;
}
bool Eq(f32 self, f32 other)
{
    return self == other;
}
bool Neq(f32 self, f32 other)
{
    return self != other;
}
bool Less(f32 self, f32 other)
{
    return self < other;
}
bool Great(f32 self, f32 other)
{
    return self > other;
}
bool Leq(f32 self, f32 other)
{
    return self <= other;
}
bool Greq(f32 self, f32 other)
{
    return self >= other;
}
void New(f32 *what)
{
    *what = 0;
    return;
}
f32 Copy(f32 *self, f32 other)
{
    *self = other;
    return *self;
}
f32 AddEq(f32 *self, f32 other)
{
    *self += other;
    return *self;
}
f32 SubEq(f32 *self, f32 other)
{
    *self -= other;
    return *self;
}
f32 MultEq(f32 *self, f32 other)
{
    *self *= other;
    return *self;
}
f32 DivEq(f32 *self, f32 other)
{
    *self /= other;
    return *self;
}
f32 Incr(f32 *self)
{
    *self += 1;
    return *self;
}
f32 Decr(f32 *self)
{
    *self -= 1;
    return *self;
}

// Doubles
f64 Add(f64 self, f64 other)
{
    return self + other;
}
f64 Sub(f64 self, f64 other)
{
    return self - other;
}
f64 Mult(f64 self, f64 other)
{
    return self * other;
}
f64 Div(f64 self, f64 other)
{
    return self / other;
}
bool Eq(f64 self, f64 other)
{
    return self == other;
}
bool Neq(f64 self, f64 other)
{
    return self != other;
}
bool Less(f64 self, f64 other)
{
    return self < other;
}
bool Great(f64 self, f64 other)
{
    return self > other;
}
bool Leq(f64 self, f64 other)
{
    return self <= other;
}
bool Greq(f64 self, f64 other)
{
    return self >= other;
}
void New(f64 *what)
{
    *what = 0;
    return;
}
f64 Copy(f64 *self, f64 other)
{
    *self = other;
    return *self;
}
f64 AddEq(f64 *self, f64 other)
{
    *self += other;
    return *self;
}
f64 SubEq(f64 *self, f64 other)
{
    *self -= other;
    return *self;
}
f64 MultEq(f64 *self, f64 other)
{
    *self *= other;
    return *self;
}
f64 DivEq(f64 *self, f64 other)
{
    *self /= other;
    return *self;
}
f64 Incr(f64 *self)
{
    *self += 1;
    return *self;
}
f64 Decr(f64 *self)
{
    *self -= 1;
    return *self;
}

// Long doubles / Quads
f128 Add(f128 self, f128 other)
{
    return self + other;
}
f128 Sub(f128 self, f128 other)
{
    return self - other;
}
f128 Mult(f128 self, f128 other)
{
    return self * other;
}
f128 Div(f128 self, f128 other)
{
    return self / other;
}
bool Eq(f128 self, f128 other)
{
    return self == other;
}
bool Neq(f128 self, f128 other)
{
    return self != other;
}
bool Less(f128 self, f128 other)
{
    return self < other;
}
bool Great(f128 self, f128 other)
{
    return self > other;
}
bool Leq(f128 self, f128 other)
{
    return self <= other;
}
bool Greq(f128 self, f128 other)
{
    return self >= other;
}
void New(f128 *what)
{
    *what = 0;
    return;
}
f128 Copy(f128 *self, f128 other)
{
    *self = other;
    return *self;
}
f128 AddEq(f128 *self, f128 other)
{
    *self += other;
    return *self;
}
f128 SubEq(f128 *self, f128 other)
{
    *self -= other;
    return *self;
}
f128 MultEq(f128 *self, f128 other)
{
    *self *= other;
    return *self;
}
f128 DivEq(f128 *self, f128 other)
{
    *self /= other;
    return *self;
}
f128 Incr(f128 *self)
{
    *self += 1;
    return *self;
}
f128 Decr(f128 *self)
{
    *self -= 1;
    return *self;
}
