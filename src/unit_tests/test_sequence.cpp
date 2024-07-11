/*
Unit testing for Oak.

Jordan Dehmel, 2024
jdehmel@outlook.com
*/

#include "../oakc_fns.hpp"
#include "test.hpp"
#include <string>

#warning "File is unimplemented!"

void test_create_sequence()
{
    std::string input = "let foo() -> void { }";
    std::string expected = "void foo_FN_MAPS_void(void) { }";

    AcornSettings s;
    std::list<Token> from = Lexer().lex_list(input);

    ASTNode root = createSequence(from, s);
    std::string c = toC(root, s);

    std::cout << "Observed:\n```\n"
              << c << "\n```\n"
              << "Expected:\n```\n"
              << expected << "\n```\n";

    // fakeAssert(c == expected);
}

void test_resolve_function()
{
}

int main()
{
    // Commented out because it is sigsev-ing:
    // test_create_sequence();

    test_resolve_function();

    return 0;
}
