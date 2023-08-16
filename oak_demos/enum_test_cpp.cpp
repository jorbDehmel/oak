// This file for designing a valid way to do enums in Oak

#define match(what) switch (what.__info)
#define unit \
    struct   \
    {        \
    }

struct test
{
    int a, b, c, d;
};

/*
In Oak:

let test: struct
{
    i32 a, b, c, d,
}

let main_struct: enum
{
    a: test, // Takes a pre-existing struct
    b: unit,       // This is a unit struct
    c: unit,
    d: unit,
}

let main() -> i32
{
    let obj: main_struct;

    match (obj)
    {
        case a(data)
        {
            data.a += 2;
        }

        default
        {
            ;
        }
    }

    0
}

*/

////////// Internally generated //////////

struct main_struct
{
    enum
    {
        a,
        b,
        c,
        d,
    } __info;

    union
    {
        test a_data;
        unit b_data;
        unit c_data;
        unit d_data;
    } __data;
};

////////// End internally generated //////////

int main()
{
    main_struct obj;

    match(obj)
    {
    case main_struct::a:
        auto data = obj.__data.a_data;

        data.a += 2;

        break;
    default:
        break;
    }

    return 0;
}
