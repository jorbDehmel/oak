/*
Jordan Dehmel, 2023
jdehmel@outlook.com

For .o linking w/ Acorn STD package

Abandon all hope, ye who enter here
*/

#include "oak/std_oak_header.hpp"

// u8
u8 Add(u8 self, u8 other)
{
    return self + other;
}
u8 Sub(u8 self, u8 other)
{
    return self - other;
}
u8 Mult(u8 self, u8 other)
{
    return self * other;
}
u8 Div(u8 self, u8 other)
{
    return self / other;
}
u8 Mod(u8 self, u8 other)
{
    return self % other;
}
u8 Xor(u8 self, u8 other)
{
    return self ^ other;
}
bool Eq(u8 self, u8 other)
{
    return self == other;
}
bool Neq(u8 self, u8 other)
{
    return self != other;
}
bool Less(u8 self, u8 other)
{
    return self < other;
}
bool Great(u8 self, u8 other)
{
    return self > other;
}
bool Leq(u8 self, u8 other)
{
    return self <= other;
}
bool Greq(u8 self, u8 other)
{
    return self >= other;
}
u8 And(u8 self, u8 other)
{
    return self & other;
}
u8 Or(u8 self, u8 other)
{
    return self | other;
}
u8 Lbs(u8 self, u8 other)
{
    return self << other;
}
u8 Rbs(u8 self, u8 other)
{
    return self >> other;
}
u8 Incr(u8 *self)
{
    (*self) += 1;
    return (*self);
}
u8 Decr(u8 *self)
{
    (*self) -= 1;
    return (*self);
}
void New(u8 *self)
{
    (*self) = 0;
    return;
}
u8 Copy(u8 *self, u8 other)
{
    (*self) = other;
    return (*self);
}
u8 AddEq(u8 *self, u8 other)
{
    (*self) += other;
    return (*self);
}
u8 SubEq(u8 *self, u8 other)
{
    (*self) -= other;
    return (*self);
}
u8 MultEq(u8 *self, u8 other)
{
    (*self) *= other;
    return (*self);
}
u8 DivEq(u8 *self, u8 other)
{
    (*self) /= other;
    return (*self);
}
u8 ModEq(u8 *self, u8 other)
{
    (*self) %= other;
    return (*self);
}
u8 AndEq(u8 *self, u8 other)
{
    (*self) &= other;
    return (*self);
}
u8 OrEq(u8 *self, u8 other)
{
    (*self) |= other;
    return (*self);
}
u8 XorEq(u8 *self, u8 other)
{
    (*self) ^= other;
    return (*self);
}

// u16
u16 Add(u16 self, u16 other)
{
    return self + other;
}
u16 Sub(u16 self, u16 other)
{
    return self - other;
}
u16 Mult(u16 self, u16 other)
{
    return self * other;
}
u16 Div(u16 self, u16 other)
{
    return self / other;
}
u16 Mod(u16 self, u16 other)
{
    return self % other;
}
u16 Xor(u16 self, u16 other)
{
    return self ^ other;
}
bool Eq(u16 self, u16 other)
{
    return self == other;
}
bool Neq(u16 self, u16 other)
{
    return self != other;
}
bool Less(u16 self, u16 other)
{
    return self < other;
}
bool Great(u16 self, u16 other)
{
    return self > other;
}
bool Leq(u16 self, u16 other)
{
    return self <= other;
}
bool Greq(u16 self, u16 other)
{
    return self >= other;
}
u16 And(u16 self, u16 other)
{
    return self & other;
}
u16 Or(u16 self, u16 other)
{
    return self | other;
}
u16 Lbs(u16 self, u16 other)
{
    return self << other;
}
u16 Rbs(u16 self, u16 other)
{
    return self >> other;
}
u16 Incr(u16 *self)
{
    (*self) += 1;
    return (*self);
}
u16 Decr(u16 *self)
{
    (*self) -= 1;
    return (*self);
}
void New(u16 *self)
{
    (*self) = 0;
    return;
}
u16 Copy(u16 *self, u16 other)
{
    (*self) = other;
    return (*self);
}
u16 AddEq(u16 *self, u16 other)
{
    (*self) += other;
    return (*self);
}
u16 SubEq(u16 *self, u16 other)
{
    (*self) -= other;
    return (*self);
}
u16 MultEq(u16 *self, u16 other)
{
    (*self) *= other;
    return (*self);
}
u16 DivEq(u16 *self, u16 other)
{
    (*self) /= other;
    return (*self);
}
u16 ModEq(u16 *self, u16 other)
{
    (*self) %= other;
    return (*self);
}
u16 AndEq(u16 *self, u16 other)
{
    (*self) &= other;
    return (*self);
}
u16 OrEq(u16 *self, u16 other)
{
    (*self) |= other;
    return (*self);
}
u16 XorEq(u16 *self, u16 other)
{
    (*self) ^= other;
    return (*self);
}

