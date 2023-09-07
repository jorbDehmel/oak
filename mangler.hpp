// Canonical manglings for Oak symbols

#ifndef MANGLER_HPP
#define MANGLER_HPP

#include <string>
#include <vector>

#include "type-builder.hpp"

using namespace std;

/*
Mangling is a way to reduce items which require multiple
symbols to get to into a single symbol.

// Each thing maps to exactly one mangle, which is unique to it

// Examples of canonical mangles:

// Struct overrides are not allowed in Oak
// This means that non-generic structs go un-mangled

let test: struct
{
    a, b, c: i32,
}

-> "test"

let test<t>: struct
{
    a, b, c: i32,
    data: t,
}

test<i32>
-> "test_GEN_i32_ENDGEN"

let test<t, h>: struct
{
    a: t,
    b: h,
}

test<i32, bool>
-> "test_GEN_i32_JOIN_bool_ENDGEN"

test<i32, ^(i32, i32) -> bool>
-> "test_GEN_i32_JOIN_PTR_FN_i32_i32_MAPS_bool_ENDGEN"

// Enumerations also cannot be overridden except in the case of generics
// They mangle exactly like structs

let test<t>: enum
{
    some: t,
    none: unit,
}

test<i32>
-> "test_GEN_i32_ENDGEN"

// Symbols are the important part
// Functions can be overridden by arg type or generic
// Symbs can be overridden by type

let test(self: ^bool, other: bool) -> void;
-> "test_FN_PTR_bool_JOIN_bool_MAPS_void"

let test<t>() -> t;

test<i32>
-> "test_GEN_i32_ENDGEN_FN_MAPS_i32"

map<string, vector<i32>>
-> "map_GEN_string_JOIN_vector_GEN_i32_ENDGEN_ENDGEN"

a: map<string, vector<i32>>
-> "a_TYPE_map_GEN_string_JOIN_vector_GEN_i32_ENDGEN_ENDGEN_ENTYPE"

*/

// Used for mangling and resolving generics pre-sequencing
// Immediately useful as of Oak ~0.0.12 (re-implementing generics)

string mangle(const vector<string> &what);

// Mangle a struct, given its generic substitutions
string mangleStruct(const string &name, const vector<string> &generics = vector<string>());

// Mangle an enumeration, givne its generic substitutions
string mangleEnum(const string &name, const vector<string> &generics = vector<string>());

// Used for translation-time variable uniqueness in the face
// of overloaded names. Not immediately useful, but necessary
// for future lower-level (C or LLVM IR) translations.
string mangleType(Type &type);
string mangleSymb(const string &name, Type &type);
string mangleSymb(const string &name, const string &typeStr);

#endif
