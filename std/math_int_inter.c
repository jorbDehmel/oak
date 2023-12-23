/*
Jordan Dehmel, 2023
jdehmel@outlook.com

For .o linking w/ Acorn STD package

Abandon all hope, ye who enter here
*/

#include "oak/std_oak_header.h"

// u8
u8 Add_FN_u8_JOIN_u8_MAPS_u8(u8 self, u8 other)
{
    return self + other;
}
u8 Sub_FN_u8_JOIN_u8_MAPS_u8(u8 self, u8 other)
{
    return self - other;
}
u8 Mult_FN_u8_JOIN_u8_MAPS_u8(u8 self, u8 other)
{
    return self * other;
}
u8 Div_FN_u8_JOIN_u8_MAPS_u8(u8 self, u8 other)
{
    return self / other;
}
u8 Mod_FN_u8_JOIN_u8_MAPS_u8(u8 self, u8 other)
{
    return self % other;
}
u8 Xor_FN_u8_JOIN_u8_MAPS_u8(u8 self, u8 other)
{
    return self ^ other;
}
bool Eq_FN_u8_JOIN_u8_MAPS_bool(u8 self, u8 other)
{
    return self == other;
}
bool Neq_FN_u8_JOIN_u8_MAPS_bool(u8 self, u8 other)
{
    return self != other;
}
bool Less_FN_u8_JOIN_u8_MAPS_bool(u8 self, u8 other)
{
    return self < other;
}
bool Great_FN_u8_JOIN_u8_MAPS_bool(u8 self, u8 other)
{
    return self > other;
}
bool Leq_FN_u8_JOIN_u8_MAPS_bool(u8 self, u8 other)
{
    return self <= other;
}
bool Greq_FN_u8_JOIN_u8_MAPS_bool(u8 self, u8 other)
{
    return self >= other;
}
u8 And_FN_u8_JOIN_u8_MAPS_u8(u8 self, u8 other)
{
    return self & other;
}
u8 Or_FN_u8_JOIN_u8_MAPS_u8(u8 self, u8 other)
{
    return self | other;
}
u8 Lbs_FN_u8_JOIN_u8_MAPS_u8(u8 self, u8 other)
{
    return self << other;
}
u8 Rbs_FN_u8_JOIN_u8_MAPS_u8(u8 self, u8 other)
{
    return self >> other;
}
void New_FN_PTR_u8_MAPS_void(u8 *self)
{
    (*self) = 0;
    return;
}
u8 Copy_FN_PTR_u8_JOIN_u8_MAPS_u8(u8 *self, u8 other)
{
    (*self) = other;
    return (*self);
}
u8 AddEq_FN_PTR_u8_JOIN_u8_MAPS_u8(u8 *self, u8 other)
{
    (*self) += other;
    return (*self);
}
u8 SubEq_FN_PTR_u8_JOIN_u8_MAPS_u8(u8 *self, u8 other)
{
    (*self) -= other;
    return (*self);
}
u8 MultEq_FN_PTR_u8_JOIN_u8_MAPS_u8(u8 *self, u8 other)
{
    (*self) *= other;
    return (*self);
}
u8 DivEq_FN_PTR_u8_JOIN_u8_MAPS_u8(u8 *self, u8 other)
{
    (*self) /= other;
    return (*self);
}
u8 ModEq_FN_PTR_u8_JOIN_u8_MAPS_u8(u8 *self, u8 other)
{
    (*self) %= other;
    return (*self);
}
u8 AndEq_FN_PTR_u8_JOIN_u8_MAPS_u8(u8 *self, u8 other)
{
    (*self) &= other;
    return (*self);
}
u8 OrEq_FN_PTR_u8_JOIN_u8_MAPS_u8(u8 *self, u8 other)
{
    (*self) |= other;
    return (*self);
}
u8 XorEq_FN_PTR_u8_JOIN_u8_MAPS_u8(u8 *self, u8 other)
{
    (*self) ^= other;
    return (*self);
}

