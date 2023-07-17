#include "lexer.hpp"
#include "op-sub.hpp"
#include <iostream>
using namespace std;

int main()
{
    vector<string> tests = {
        "(hello + hi && what)",
        "(1 + 3 > 2 && i - 43 >= 12 + 2)",
        "if (i - 7 && 2 + 3 > 5 || 7 - 12 == 5) { do; do; do; } ",
        "let hi(a: b, c: d) -> bool { (a + c >= 6) }",
    };

    for (string what : tests)
    {
        cout << "Case '" << what << "':\n\n";

        vector<string> lexed = lex(what);

        cout << "Lexed:\n";
        for (auto s : lexed)
        {
            cout << s << ' ';
        }
        cout << "\n\n";

        parenSub(lexed);

        cout << "After substitution:\n";
        for (auto s : lexed)
        {
            cout << s << ' ';
        }
        cout << "\n\n";
    }

    return 0;
}