// u32
u32 Add(u32 self, u32 other)
{
    return self + other;
}
u32 Sub(u32 self, u32 other)
{
    return self - other;
}
u32 Mult(u32 self, u32 other)
{
    return self * other;
}
u32 Div(u32 self, u32 other)
{
    return self / other;
}
u32 Mod(u32 self, u32 other)
{
    return self % other;
}
u32 Xor(u32 self, u32 other)
{
    return self ^ other;
}
bool Eq(u32 self, u32 other)
{
    return self == other;
}
bool Neq(u32 self, u32 other)
{
    return self != other;
}
bool Less(u32 self, u32 other)
{
    return self < other;
}
bool Great(u32 self, u32 other)
{
    return self > other;
}
bool Leq(u32 self, u32 other)
{
    return self <= other;
}
bool Greq(u32 self, u32 other)
{
    return self >= other;
}
u32 And(u32 self, u32 other)
{
    return self & other;
}
u32 Or(u32 self, u32 other)
{
    return self | other;
}
u32 Lbs(u32 self, u32 other)
{
    return self << other;
}
u32 Rbs(u32 self, u32 other)
{
    return self >> other;
}
u32 Incr(u32 *self)
{
    (*self) += 1;
    return (*self);
}
u32 Decr(u32 *self)
{
    (*self) -= 1;
    return (*self);
}
void New(u32 *self)
{
    (*self) = 0;
    return;
}
u32 Copy(u32 *self, u32 other)
{
    (*self) = other;
    return (*self);
}
u32 AddEq(u32 *self, u32 other)
{
    (*self) += other;
    return (*self);
}
u32 SubEq(u32 *self, u32 other)
{
    (*self) -= other;
    return (*self);
}
u32 MultEq(u32 *self, u32 other)
{
    (*self) *= other;
    return (*self);
}
u32 DivEq(u32 *self, u32 other)
{
    (*self) /= other;
    return (*self);
}
u32 ModEq(u32 *self, u32 other)
{
    (*self) %= other;
    return (*self);
}
u32 AndEq(u32 *self, u32 other)
{
    (*self) &= other;
    return (*self);
}
u32 OrEq(u32 *self, u32 other)
{
    (*self) |= other;
    return (*self);
}
u32 XorEq(u32 *self, u32 other)
{
    (*self) ^= other;
    return (*self);
}