// u16
u16 Add_FN_u16_JOIN_u16_MAPS_u16(u16 self, u16 other)
{
    return self + other;
}
u16 Sub_FN_u16_JOIN_u16_MAPS_u16(u16 self, u16 other)
{
    return self - other;
}
u16 Mult_FN_u16_JOIN_u16_MAPS_u16(u16 self, u16 other)
{
    return self * other;
}
u16 Div_FN_u16_JOIN_u16_MAPS_u16(u16 self, u16 other)
{
    return self / other;
}
u16 Mod_FN_u16_JOIN_u16_MAPS_u16(u16 self, u16 other)
{
    return self % other;
}
u16 Xor_FN_u16_JOIN_u16_MAPS_u16(u16 self, u16 other)
{
    return self ^ other;
}
bool Eq_FN_u16_JOIN_u16_MAPS_bool(u16 self, u16 other)
{
    return self == other;
}
bool Neq_FN_u16_JOIN_u16_MAPS_bool(u16 self, u16 other)
{
    return self != other;
}
bool Less_FN_u16_JOIN_u16_MAPS_bool(u16 self, u16 other)
{
    return self < other;
}
bool Great_FN_u16_JOIN_u16_MAPS_bool(u16 self, u16 other)
{
    return self > other;
}
bool Leq_FN_u16_JOIN_u16_MAPS_bool(u16 self, u16 other)
{
    return self <= other;
}
bool Greq_FN_u16_JOIN_u16_MAPS_bool(u16 self, u16 other)
{
    return self >= other;
}
u16 And_FN_u16_JOIN_u16_MAPS_u16(u16 self, u16 other)
{
    return self & other;
}
u16 Or_FN_u16_JOIN_u16_MAPS_u16(u16 self, u16 other)
{
    return self | other;
}
u16 Lbs_FN_u16_JOIN_u16_MAPS_u16(u16 self, u16 other)
{
    return self << other;
}
u16 Rbs_FN_u16_JOIN_u16_MAPS_u16(u16 self, u16 other)
{
    return self >> other;
}
void New_FN_PTR_u16_MAPS_void(u16 *self)
{
    (*self) = 0;
    return;
}
u16 Copy_FN_PTR_u16_JOIN_u16_MAPS_u16(u16 *self, u16 other)
{
    (*self) = other;
    return (*self);
}
u16 AddEq_FN_PTR_u16_JOIN_u16_MAPS_u16(u16 *self, u16 other)
{
    (*self) += other;
    return (*self);
}
u16 SubEq_FN_PTR_u16_JOIN_u16_MAPS_u16(u16 *self, u16 other)
{
    (*self) -= other;
    return (*self);
}
u16 MultEq_FN_PTR_u16_JOIN_u16_MAPS_u16(u16 *self, u16 other)
{
    (*self) *= other;
    return (*self);
}
u16 DivEq_FN_PTR_u16_JOIN_u16_MAPS_u16(u16 *self, u16 other)
{
    (*self) /= other;
    return (*self);
}
u16 ModEq_FN_PTR_u16_JOIN_u16_MAPS_u16(u16 *self, u16 other)
{
    (*self) %= other;
    return (*self);
}
u16 AndEq_FN_PTR_u16_JOIN_u16_MAPS_u16(u16 *self, u16 other)
{
    (*self) &= other;
    return (*self);
}
u16 OrEq_FN_PTR_u16_JOIN_u16_MAPS_u16(u16 *self, u16 other)
{
    (*self) |= other;
    return (*self);
}
u16 XorEq_FN_PTR_u16_JOIN_u16_MAPS_u16(u16 *self, u16 other)
{
    (*self) ^= other;
    return (*self);
}

