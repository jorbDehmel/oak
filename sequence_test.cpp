#include <iostream>
#include "lexer.hpp"
#include "sequence.hpp"
using namespace std;

// let i16.get(self: *i16) -> *i16
// {
//      self
// }
// let i: i16 = 5;
// i.get();
//
// Becomes
// i16 *get(i16 *self)
// {
//      return self;
// }
// i16 i = 5;
// get(i);

#ifndef privtvec
#define printvec(What)        \
    {                         \
        cout << #What ":\n";  \
        for (auto s : What)   \
            cout << s << ' '; \
        cout << '\n';         \
    }
#endif

int main()
{
    // Manual sequence tests
    {
        sequence s;

        s.info = code_scope;
        s.items.push_back(sequence{Type(), vector<sequence>(), code_line});
        s.items.push_back(sequence{Type(), vector<sequence>(), code_line});
        s.items.push_back(sequence{Type(), vector<sequence>(), code_line});

        cout << "// Code scope with empty lines:\n"
             << toC(s) << '\n';

        s.items.clear();

        cout << "// Unit scope:\n"
             << toC(s) << '\n';

        sequence t;

        t.info = for_triple;
        t.items.push_back(sequence{Type(), vector<sequence>(), code_line});
        t.items.push_back(sequence{Type(atomic, "bool"), vector<sequence>(), code_line});
        t.items.push_back(sequence{Type(), vector<sequence>(), code_line});

        s.items.clear();
        s.items.push_back(t);

        cout << "// For loop triple:\n"
             << toC(s) << '\n';
    }

    cout << "\n---------------------------------------\n\n";

    // createSequence tests
    {
        string what =
            "{\n"
            "\t5;\n"
            "\t0\n"
            "}\n";

        vector<string> lexed = lex(what);

        printvec(lexed);

        sequence s = createSequence(lexed);

        cout << "// Auto-generated:\n"
             << toC(s) << '\n';
    }

    cout << "\n---------------------------------------\n\n";

    // Method call fixing tests
    {
        vector<string> input = lex("a.b().c(d.e(), f.g(), h(i))");

        printvec(input);
        vector<string> out = fixMethodCall(input);
        printvec(out);

        cout << "Expected:\n"
             << "c(&b(&a), e(&d), g(&f), h(i))\n";
    }

    cout << "\n---------------------------------------\n\n";

    // A few more sequencing tests
    {
        vector<string> input = lex("a.b.c.d.e.f.g.h");
        printvec(input);
        vector<string> out = fixMethodCall(input);
        printvec(out);

        input = lex("album(ab.c())");
        printvec(input);
        out = fixMethodCall(input);
        printvec(out);

        input = lex("abracadabra");
        printvec(input);
        out = fixMethodCall(input);
        printvec(out);

        input = lex("abra()");
        printvec(input);
        out = fixMethodCall(input);
        printvec(out);
    }

    cout << "\n---------------------------------------\n\n";

    // Test create sequence
    {
        string what =
            "{\n"
            "   let hi: i16;\n"
            "   let histruct: struct\n"
            "   {\n"
            "       a: i16,\n"
            "       b: i16,\n"
            "   }\n"
            "}\n";

        printvec(lex(what));

        auto s = createSequence(what);

        cout << "Reconstructed:\n"
             << toC(s) << '\n';

        what =
            "let not(what: bool) -> bool\n"
            "{\n"
            "   if what\n"
            "       return false;\n"
            "   else\n"
            "       return true;\n"
            "}\n";

        s = createSequence(what);

        cout << "Reconstructed:\n"
             << toC(s) << '\n';
    }

    return 0;
}
