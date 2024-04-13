/*
Unit testing for Oak.

Jordan Dehmel, 2024
jdehmel@outlook.com
*/

#include "../oakc_fns.hpp"
#include "test.hpp"

void test(const std::string &_inp, const std::string &_exp)
{
    Lexer l;

    std::list<Token> inp = l.lex_list(_inp);
    std::list<Token> exp = l.lex_list(_exp);

    operatorSub(inp);
    assertEqual(inp, exp);
}

int main()
{
    test("a.b.c.d.e()", "e(a.b.c.d)");
    test("a.b(c, d, e)", "b(a, c, d, e)");
    test("1 + 2", "Add(1, 2)");
    test("20.0 / 10.0", "Div(20.0, 10.0)");

    return 0;
}