// u32
u32 Add_FN_u32_JOIN_u32_MAPS_u32(u32 self, u32 other)
{
    return self + other;
}
u32 Sub_FN_u32_JOIN_u32_MAPS_u32(u32 self, u32 other)
{
    return self - other;
}
u32 Mult_FN_u32_JOIN_u32_MAPS_u32(u32 self, u32 other)
{
    return self * other;
}
u32 Div_FN_u32_JOIN_u32_MAPS_u32(u32 self, u32 other)
{
    return self / other;
}
u32 Mod_FN_u32_JOIN_u32_MAPS_u32(u32 self, u32 other)
{
    return self % other;
}
u32 Xor_FN_u32_JOIN_u32_MAPS_u32(u32 self, u32 other)
{
    return self ^ other;
}
bool Eq_FN_u32_JOIN_u32_MAPS_bool(u32 self, u32 other)
{
    return self == other;
}
bool Neq_FN_u32_JOIN_u32_MAPS_bool(u32 self, u32 other)
{
    return self != other;
}
bool Less_FN_u32_JOIN_u32_MAPS_bool(u32 self, u32 other)
{
    return self < other;
}
bool Great_FN_u32_JOIN_u32_MAPS_bool(u32 self, u32 other)
{
    return self > other;
}
bool Leq_FN_u32_JOIN_u32_MAPS_bool(u32 self, u32 other)
{
    return self <= other;
}
bool Greq_FN_u32_JOIN_u32_MAPS_bool(u32 self, u32 other)
{
    return self >= other;
}
u32 And_FN_u32_JOIN_u32_MAPS_u32(u32 self, u32 other)
{
    return self & other;
}
u32 Or_FN_u32_JOIN_u32_MAPS_u32(u32 self, u32 other)
{
    return self | other;
}
u32 Lbs_FN_u32_JOIN_u32_MAPS_u32(u32 self, u32 other)
{
    return self << other;
}
u32 Rbs_FN_u32_JOIN_u32_MAPS_u32(u32 self, u32 other)
{
    return self >> other;
}
void New_FN_PTR_u32_MAPS_void(u32 *self)
{
    (*self) = 0;
    return;
}
u32 Copy_FN_PTR_u32_JOIN_u32_MAPS_u32(u32 *self, u32 other)
{
    (*self) = other;
    return (*self);
}
u32 AddEq_FN_PTR_u32_JOIN_u32_MAPS_u32(u32 *self, u32 other)
{
    (*self) += other;
    return (*self);
}
u32 SubEq_FN_PTR_u32_JOIN_u32_MAPS_u32(u32 *self, u32 other)
{
    (*self) -= other;
    return (*self);
}
u32 MultEq_FN_PTR_u32_JOIN_u32_MAPS_u32(u32 *self, u32 other)
{
    (*self) *= other;
    return (*self);
}
u32 DivEq_FN_PTR_u32_JOIN_u32_MAPS_u32(u32 *self, u32 other)
{
    (*self) /= other;
    return (*self);
}
u32 ModEq_FN_PTR_u32_JOIN_u32_MAPS_u32(u32 *self, u32 other)
{
    (*self) %= other;
    return (*self);
}
u32 AndEq_FN_PTR_u32_JOIN_u32_MAPS_u32(u32 *self, u32 other)
{
    (*self) &= other;
    return (*self);
}
u32 OrEq_FN_PTR_u32_JOIN_u32_MAPS_u32(u32 *self, u32 other)
{
    (*self) |= other;
    return (*self);
}
u32 XorEq_FN_PTR_u32_JOIN_u32_MAPS_u32(u32 *self, u32 other)
{
    (*self) ^= other;
    return (*self);
}

// u64
u64 Add_FN_u64_JOIN_u64_MAPS_u64(u64 self, u64 other)
{
    return self + other;
}
u64 Sub_FN_u64_JOIN_u64_MAPS_u64(u64 self, u64 other)
{
    return self - other;
}
u64 Mult_FN_u64_JOIN_u64_MAPS_u64(u64 self, u64 other)
{
    return self * other;
}
u64 Div_FN_u64_JOIN_u64_MAPS_u64(u64 self, u64 other)
{
    return self / other;
}
u64 Mod_FN_u64_JOIN_u64_MAPS_u64(u64 self, u64 other)
{
    return self % other;
}
u64 Xor_FN_u64_JOIN_u64_MAPS_u64(u64 self, u64 other)
{
    return self ^ other;
}
bool Eq_FN_u64_JOIN_u64_MAPS_bool(u64 self, u64 other)
{
    return self == other;
}
bool Neq_FN_u64_JOIN_u64_MAPS_bool(u64 self, u64 other)
{
    return self != other;
}
bool Less_FN_u64_JOIN_u64_MAPS_bool(u64 self, u64 other)
{
    return self < other;
}
bool Great_FN_u64_JOIN_u64_MAPS_bool(u64 self, u64 other)
{
    return self > other;
}
bool Leq_FN_u64_JOIN_u64_MAPS_bool(u64 self, u64 other)
{
    return self <= other;
}
bool Greq_FN_u64_JOIN_u64_MAPS_bool(u64 self, u64 other)
{
    return self >= other;
}
u64 And_FN_u64_JOIN_u64_MAPS_u64(u64 self, u64 other)
{
    return self & other;
}
u64 Or_FN_u64_JOIN_u64_MAPS_u64(u64 self, u64 other)
{
    return self | other;
}
u64 Lbs_FN_u64_JOIN_u64_MAPS_u64(u64 self, u64 other)
{
    return self << other;
}
u64 Rbs_FN_u64_JOIN_u64_MAPS_u64(u64 self, u64 other)
{
    return self >> other;
}
void New_FN_PTR_u64_MAPS_void(u64 *self)
{
    (*self) = 0;
    return;
}
u64 Copy_FN_PTR_u64_JOIN_u64_MAPS_u64(u64 *self, u64 other)
{
    (*self) = other;
    return (*self);
}
u64 AddEq_FN_PTR_u64_JOIN_u64_MAPS_u64(u64 *self, u64 other)
{
    (*self) += other;
    return (*self);
}
u64 SubEq_FN_PTR_u64_JOIN_u64_MAPS_u64(u64 *self, u64 other)
{
    (*self) -= other;
    return (*self);
}
u64 MultEq_FN_PTR_u64_JOIN_u64_MAPS_u64(u64 *self, u64 other)
{
    (*self) *= other;
    return (*self);
}
u64 DivEq_FN_PTR_u64_JOIN_u64_MAPS_u64(u64 *self, u64 other)
{
    (*self) /= other;
    return (*self);
}
u64 ModEq_FN_PTR_u64_JOIN_u64_MAPS_u64(u64 *self, u64 other)
{
    (*self) %= other;
    return (*self);
}
u64 AndEq_FN_PTR_u64_JOIN_u64_MAPS_u64(u64 *self, u64 other)
{
    (*self) &= other;
    return (*self);
}
u64 OrEq_FN_PTR_u64_JOIN_u64_MAPS_u64(u64 *self, u64 other)
{
    (*self) |= other;
    return (*self);
}
u64 XorEq_FN_PTR_u64_JOIN_u64_MAPS_u64(u64 *self, u64 other)
{
    (*self) ^= other;
    return (*self);
}

