/*
Unit testing for Oak.

Jordan Dehmel, 2024
jdehmel@outlook.com
*/

#include "../oakc_fns.hpp"
#include "test.hpp"

/*
Things that must be tested:

checkLiteral
*/

static AcornSettings s;

Type toType(const std::string &what)
{
    Lexer l;

    return toType(l.lex_list(what), s);
}

int main()
{
    Type a = toType("(_: i32, _: f32) -> void");
    Type b = toType("(_: i64, _: f64) -> void");
    int garbage;

    fakeAssert(!typesAreSame(&a, &b, garbage));
    fakeAssert(!typesAreSameExact(&a, &b));
    fakeAssert(typesAreSameCast(&a, &b, garbage));

    b = toType("(_: ^i32, _: ^^f32) -> void");

    fakeAssert(typesAreSame(&a, &b, garbage));
    fakeAssert(!typesAreSameExact(&a, &b));
    fakeAssert(typesAreSameCast(&a, &b, garbage));

    b = toType("(_: i32, _: f32) -> void");

    fakeAssert(typesAreSame(&a, &b, garbage));
    fakeAssert(typesAreSameExact(&a, &b));
    fakeAssert(typesAreSameCast(&a, &b, garbage));

    a = toType("^void");
    fakeAssert(toStrC(&a, s) == "void*");

    // Don't forget about function mangling!
    a = toType("(a: i32, b: f32) -> void");
    fakeAssert(
        toStrCFunction(&a, s, "foo") ==
        "void foo_FN_i32_JOIN_f32_MAPS_void(i32 a, f32 b)");

    a.prepend(pointer);
    fakeAssert(toStrCFunctionRef(&a, s, "foo") ==
               "void (*foo)(i32, f32)");

    // Check literal tests
    fakeAssert(checkLiteral("foobar") == nullType);
    fakeAssert(checkLiteral("123") == Type(atomic, "i32"));
    fakeAssert(checkLiteral("123u8") == Type(atomic, "u8"));
    fakeAssert(checkLiteral("123.456") == Type(atomic, "f64"));
    fakeAssert(checkLiteral("true") == Type(atomic, "bool"));
    fakeAssert(checkLiteral("false") == Type(atomic, "bool"));
    fakeAssert(checkLiteral("0b11110000") ==
               Type(atomic, "u8"));
    fakeAssert(checkLiteral("0xFF00") == Type(atomic, "u16"));
    fakeAssert(checkLiteral("'foobar foo'") ==
               Type(atomic, "str"));
    fakeAssert(checkLiteral("\"foobar foo\"") ==
               Type(atomic, "str"));

    return 0;
}