// u64
u64 Add(u64 self, u64 other)
{
    return self + other;
}
u64 Sub(u64 self, u64 other)
{
    return self - other;
}
u64 Mult(u64 self, u64 other)
{
    return self * other;
}
u64 Div(u64 self, u64 other)
{
    return self / other;
}
u64 Mod(u64 self, u64 other)
{
    return self % other;
}
u64 Xor(u64 self, u64 other)
{
    return self ^ other;
}
bool Eq(u64 self, u64 other)
{
    return self == other;
}
bool Neq(u64 self, u64 other)
{
    return self != other;
}
bool Less(u64 self, u64 other)
{
    return self < other;
}
bool Great(u64 self, u64 other)
{
    return self > other;
}
bool Leq(u64 self, u64 other)
{
    return self <= other;
}
bool Greq(u64 self, u64 other)
{
    return self >= other;
}
u64 And(u64 self, u64 other)
{
    return self & other;
}
u64 Or(u64 self, u64 other)
{
    return self | other;
}
u64 Lbs(u64 self, u64 other)
{
    return self << other;
}
u64 Rbs(u64 self, u64 other)
{
    return self >> other;
}
u64 Incr(u64 *self)
{
    (*self) += 1;
    return (*self);
}
u64 Decr(u64 *self)
{
    (*self) -= 1;
    return (*self);
}
void New(u64 *self)
{
    (*self) = 0;
    return;
}
u64 Copy(u64 *self, u64 other)
{
    (*self) = other;
    return (*self);
}
u64 AddEq(u64 *self, u64 other)
{
    (*self) += other;
    return (*self);
}
u64 SubEq(u64 *self, u64 other)
{
    (*self) -= other;
    return (*self);
}
u64 MultEq(u64 *self, u64 other)
{
    (*self) *= other;
    return (*self);
}
u64 DivEq(u64 *self, u64 other)
{
    (*self) /= other;
    return (*self);
}
u64 ModEq(u64 *self, u64 other)
{
    (*self) %= other;
    return (*self);
}
u64 AndEq(u64 *self, u64 other)
{
    (*self) &= other;
    return (*self);
}
u64 OrEq(u64 *self, u64 other)
{
    (*self) |= other;
    return (*self);
}
u64 XorEq(u64 *self, u64 other)
{
    (*self) ^= other;
    return (*self);
}

// u128
u128 Add(u128 self, u128 other)
{
    return self + other;
}
u128 Sub(u128 self, u128 other)
{
    return self - other;
}
u128 Mult(u128 self, u128 other)
{
    return self * other;
}
u128 Div(u128 self, u128 other)
{
    return self / other;
}
u128 Mod(u128 self, u128 other)
{
    return self % other;
}
u128 Xor(u128 self, u128 other)
{
    return self ^ other;
}
bool Eq(u128 self, u128 other)
{
    return self == other;
}
bool Neq(u128 self, u128 other)
{
    return self != other;
}
bool Less(u128 self, u128 other)
{
    return self < other;
}
bool Great(u128 self, u128 other)
{
    return self > other;
}
bool Leq(u128 self, u128 other)
{
    return self <= other;
}
bool Greq(u128 self, u128 other)
{
    return self >= other;
}
u128 And(u128 self, u128 other)
{
    return self & other;
}
u128 Or(u128 self, u128 other)
{
    return self | other;
}
u128 Lbs(u128 self, u128 other)
{
    return self << other;
}
u128 Rbs(u128 self, u128 other)
{
    return self >> other;
}
u128 Incr(u128 *self)
{
    (*self) += 1;
    return (*self);
}
u128 Decr(u128 *self)
{
    (*self) -= 1;
    return (*self);
}
void New(u128 *self)
{
    (*self) = 0;
    return;
}
u128 Copy(u128 *self, u128 other)
{
    (*self) = other;
    return (*self);
}
u128 AddEq(u128 *self, u128 other)
{
    (*self) += other;
    return (*self);
}
u128 SubEq(u128 *self, u128 other)
{
    (*self) -= other;
    return (*self);
}
u128 MultEq(u128 *self, u128 other)
{
    (*self) *= other;
    return (*self);
}
u128 DivEq(u128 *self, u128 other)
{
    (*self) /= other;
    return (*self);
}
u128 ModEq(u128 *self, u128 other)
{
    (*self) %= other;
    return (*self);
}
u128 AndEq(u128 *self, u128 other)
{
    (*self) &= other;
    return (*self);
}
u128 OrEq(u128 *self, u128 other)
{
    (*self) |= other;
    return (*self);
}
u128 XorEq(u128 *self, u128 other)
{
    (*self) ^= other;
    return (*self);
}