// u128
u128 Add_FN_u128_JOIN_u128_MAPS_u128(u128 self, u128 other)
{
    return self + other;
}
u128 Sub_FN_u128_JOIN_u128_MAPS_u128(u128 self, u128 other)
{
    return self - other;
}
u128 Mult_FN_u128_JOIN_u128_MAPS_u128(u128 self, u128 other)
{
    return self * other;
}
u128 Div_FN_u128_JOIN_u128_MAPS_u128(u128 self, u128 other)
{
    return self / other;
}
u128 Mod_FN_u128_JOIN_u128_MAPS_u128(u128 self, u128 other)
{
    return self % other;
}
u128 Xor_FN_u128_JOIN_u128_MAPS_u128(u128 self, u128 other)
{
    return self ^ other;
}
bool Eq_FN_u128_JOIN_u128_MAPS_bool(u128 self, u128 other)
{
    return self == other;
}
bool Neq_FN_u128_JOIN_u128_MAPS_bool(u128 self, u128 other)
{
    return self != other;
}
bool Less_FN_u128_JOIN_u128_MAPS_bool(u128 self, u128 other)
{
    return self < other;
}
bool Great_FN_u128_JOIN_u128_MAPS_bool(u128 self, u128 other)
{
    return self > other;
}
bool Leq_FN_u128_JOIN_u128_MAPS_bool(u128 self, u128 other)
{
    return self <= other;
}
bool Greq_FN_u128_JOIN_u128_MAPS_bool(u128 self, u128 other)
{
    return self >= other;
}
u128 And_FN_u128_JOIN_u128_MAPS_u128(u128 self, u128 other)
{
    return self & other;
}
u128 Or_FN_u128_JOIN_u128_MAPS_u128(u128 self, u128 other)
{
    return self | other;
}
u128 Lbs_FN_u128_JOIN_u128_MAPS_u128(u128 self, u128 other)
{
    return self << other;
}
u128 Rbs_FN_u128_JOIN_u128_MAPS_u128(u128 self, u128 other)
{
    return self >> other;
}
void New_FN_PTR_u128_MAPS_void(u128 *self)
{
    (*self) = 0;
    return;
}
u128 Copy_FN_PTR_u128_JOIN_u128_MAPS_u128(u128 *self, u128 other)
{
    (*self) = other;
    return (*self);
}
u128 AddEq_FN_PTR_u128_JOIN_u128_MAPS_u128(u128 *self, u128 other)
{
    (*self) += other;
    return (*self);
}
u128 SubEq_FN_PTR_u128_JOIN_u128_MAPS_u128(u128 *self, u128 other)
{
    (*self) -= other;
    return (*self);
}
u128 MultEq_FN_PTR_u128_JOIN_u128_MAPS_u128(u128 *self, u128 other)
{
    (*self) *= other;
    return (*self);
}
u128 DivEq_FN_PTR_u128_JOIN_u128_MAPS_u128(u128 *self, u128 other)
{
    (*self) /= other;
    return (*self);
}
u128 ModEq_FN_PTR_u128_JOIN_u128_MAPS_u128(u128 *self, u128 other)
{
    (*self) %= other;
    return (*self);
}
u128 AndEq_FN_PTR_u128_JOIN_u128_MAPS_u128(u128 *self, u128 other)
{
    (*self) &= other;
    return (*self);
}
u128 OrEq_FN_PTR_u128_JOIN_u128_MAPS_u128(u128 *self, u128 other)
{
    (*self) |= other;
    return (*self);
}
u128 XorEq_FN_PTR_u128_JOIN_u128_MAPS_u128(u128 *self, u128 other)
{
    (*self) ^= other;
    return (*self);
}

