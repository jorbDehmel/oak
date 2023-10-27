struct unit
{
    char __dummy;
};

struct a_data
{
    long long data;
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
        struct a_data a_data;
        struct unit b_data;
        struct unit c_data;

    } __data;
};

int main()
{
    struct test inst;
    switch (inst.__info)
    {
    case a:
        break;
    }

    return 0;
}