// i8
i8 Add(i8 self, i8 other)
{
    return self + other;
}
i8 Sub(i8 self, i8 other)
{
    return self - other;
}
i8 Mult(i8 self, i8 other)
{
    return self * other;
}
i8 Div(i8 self, i8 other)
{
    return self / other;
}
i8 Mod(i8 self, i8 other)
{
    return self % other;
}
i8 Xor(i8 self, i8 other)
{
    return self ^ other;
}
bool Eq(i8 self, i8 other)
{
    return self == other;
}
bool Neq(i8 self, i8 other)
{
    return self != other;
}
bool Less(i8 self, i8 other)
{
    return self < other;
}
bool Great(i8 self, i8 other)
{
    return self > other;
}
bool Leq(i8 self, i8 other)
{
    return self <= other;
}
bool Greq(i8 self, i8 other)
{
    return self >= other;
}
i8 And(i8 self, i8 other)
{
    return self & other;
}
i8 Or(i8 self, i8 other)
{
    return self | other;
}
i8 Lbs(i8 self, i8 other)
{
    return self << other;
}
i8 Rbs(i8 self, i8 other)
{
    return self >> other;
}
i8 Incr(i8 *self)
{
    (*self) += 1;
    return (*self);
}
i8 Decr(i8 *self)
{
    (*self) -= 1;
    return (*self);
}
void New(i8 *self)
{
    (*self) = 0;
    return;
}
i8 Copy(i8 *self, i8 other)
{
    (*self) = other;
    return (*self);
}
i8 AddEq(i8 *self, i8 other)
{
    (*self) += other;
    return (*self);
}
i8 SubEq(i8 *self, i8 other)
{
    (*self) -= other;
    return (*self);
}
i8 MultEq(i8 *self, i8 other)
{
    (*self) *= other;
    return (*self);
}
i8 DivEq(i8 *self, i8 other)
{
    (*self) /= other;
    return (*self);
}
i8 ModEq(i8 *self, i8 other)
{
    (*self) %= other;
    return (*self);
}
i8 AndEq(i8 *self, i8 other)
{
    (*self) &= other;
    return (*self);
}
i8 OrEq(i8 *self, i8 other)
{
    (*self) |= other;
    return (*self);
}
i8 XorEq(i8 *self, i8 other)
{
    (*self) ^= other;
    return (*self);
}

// i16
i16 Add(i16 self, i16 other)
{
    return self + other;
}
i16 Sub(i16 self, i16 other)
{
    return self - other;
}
i16 Mult(i16 self, i16 other)
{
    return self * other;
}
i16 Div(i16 self, i16 other)
{
    return self / other;
}
i16 Mod(i16 self, i16 other)
{
    return self % other;
}
i16 Xor(i16 self, i16 other)
{
    return self ^ other;
}
bool Eq(i16 self, i16 other)
{
    return self == other;
}
bool Neq(i16 self, i16 other)
{
    return self != other;
}
bool Less(i16 self, i16 other)
{
    return self < other;
}
bool Great(i16 self, i16 other)
{
    return self > other;
}
bool Leq(i16 self, i16 other)
{
    return self <= other;
}
bool Greq(i16 self, i16 other)
{
    return self >= other;
}
i16 And(i16 self, i16 other)
{
    return self & other;
}
i16 Or(i16 self, i16 other)
{
    return self | other;
}
i16 Lbs(i16 self, i16 other)
{
    return self << other;
}
i16 Rbs(i16 self, i16 other)
{
    return self >> other;
}
i16 Incr(i16 *self)
{
    (*self) += 1;
    return (*self);
}
i16 Decr(i16 *self)
{
    (*self) -= 1;
    return (*self);
}
void New(i16 *self)
{
    (*self) = 0;
    return;
}
i16 Copy(i16 *self, i16 other)
{
    (*self) = other;
    return (*self);
}
i16 AddEq(i16 *self, i16 other)
{
    (*self) += other;
    return (*self);
}
i16 SubEq(i16 *self, i16 other)
{
    (*self) -= other;
    return (*self);
}
i16 MultEq(i16 *self, i16 other)
{
    (*self) *= other;
    return (*self);
}
i16 DivEq(i16 *self, i16 other)
{
    (*self) /= other;
    return (*self);
}
i16 ModEq(i16 *self, i16 other)
{
    (*self) %= other;
    return (*self);
}
i16 AndEq(i16 *self, i16 other)
{
    (*self) &= other;
    return (*self);
}
i16 OrEq(i16 *self, i16 other)
{
    (*self) |= other;
    return (*self);
}
i16 XorEq(i16 *self, i16 other)
{
    (*self) ^= other;
    return (*self);
}

