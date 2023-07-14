#define impl(onto, name, what) \
    onto.methods[name] = (void *)(&what)
#define call(from, name, type) \
    ((type)from.methods[name])(from)
#include <map>
////////////////////////////////////////////////////

#include <iostream>
using namespace std;

struct test
{
    int a, b, c;

    map<string, void *> methods;
};

int add_1(test &on)
{
    return on.a + on.b + on.c;
}

int add_2(test &on)
{
    return 69;
}

int main()
{
    test a;
    a.a = a.b = a.c = 5;

    impl(a, "add", add_1);
    cout << call(a, "add", int (*)(test &)) << '\n';

    impl(a, "add", add_2);
    cout << call(a, "add", int (*)(test &)) << '\n';

    return 0;
}
