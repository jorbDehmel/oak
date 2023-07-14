/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "reconstruct.hpp"

#include <iostream>
using namespace std;

int main()
{
    string name = "__translation_out.oak";

    deps.insert("map");
    deps.insert("string");

    atomics.insert("map");
    atomics.insert("string");

    addSymb("map_pointer_variable", "*map<string, u16>");
    addSymb("fn", "let fn(a: bool) -> map<string, bool>;");
    addSymb("fn2", "let fn2<T>(a: T) -> T;");

    addSymb("main", "let main(argc: u16, argv: *str) -> u16;");

    addSymb("hello_darkness", "let hello_darkness<MY, OLD, FRIEND>(a: MY) -> void;");

    // let fn(a: bool) -> map<string, bool>;
    // map<string> fn(bool *a);

    addStruct("let type<T, F>: struct {a, b, c: i32, d: u32, }");
    addStruct("let type2<T, F>: live {a, b, c: T, d: bool, e: type<T>, }");

    reconstructAndSave(name);

    return 0;
}