// i32
i32 Add(i32 self, i32 other)
{
    return self + other;
}
i32 Sub(i32 self, i32 other)
{
    return self - other;
}
i32 Mult(i32 self, i32 other)
{
    return self * other;
}
i32 Div(i32 self, i32 other)
{
    return self / other;
}
i32 Mod(i32 self, i32 other)
{
    return self % other;
}
i32 Xor(i32 self, i32 other)
{
    return self ^ other;
}
bool Eq(i32 self, i32 other)
{
    return self == other;
}
bool Neq(i32 self, i32 other)
{
    return self != other;
}
bool Less(i32 self, i32 other)
{
    return self < other;
}
bool Great(i32 self, i32 other)
{
    return self > other;
}
bool Leq(i32 self, i32 other)
{
    return self <= other;
}
bool Greq(i32 self, i32 other)
{
    return self >= other;
}
i32 And(i32 self, i32 other)
{
    return self & other;
}
i32 Or(i32 self, i32 other)
{
    return self | other;
}
i32 Lbs(i32 self, i32 other)
{
    return self << other;
}
i32 Rbs(i32 self, i32 other)
{
    return self >> other;
}
i32 Incr(i32 *self)
{
    (*self) += 1;
    return (*self);
}
i32 Decr(i32 *self)
{
    (*self) -= 1;
    return (*self);
}
void New(i32 *self)
{
    (*self) = 0;
    return;
}
i32 Copy(i32 *self, i32 other)
{
    (*self) = other;
    return (*self);
}
i32 AddEq(i32 *self, i32 other)
{
    (*self) += other;
    return (*self);
}
i32 SubEq(i32 *self, i32 other)
{
    (*self) -= other;
    return (*self);
}
i32 MultEq(i32 *self, i32 other)
{
    (*self) *= other;
    return (*self);
}
i32 DivEq(i32 *self, i32 other)
{
    (*self) /= other;
    return (*self);
}
i32 ModEq(i32 *self, i32 other)
{
    (*self) %= other;
    return (*self);
}
i32 AndEq(i32 *self, i32 other)
{
    (*self) &= other;
    return (*self);
}
i32 OrEq(i32 *self, i32 other)
{
    (*self) |= other;
    return (*self);
}
i32 XorEq(i32 *self, i32 other)
{
    (*self) ^= other;
    return (*self);
}

