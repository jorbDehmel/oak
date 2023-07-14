/*
For the testing of the link!(...) macro
in .oak translated files via Acorn
*/

#include <iostream>
using namespace std;

void print(const char *const What)
{
    cout << What;
    return;
}

void print(const int What)
{
    cout << What;
    return;
}

void print(const double What)
{
    cout << What;
    return;
}

template <class T>
void print(const T What)
{
    cout << What;
    return;
}
