#include <sqlite3.h>
#include <oak/std_oak_header.h>
#include <oak/std/string.c>

struct sqlite
{
    sqlite3 *db;
};

struct sqlite_res
{
    struct string **data;
    u128 nrows, ncols;
};

// let Copy(self: ^sqlite_res, rows: u128, cols: u128
//    ) -> sqlite_res;
struct sqlite_res
    Copy_FN_PTR_sqlite_res_JOIN_u128_JOIN_u128_MAPS_sqlite_res(
    struct sqlite_res *, u128, u128);
