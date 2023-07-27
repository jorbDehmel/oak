#include "sequence.hpp"
#include <vector>
#include <string>
#include <iostream>
using namespace std;

int main()
{
    addStruct("let test: struct {a: i8, b: ^str, } ");

    Type type = toType("()->i8");
    table["hi"].push_back(__multiTableSymbol{sequence{}, type});

    type = toType("(what: test) -> ^test");
    table["unit"].push_back(__multiTableSymbol{sequence{}, type});

    type = toType("test");
    table["do"].push_back(__multiTableSymbol{sequence{}, type});

    type = toType("(c: i8, d: i8) -> str");
    table["what"].push_back(__multiTableSymbol{sequence{}, type});

    vector<string> tests = {
        "what(hi(), do.a) do",
        "do",
        "do.a",
        "do.b",
        "unit(do).b",
        "false true -10 5 0.0 'hi'",
        "(false)",
    };

    for (string test : tests)
    {
        string what = test;
        int pos = 0;

        vector<string> lexed = lex(what);
        cout << what << '\n';

        while (pos < lexed.size())
        {
            string c = "";
            Type retType = resolveFunction(lexed, pos, c);
            cout << c << '\n';

            cout << "The output type was: " << toStr(&retType) << "\n\n";
        }
    }

    return 0;
}
