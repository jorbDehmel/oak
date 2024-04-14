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
    // Methods
    test("a.b.c.d.e()", "e(a.b.c.d)");
    test("a.b(c, d, e)", "b(a, c, d, e)");

    // Math and logic
    test("1 + 2", "Add(1, 2)");
    test("20.0 / 10.0", "Div(20.0, 10.0)");
    test("1 + 2 / 3 * 4 + 5", "Add(Add(1, Mult(Div(2, 3), 4)), 5)");
    test("a && !b && c", "Andd(Andd(a, Not(b)), c)");
    test("a || !b || c", "Orr(Orr(a, Not(b)), c)");

    // Unaries
    test("++a", "Incr(a)");
    test("--a", "Decr(a)");
    test("!a", "Not(a)");

    return 0;
}
