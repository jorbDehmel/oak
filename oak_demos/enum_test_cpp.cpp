#include "std_oak_header.hpp"

// Struct definitions
struct a_data
{
    i128 data;
};

struct unit
{
};

// Enumeration definitions
struct test
{
    enum
    {
        a,
        b,
        c,

    } __info;
    union
    {
        a_data a_data;
        unit b_data;
        unit c_data;

    } __data;
};

int main()
{
    test obj;

    /*
    match (obj)
    {
        case a(data)
        {
            ;
        }

        case b()
        {
            ;
        }

        default
        {
            ;
        }
    }
    */

    if (obj.__info == test::a)
    {
        auto *data = &obj.__data.a_data;
    }
    else if (obj.__info == test::b)
    {
        // No captured variable here (empty capture parenthesis)
    }
    else
    {
        // No captured variable here (cannot capture from default)
    }

    return 0;
}
