#include "sequence.hpp"
#include <vector>
#include <string>
#include <iostream>
using namespace std;

int main()
{
    {
        Type type = toType("()->i8");
        table["hi"].push_back(__multiTableSymbol{sequence{}, type});
    }

    addStruct("let test: struct {a: i8, } ");

    {
        Type type = toType("test");
        table["do"].push_back(__multiTableSymbol{sequence{}, type});
    }

    {
        Type type = toType("(c: i8, d: i8) -> str");
        table["what"].push_back(__multiTableSymbol{sequence{}, type});
    }

    string what = "what(hi(), do.a) hihihi";
    vector<string> lexed = lex(what);

    cout << what << '\n';
    for (string s : lexed)
    {
        cout << s << ' ';
    }
    cout << '\n';

    Type retType = resolveFunction(lexed);
    cout << "The output type was: " << toStr(&retType) << '\n';

    return 0;
}
