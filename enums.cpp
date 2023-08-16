#include "enums.hpp"

/*
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
*/

/*
struct NAME
{
    enum
    {
        OPTION_NAMES,
    } __info;

    union
    {
        OPTION_TYPE NAME_data;
    } __data;
};
*/

map<string, __enumLookupData> enumData;
map<string, __templEnumLookupData> templEnumData;