// i8
i8 Add_FN_i8_JOIN_i8_MAPS_i8(i8 self, i8 other)
{
    return self + other;
}
i8 Sub_FN_i8_JOIN_i8_MAPS_i8(i8 self, i8 other)
{
    return self - other;
}
i8 Mult_FN_i8_JOIN_i8_MAPS_i8(i8 self, i8 other)
{
    return self * other;
}
i8 Div_FN_i8_JOIN_i8_MAPS_i8(i8 self, i8 other)
{
    return self / other;
}
i8 Mod_FN_i8_JOIN_i8_MAPS_i8(i8 self, i8 other)
{
    return self % other;
}
i8 Xor_FN_i8_JOIN_i8_MAPS_i8(i8 self, i8 other)
{
    return self ^ other;
}
bool Eq_FN_i8_JOIN_i8_MAPS_bool(i8 self, i8 other)
{
    return self == other;
}
bool Neq_FN_i8_JOIN_i8_MAPS_bool(i8 self, i8 other)
{
    return self != other;
}
bool Less_FN_i8_JOIN_i8_MAPS_bool(i8 self, i8 other)
{
    return self < other;
}
bool Great_FN_i8_JOIN_i8_MAPS_bool(i8 self, i8 other)
{
    return self > other;
}
bool Leq_FN_i8_JOIN_i8_MAPS_bool(i8 self, i8 other)
{
    return self <= other;
}
bool Greq_FN_i8_JOIN_i8_MAPS_bool(i8 self, i8 other)
{
    return self >= other;
}
i8 And_FN_i8_JOIN_i8_MAPS_i8(i8 self, i8 other)
{
    return self & other;
}
i8 Or_FN_i8_JOIN_i8_MAPS_i8(i8 self, i8 other)
{
    return self | other;
}
i8 Lbs_FN_i8_JOIN_i8_MAPS_i8(i8 self, i8 other)
{
    return self << other;
}
i8 Rbs_FN_i8_JOIN_i8_MAPS_i8(i8 self, i8 other)
{
    return self >> other;
}
void New_FN_PTR_i8_MAPS_void(i8 *self)
{
    (*self) = 0;
    return;
}
i8 Copy_FN_PTR_i8_JOIN_i8_MAPS_i8(i8 *self, i8 other)
{
    (*self) = other;
    return (*self);
}
i8 AddEq_FN_PTR_i8_JOIN_i8_MAPS_i8(i8 *self, i8 other)
{
    (*self) += other;
    return (*self);
}
i8 SubEq_FN_PTR_i8_JOIN_i8_MAPS_i8(i8 *self, i8 other)
{
    (*self) -= other;
    return (*self);
}
i8 MultEq_FN_PTR_i8_JOIN_i8_MAPS_i8(i8 *self, i8 other)
{
    (*self) *= other;
    return (*self);
}
i8 DivEq_FN_PTR_i8_JOIN_i8_MAPS_i8(i8 *self, i8 other)
{
    (*self) /= other;
    return (*self);
}
i8 ModEq_FN_PTR_i8_JOIN_i8_MAPS_i8(i8 *self, i8 other)
{
    (*self) %= other;
    return (*self);
}
i8 AndEq_FN_PTR_i8_JOIN_i8_MAPS_i8(i8 *self, i8 other)
{
    (*self) &= other;
    return (*self);
}
i8 OrEq_FN_PTR_i8_JOIN_i8_MAPS_i8(i8 *self, i8 other)
{
    (*self) |= other;
    return (*self);
}
i8 XorEq_FN_PTR_i8_JOIN_i8_MAPS_i8(i8 *self, i8 other)
{
    (*self) ^= other;
    return (*self);
}

