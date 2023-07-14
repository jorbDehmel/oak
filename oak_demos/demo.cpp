#define impl(onto, name, what) \
    onto.methods[name] = (void *)(&what)
#define call(from, name, type) \
    (from.methods.count(name)) ? ((type)from.methods[name])(from) : (throw 1)
#include <map>
////////////////////////////////

#include <iostream>
#include <vector>
using namespace std;

struct test
{
    int a, b, c;

    map<string, void *> methods;
    map<string, void *> vars;
};

int sum_test(test &what)
{
    return what.a + what.b + what.c;
}

/*// A demo of a smarter macro
#def shorten_var_name(what)
{
#let newname = name(what)[ : -2];
    let $(newname) : $what = what;
}*/

// Smart generics
template <class T>
struct holder
{
    T data;

    holder(T &from)
    {
        this->data = from;

        return this;
    }

    T operator[](int &index)
    {
        return data;
    }
};

int main()
{
    printf("Hi! %s\n", "String");

    for (int i = 0; i < 15; i++)
    {
        printf("%d\n", i);
    }

    test a;
    a.a = a.b = a.c = 5;

    // let a.test($a)->u32 = sum_test;
    impl(a, "test", sum_test);

    // printf("%d", a.test());
    printf("%d", call(a, "test", int (*)(test &)));

    // let a.d : u32 = a.test();
    a.vars["d"] = (void *)(new int(call(a, "test", int (*)(test &))));

    // let ad_type : type = $a.d;
    int ad_type;

    // Dup will be of type u32
    // let dup: ad_type;
    typeof(ad_type) dup;

    while (true)
    {
        break;
    }

    if (false)
    {
        return 1;
    }
    else if (false)
    {
        return 2;
    }
    else
    {
        printf("kitttyyy");
    }

    test *pointer = new test;

    pointer->a = pointer->b = pointer->c = 1;

    delete pointer;

    //////////////////////////
    delete (int *)(a.vars["d"]);
    //////////////////////////

    return 0;
}
