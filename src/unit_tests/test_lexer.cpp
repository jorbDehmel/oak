/*
Unit testing for Oak lexer.

Jordan Dehmel, 2024
jdehmel@outlook.com
*/

#include "../lexer.hpp"
#include "test.hpp"

void assert_lex_match(const std::string &_inp,
                      const std::list<std::string> &_exp)
{
    Lexer l;

    auto obs = l.lex_list(_inp);
    auto o = obs.begin();
    auto e = _exp.begin();

    for (; o != obs.end() && e != _exp.end(); ++o, ++e)
    {
        fakeAssert(*o == *e);
    }
}

void test_lexer()
{
    assert_lex_match("a b cd foo ; ;; = == ->",
                     {"a", "b", "cd", "foo", ";", ";", ";", "=",
                      "==", "->"});

    assert_lex_match(
        "let main() -> i32\n{\n\t0\n}\n",
        {"let", "main", "(", ")", "->", "i32", "{", "0", "}"});
}

int main()
{
    test_lexer();

    return 0;
}