// i16
i16 Add_FN_i16_JOIN_i16_MAPS_i16(i16 self, i16 other)
{
    return self + other;
}
i16 Sub_FN_i16_JOIN_i16_MAPS_i16(i16 self, i16 other)
{
    return self - other;
}
i16 Mult_FN_i16_JOIN_i16_MAPS_i16(i16 self, i16 other)
{
    return self * other;
}
i16 Div_FN_i16_JOIN_i16_MAPS_i16(i16 self, i16 other)
{
    return self / other;
}
i16 Mod_FN_i16_JOIN_i16_MAPS_i16(i16 self, i16 other)
{
    return self % other;
}
i16 Xor_FN_i16_JOIN_i16_MAPS_i16(i16 self, i16 other)
{
    return self ^ other;
}
bool Eq_FN_i16_JOIN_i16_MAPS_bool(i16 self, i16 other)
{
    return self == other;
}
bool Neq_FN_i16_JOIN_i16_MAPS_bool(i16 self, i16 other)
{
    return self != other;
}
bool Less_FN_i16_JOIN_i16_MAPS_bool(i16 self, i16 other)
{
    return self < other;
}
bool Great_FN_i16_JOIN_i16_MAPS_bool(i16 self, i16 other)
{
    return self > other;
}
bool Leq_FN_i16_JOIN_i16_MAPS_bool(i16 self, i16 other)
{
    return self <= other;
}
bool Greq_FN_i16_JOIN_i16_MAPS_bool(i16 self, i16 other)
{
    return self >= other;
}
i16 And_FN_i16_JOIN_i16_MAPS_bool(i16 self, i16 other)
{
    return self & other;
}
i16 Or_FN_i16_JOIN_i16_MAPS_bool(i16 self, i16 other)
{
    return self | other;
}
i16 Lbs_FN_i16_JOIN_i16_MAPS_bool(i16 self, i16 other)
{
    return self << other;
}
i16 Rbs_FN_i16_JOIN_i16_MAPS_bool(i16 self, i16 other)
{
    return self >> other;
}
void New_FN_PTR_i16_MAPS_void(i16 *self)
{
    (*self) = 0;
    return;
}
i16 Copy_FN_PTR_i16_JOIN_i16_MAPS_i16(i16 *self, i16 other)
{
    (*self) = other;
    return (*self);
}
i16 AddEq_FN_PTR_i16_JOIN_i16_MAPS_i16(i16 *self, i16 other)
{
    (*self) += other;
    return (*self);
}
i16 SubEq_FN_PTR_i16_JOIN_i16_MAPS_i16(i16 *self, i16 other)
{
    (*self) -= other;
    return (*self);
}
i16 MultEq_FN_PTR_i16_JOIN_i16_MAPS_i16(i16 *self, i16 other)
{
    (*self) *= other;
    return (*self);
}
i16 DivEq_FN_PTR_i16_JOIN_i16_MAPS_i16(i16 *self, i16 other)
{
    (*self) /= other;
    return (*self);
}
i16 ModEq_FN_PTR_i16_JOIN_i16_MAPS_i16(i16 *self, i16 other)
{
    (*self) %= other;
    return (*self);
}
i16 AndEq_FN_PTR_i16_JOIN_i16_MAPS_i16(i16 *self, i16 other)
{
    (*self) &= other;
    return (*self);
}
i16 OrEq_FN_PTR_i16_JOIN_i16_MAPS_i16(i16 *self, i16 other)
{
    (*self) |= other;
    return (*self);
}
i16 XorEq_FN_PTR_i16_JOIN_i16_MAPS_i16(i16 *self, i16 other)
{
    (*self) ^= other;
    return (*self);
}

