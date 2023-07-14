/*
Jordan Dehmel, 2023
GPLv3

Standard io object files with std_io.oak
This file must be compiled, as symbols are
provided during linking for cross-language
compilation.
*/

#include <iostream>
using namespace std;

typedef const char *const str;

void print(str what)
{
    cout << what;
    return;
}

void println(str what)
{
    cout << what << '\n';
    return;
}

void print_err(str what)
{
    cerr << what;
    return;
}

void println_err(str what)
{
    cerr << what << '\n';
    return;
}

char get_char()
{
    char out;
    cin >> out;
    return out;
}
