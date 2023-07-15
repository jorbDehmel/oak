/*
Jordan Dehmel, 2023
jdehmel@outlook.com

For .o linking w/ Acorn STD package
*/

// Signed integer types
typedef char i8;
typedef short int i16;
typedef int i32;
typedef long int i64;
typedef long long int i128;

// Unsigned integer types
typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long int u64;
typedef unsigned long long int u128;

// Floating point types
typedef float f32;
typedef double f64;
typedef long double f128;

// Misc
typedef const char *const str;

///////////////////////////////////////////////

bool Copy(bool *self, bool other)
{
    (*self) = other;
    return (*self);
}

i8 Get(str self, u128 index)
{
    return self[index];
}

bool Eq(str self, str other)
{
    int l = 0, r = 0;

    do
    {
        if (self[l] != other[r])
        {
            return false;
        }

        l++;
        r++;
    } while (self[l] != '\0' && other[r] != '\0');

    return true;
}

bool Neq(str self, str other)
{
    return !Eq(self, other);
}