// i32
i32 Add_FN_i32_JOIN_i32_MAPS_i32(i32 self, i32 other)
{
    return self + other;
}
i32 Sub_FN_i32_JOIN_i32_MAPS_i32(i32 self, i32 other)
{
    return self - other;
}
i32 Mult_FN_i32_JOIN_i32_MAPS_i32(i32 self, i32 other)
{
    return self * other;
}
i32 Div_FN_i32_JOIN_i32_MAPS_i32(i32 self, i32 other)
{
    return self / other;
}
i32 Mod_FN_i32_JOIN_i32_MAPS_i32(i32 self, i32 other)
{
    return self % other;
}
i32 Xor_FN_i32_JOIN_i32_MAPS_i32(i32 self, i32 other)
{
    return self ^ other;
}
bool Eq_FN_i32_JOIN_i32_MAPS_bool(i32 self, i32 other)
{
    return self == other;
}
bool Neq_FN_i32_JOIN_i32_MAPS_bool(i32 self, i32 other)
{
    return self != other;
}
bool Less_FN_i32_JOIN_i32_MAPS_bool(i32 self, i32 other)
{
    return self < other;
}
bool Great_FN_i32_JOIN_i32_MAPS_bool(i32 self, i32 other)
{
    return self > other;
}
bool Leq_FN_i32_JOIN_i32_MAPS_bool(i32 self, i32 other)
{
    return self <= other;
}
bool Greq_FN_i32_JOIN_i32_MAPS_bool(i32 self, i32 other)
{
    return self >= other;
}
i32 And_FN_i32_JOIN_i32_MAPS_i32(i32 self, i32 other)
{
    return self & other;
}
i32 Or_FN_i32_JOIN_i32_MAPS_i32(i32 self, i32 other)
{
    return self | other;
}
i32 Lbs_FN_i32_JOIN_i32_MAPS_i32(i32 self, i32 other)
{
    return self << other;
}
i32 Rbs_FN_i32_JOIN_i32_MAPS_i32(i32 self, i32 other)
{
    return self >> other;
}
void New_FN_PTR_i32_MAPS_void(i32 *self)
{
    (*self) = 0;
    return;
}
i32 Copy_FN_PTR_i32_JOIN_i32_MAPS_i32(i32 *self, i32 other)
{
    (*self) = other;
    return (*self);
}
i32 AddEq_FN_PTR_i32_JOIN_i32_MAPS_i32(i32 *self, i32 other)
{
    (*self) += other;
    return (*self);
}
i32 SubEq_FN_PTR_i32_JOIN_i32_MAPS_i32(i32 *self, i32 other)
{
    (*self) -= other;
    return (*self);
}
i32 MultEq_FN_PTR_i32_JOIN_i32_MAPS_i32(i32 *self, i32 other)
{
    (*self) *= other;
    return (*self);
}
i32 DivEq_FN_PTR_i32_JOIN_i32_MAPS_i32(i32 *self, i32 other)
{
    (*self) /= other;
    return (*self);
}
i32 ModEq_FN_PTR_i32_JOIN_i32_MAPS_i32(i32 *self, i32 other)
{
    (*self) %= other;
    return (*self);
}
i32 AndEq_FN_PTR_i32_JOIN_i32_MAPS_i32(i32 *self, i32 other)
{
    (*self) &= other;
    return (*self);
}
i32 OrEq_FN_PTR_i32_JOIN_i32_MAPS_i32(i32 *self, i32 other)
{
    (*self) |= other;
    return (*self);
}
i32 XorEq_FN_PTR_i32_JOIN_i32_MAPS_i32(i32 *self, i32 other)
{
    (*self) ^= other;
    return (*self);
}

// i64
i64 Add_FN_i64_JOIN_i64_MAPS_i64(i64 self, i64 other)
{
    return self + other;
}
i64 Sub_FN_i64_JOIN_i64_MAPS_i64(i64 self, i64 other)
{
    return self - other;
}
i64 Mult_FN_i64_JOIN_i64_MAPS_i64(i64 self, i64 other)
{
    return self * other;
}
i64 Div_FN_i64_JOIN_i64_MAPS_i64(i64 self, i64 other)
{
    return self / other;
}
i64 Mod_FN_i64_JOIN_i64_MAPS_i64(i64 self, i64 other)
{
    return self % other;
}
i64 Xor_FN_i64_JOIN_i64_MAPS_i64(i64 self, i64 other)
{
    return self ^ other;
}
bool Eq_FN_i64_JOIN_i64_MAPS_bool(i64 self, i64 other)
{
    return self == other;
}
bool Neq_FN_i64_JOIN_i64_MAPS_bool(i64 self, i64 other)
{
    return self != other;
}
bool Less_FN_i64_JOIN_i64_MAPS_bool(i64 self, i64 other)
{
    return self < other;
}
bool Great_FN_i64_JOIN_i64_MAPS_bool(i64 self, i64 other)
{
    return self > other;
}
bool Leq_FN_i64_JOIN_i64_MAPS_bool(i64 self, i64 other)
{
    return self <= other;
}
bool Greq_FN_i64_JOIN_i64_MAPS_bool(i64 self, i64 other)
{
    return self >= other;
}
i64 And_FN_i64_JOIN_i64_MAPS_i64(i64 self, i64 other)
{
    return self & other;
}
i64 Or_FN_i64_JOIN_i64_MAPS_i64(i64 self, i64 other)
{
    return self | other;
}
i64 Lbs_FN_i64_JOIN_i64_MAPS_i64(i64 self, i64 other)
{
    return self << other;
}
i64 Rbs_FN_i64_JOIN_i64_MAPS_i64(i64 self, i64 other)
{
    return self >> other;
}
void New_FN_PTR_i64_MAPS_void(i64 *self)
{
    (*self) = 0;
    return;
}
i64 Copy_FN_PTR_i64_JOIN_i64_MAPS_i64(i64 *self, i64 other)
{
    (*self) = other;
    return (*self);
}
i64 AddEq_FN_PTR_i64_JOIN_i64_MAPS_i64(i64 *self, i64 other)
{
    (*self) += other;
    return (*self);
}
i64 SubEq_FN_PTR_i64_JOIN_i64_MAPS_i64(i64 *self, i64 other)
{
    (*self) -= other;
    return (*self);
}
i64 MultEq_FN_PTR_i64_JOIN_i64_MAPS_i64(i64 *self, i64 other)
{
    (*self) *= other;
    return (*self);
}
i64 DivEq_FN_PTR_i64_JOIN_i64_MAPS_i64(i64 *self, i64 other)
{
    (*self) /= other;
    return (*self);
}
i64 ModEq_FN_PTR_i64_JOIN_i64_MAPS_i64(i64 *self, i64 other)
{
    (*self) %= other;
    return (*self);
}
i64 AndEq_FN_PTR_i64_JOIN_i64_MAPS_i64(i64 *self, i64 other)
{
    (*self) &= other;
    return (*self);
}
i64 OrEq_FN_PTR_i64_JOIN_i64_MAPS_i64(i64 *self, i64 other)
{
    (*self) |= other;
    return (*self);
}
i64 XorEq_FN_PTR_i64_JOIN_i64_MAPS_i64(i64 *self, i64 other)
{
    (*self) ^= other;
    return (*self);
}