// i64
i64 Add(i64 self, i64 other)
{
    return self + other;
}
i64 Sub(i64 self, i64 other)
{
    return self - other;
}
i64 Mult(i64 self, i64 other)
{
    return self * other;
}
i64 Div(i64 self, i64 other)
{
    return self / other;
}
i64 Mod(i64 self, i64 other)
{
    return self % other;
}
i64 Xor(i64 self, i64 other)
{
    return self ^ other;
}
bool Eq(i64 self, i64 other)
{
    return self == other;
}
bool Neq(i64 self, i64 other)
{
    return self != other;
}
bool Less(i64 self, i64 other)
{
    return self < other;
}
bool Great(i64 self, i64 other)
{
    return self > other;
}
bool Leq(i64 self, i64 other)
{
    return self <= other;
}
bool Greq(i64 self, i64 other)
{
    return self >= other;
}
i64 And(i64 self, i64 other)
{
    return self & other;
}
i64 Or(i64 self, i64 other)
{
    return self | other;
}
i64 Lbs(i64 self, i64 other)
{
    return self << other;
}
i64 Rbs(i64 self, i64 other)
{
    return self >> other;
}
i64 Incr(i64 *self)
{
    (*self) += 1;
    return (*self);
}
i64 Decr(i64 *self)
{
    (*self) -= 1;
    return (*self);
}
void New(i64 *self)
{
    (*self) = 0;
    return;
}
i64 Copy(i64 *self, i64 other)
{
    (*self) = other;
    return (*self);
}
i64 AddEq(i64 *self, i64 other)
{
    (*self) += other;
    return (*self);
}
i64 SubEq(i64 *self, i64 other)
{
    (*self) -= other;
    return (*self);
}
i64 MultEq(i64 *self, i64 other)
{
    (*self) *= other;
    return (*self);
}
i64 DivEq(i64 *self, i64 other)
{
    (*self) /= other;
    return (*self);
}
i64 ModEq(i64 *self, i64 other)
{
    (*self) %= other;
    return (*self);
}
i64 AndEq(i64 *self, i64 other)
{
    (*self) &= other;
    return (*self);
}
i64 OrEq(i64 *self, i64 other)
{
    (*self) |= other;
    return (*self);
}
i64 XorEq(i64 *self, i64 other)
{
    (*self) ^= other;
    return (*self);
}

// i128
i128 Add(i128 self, i128 other)
{
    return self + other;
}
i128 Sub(i128 self, i128 other)
{
    return self - other;
}
i128 Mult(i128 self, i128 other)
{
    return self * other;
}
i128 Div(i128 self, i128 other)
{
    return self / other;
}
i128 Mod(i128 self, i128 other)
{
    return self % other;
}
i128 Xor(i128 self, i128 other)
{
    return self ^ other;
}
bool Eq(i128 self, i128 other)
{
    return self == other;
}
bool Neq(i128 self, i128 other)
{
    return self != other;
}
bool Less(i128 self, i128 other)
{
    return self < other;
}
bool Great(i128 self, i128 other)
{
    return self > other;
}
bool Leq(i128 self, i128 other)
{
    return self <= other;
}
bool Greq(i128 self, i128 other)
{
    return self >= other;
}
i128 And(i128 self, i128 other)
{
    return self & other;
}
i128 Or(i128 self, i128 other)
{
    return self | other;
}
i128 Lbs(i128 self, i128 other)
{
    return self << other;
}
i128 Rbs(i128 self, i128 other)
{
    return self >> other;
}
i128 Incr(i128 *self)
{
    (*self) += 1;
    return (*self);
}
i128 Decr(i128 *self)
{
    (*self) -= 1;
    return (*self);
}
void New(i128 *self)
{
    (*self) = 0;
    return;
}
i128 Copy(i128 *self, i128 other)
{
    (*self) = other;
    return (*self);
}
i128 AddEq(i128 *self, i128 other)
{
    (*self) += other;
    return (*self);
}
i128 SubEq(i128 *self, i128 other)
{
    (*self) -= other;
    return (*self);
}
i128 MultEq(i128 *self, i128 other)
{
    (*self) *= other;
    return (*self);
}
i128 DivEq(i128 *self, i128 other)
{
    (*self) /= other;
    return (*self);
}
i128 ModEq(i128 *self, i128 other)
{
    (*self) %= other;
    return (*self);
}
i128 AndEq(i128 *self, i128 other)
{
    (*self) &= other;
    return (*self);
}
i128 OrEq(i128 *self, i128 other)
{
    (*self) |= other;
    return (*self);
}
i128 XorEq(i128 *self, i128 other)
{
    (*self) ^= other;
    return (*self);
}