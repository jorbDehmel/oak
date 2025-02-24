
# The Oak Programming Language, v2

J Dehmel, MIT License

This document outlines the `Oak` programming language and the
`acorn` translator. Part 1 details the programmer usage, while
part 2 details maintainer usage.

# Part 1: User Manual

This section details the *usage* of `Oak`. For implementation
and maintenance details, see part 2.

## What and Why is `Oak`?

`Oak` is a modern extension of `C`.

## `Hello, World!`

```rust
// Include the `std/io.oak` file for system I/O
include!("std/io.oak");

// The main function
let main() -> i32 {
    // Print the string 'Hello, world!'
    print("Hello, world!");

    // Exit without error
    return 0i32;
}
```

## Quirks of `Oak`

`Oak` is built upon `C` and is very syntactically similar to
`Rust`. However, there are many things that are different. This
section details the largest of these.

### Referencing and dereferencing

In `C/C++`, referencing is done via `&` and dereferencing via
`*`. In `Oak`, referencing is only ever implicitly done and
dereferencing is done via `^`. Referencing in `Oak` is done to
denote mutability, as in `C`.

```rust
let foo(a: ^^i32) -> void {
  // `a` is an i32 pointer by reference
  // We can change what the thing passed as `a` *points to*
}

let foo2(a: ^i32) -> void {
  // `a` is an i32 by reference
  // We can change the value of the thing passed as `a`
}

let foo3(a: i32) -> void {
  // `a` is a copy of an i32
  // If we change `a`, it will not propagate to the thing passed
}

let main() -> void {
  let fizz: i32;

  // foo(fizz); // This is illegal! Cannot reference twice!
  foo2(fizz); // This is legal: `fizz` may be modified
  foo3(fizz); // This is legal: `fizz` won't be modified
}
```

### Plural function definition

Just as `int a, b, c;` would be valid `C`, `let a, b, c: i32;`
is valid `Oak`. However, unlike `C`, `Oak` has a version of
plural instantiation for functions. The following declares a
series of functions that all take in two and return one `i8`.

```rust
// Defines 5 fn signatures in one line
let Add, Sub, Mult, Div, Mod (lhs: i8, rhs: i8) -> i8;

// Implements those 5 fns
let Add, Sub, Mult, Div, Mod (lhs: i8, rhs: i8) -> i8 {
  // Body goes here
}
```

This makes the implementation of large interfacial files easier.

### Types

In `Oak`, an unsized array of `i32`s is `[]i32`. An array of
pointers to arrays of `bool`s, then, would be `[]^[]bool`.

A pointer to a function taking no arguments and yielding `void`
would be `^() -> void`. If there are arguments, they must be
named (usually via the dummy `_` symbol).

For example, a pointer to a function taking an `i32`, an `i64`,
and a `^i8` and yielding a pointer to a no-argument `void`
function would be:

```rust
// A really dumb (but legal) type
let foo: ^(_: i32, _: i64, _: ^i8) -> ^() -> void;
```

## Pragmas

Pragmas are activated via the `pragma!` macro. They have two
forms: `pragma!("key", "value")` and `pragma!("key")`. Most
pragma implementations use the latter. These give instructions
to the compiler on how to handle the current file (**not** the
entire translation unit). The following details current pragmas.

 Command                          | Meaning
----------------------------------|-----------------------------
 `pragma!("no_dialect")`          | Do not apply dialect rules
 `pragma!("compile_should_fail")` | Compilation should fail

## Rules and Dialects

# Part 2: Maintainer Manual

This section details the history, implementation, and
maintenance of `Oak` and `acorn`. For usage details, see part 1.

## History

`Oak`, `acorn`, and `oak2c` were originally written and licensed
under the GPLv3 in mid-2023. In late 2024, efforts to rewrite
the codebase using best practices and under the MIT license
began. This document is associated with this "second version" of
`Oak`, and details its implementation after the rewrite.
Original documentation can be found in legacy versions of the
codebase.

## Implementation

I believe that the `Oak` parser is best classified as LALR,
since it uses finite-token lookahead and reads left-to-right in
a single pass with no backtracking ('I' being J Dehmel). It is
implemented without the use of a parser generator, and thus I am
not confident in that classification.