// i128
i128 Add_FN_i128_JOIN_i128_MAPS_i128(i128 self, i128 other)
{
    return self + other;
}
i128 Sub_FN_i128_JOIN_i128_MAPS_i128(i128 self, i128 other)
{
    return self - other;
}
i128 Mult_FN_i128_JOIN_i128_MAPS_i128(i128 self, i128 other)
{
    return self * other;
}
i128 Div_FN_i128_JOIN_i128_MAPS_i128(i128 self, i128 other)
{
    return self / other;
}
i128 Mod_FN_i128_JOIN_i128_MAPS_i128(i128 self, i128 other)
{
    return self % other;
}
i128 Xor_FN_i128_JOIN_i128_MAPS_i128(i128 self, i128 other)
{
    return self ^ other;
}
bool Eq_FN_i128_JOIN_i128_MAPS_bool(i128 self, i128 other)
{
    return self == other;
}
bool Neq_FN_i128_JOIN_i128_MAPS_bool(i128 self, i128 other)
{
    return self != other;
}
bool Less_FN_i128_JOIN_i128_MAPS_bool(i128 self, i128 other)
{
    return self < other;
}
bool Great_FN_i128_JOIN_i128_MAPS_bool(i128 self, i128 other)
{
    return self > other;
}
bool Leq_FN_i128_JOIN_i128_MAPS_bool(i128 self, i128 other)
{
    return self <= other;
}
bool Greq_FN_i128_JOIN_i128_MAPS_bool(i128 self, i128 other)
{
    return self >= other;
}
i128 And_FN_i128_JOIN_i128_MAPS_i128(i128 self, i128 other)
{
    return self & other;
}
i128 Or_FN_i128_JOIN_i128_MAPS_i128(i128 self, i128 other)
{
    return self | other;
}
i128 Lbs_FN_i128_JOIN_i128_MAPS_i128(i128 self, i128 other)
{
    return self << other;
}
i128 Rbs_FN_i128_JOIN_i128_MAPS_i128(i128 self, i128 other)
{
    return self >> other;
}
void New_FN_PTR_i128_MAPS_void(i128 *self)
{
    (*self) = 0;
    return;
}
i128 Copy_FN_PTR_i128_JOIN_i128_MAPS_i128(i128 *self, i128 other)
{
    (*self) = other;
    return (*self);
}
i128 AddEq_FN_PTR_i128_JOIN_i128_MAPS_i128(i128 *self, i128 other)
{
    (*self) += other;
    return (*self);
}
i128 SubEq_FN_PTR_i128_JOIN_i128_MAPS_i128(i128 *self, i128 other)
{
    (*self) -= other;
    return (*self);
}
i128 MultEq_FN_PTR_i128_JOIN_i128_MAPS_i128(i128 *self, i128 other)
{
    (*self) *= other;
    return (*self);
}
i128 DivEq_FN_PTR_i128_JOIN_i128_MAPS_i128(i128 *self, i128 other)
{
    (*self) /= other;
    return (*self);
}
i128 ModEq_FN_PTR_i128_JOIN_i128_MAPS_i128(i128 *self, i128 other)
{
    (*self) %= other;
    return (*self);
}
i128 AndEq_FN_PTR_i128_JOIN_i128_MAPS_i128(i128 *self, i128 other)
{
    (*self) &= other;
    return (*self);
}
i128 OrEq_FN_PTR_i128_JOIN_i128_MAPS_i128(i128 *self, i128 other)
{
    (*self) |= other;
    return (*self);
}
i128 XorEq_FN_PTR_i128_JOIN_i128_MAPS_i128(i128 *self, i128 other)
{
    (*self) ^= other;
    return (*self);
}
