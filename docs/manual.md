
# The Oak Programming Language, v2

J Dehmel, MIT License

![The `Oak` logo: A pixelated tree](../logo.png)

[Oak on GitHub](https://github.com/jorbDehmel/oak)

[Oak VSCode Syntax Highlighting](https://github.com/jorbDehmel/oak-syntax-highlighting)

This document outlines the `Oak` programming language and the
`acorn` translator. Part 1 details the programmer usage, while
part 2 details maintainer usage.

# Part 1: User Manual

This section details the *usage* of `Oak`. For implementation
and maintenance details, see part 2.

## What and Why is `Oak`?

`Oak` is a modern extension of `C`. It has static typing, modern
macros, generics, traits, packages, integrated build,
compile-time inflection, and compile-time modifiable syntax. It
can be used as compile-time "glue", much as `python` can be used
at runtime. `Oak` aims to be easy to interface and extend, and
offers tools for the construction of syntactically diverse
"dialects".

`Oak`'s preprocessor is intentionally Turing-complete: Indeed,
it is `Oak`. This allows arbitrarily (or indeed infinitely)
nested preprocessor expansions. The preprocessor halts when a
pass of it fails to alter the token stream.

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

## Basic Syntax Demo

```rust
include!(
  "std/std.oak",  // The std package
  "std/rules.oak" // Rule definitions for std
);

// If we want methods, inline declarations, etc, we must use the
// std ruleset
rule::use!("std");

let a_fn(x: f64) -> f64 {
  if (x < 123.45f64) {
    return x * x * x * x;
  } else if (x > 0.0f64)
    return x; // If no curly braces, only applies to one stmt
  else
    return 0.0f64;
}

// Aforementioned curly brace rules apply to fns too
let another_fn() -> void
  return;

// All values are held at once
let S: struct {
  a: i32,
  b: f64,
  c: bool
}

// Only one option is held at once
let E: enum {
  a: f64,
  b, c: bool
}

// Main fn is necessary
// Array specifier goes before the type
let main(c: i32, v: [][]i8) -> i32 {
  // This is how we must do it without the std ruleset
  let a: i32;
  a = 0i32;

  // These are legal only because we are using the std ruleset
  let b: i32 = 0i32;
  let c = 0i32;

  if (a == b) {
    print("Hi from line ");
    print(LINE!);
    endl();
  }

  // All number literals must be typed
  let i = 128i32;

  // "for" loops don't exist in canonical oak: Need a ruleset!
  while (i > 64i32) {
    print(i);
    endl();

    // All unary operators are prefix only
    --i;
  }

  // No built-in subscript operator
  print(Get(v, 0uint));
  print('\n'); // Single quote strings are the same as double

  // Instance of enum type
  let e: E;

  // Built-in for enums that allows initialization of options
  // Without std ruleset, it would be `wrap_c(e, false);`
  e.wrap_c(false);

  match (e) {
    case a(data: f64) {
      // data captures the option value
      print(data);
      endl();
    } case b(data: bool) {
      print(data);
      endl();
    } else {
      // "default" block for unhandled cases
      // Cannot capture any value
    }
  }

  return 0i32;
}
```

## Quirks of `Oak`

`Oak` is built upon `C` and is very syntactically similar to
`Rust`. However, there are many things that are different. This
section details the largest of these.

### Built-in Types

The following are built-in types in `Oak`.

 Type    | Description
---------|------------------------------------------------------
 `i8`    | Signed char / 1-byte integer
 `u8`    | Unsigned char / 1-byte integer
 `i16`   | 2-byte signed integer
 `u16`   | 2-byte unsigned integer
 `i32`   | 4-byte signed integer
 `u32`   | 4-byte unsigned integer
 `i64`   | 8-byte signed integer
 `u64`   | 8-byte unsigned integer
 `int`   | Native signed integer representation (unsized)
 `uint`  | Native unsigned integer representation (unsized)
 `i128`  | (EXPERIMENTAL)
 `u128`  | (EXPERIMENTAL)
 `f32`   | Single-precision floating-point
 `f64`   | Double-precision floating-point
 `f128`  | (EXPERIMENTAL)
 `bool`  | Boolean value (true/false) (unsized)
 `^void` | Typeless pointer (`C`'s `void *`) (unsized)

Derived types can be done by declaring `let Foo: struct {}` or
`let Fizz: enum {}` in the global namespace, as below.

```rust
// Declare a new struct type `SomeStruct` w/ 3 members
let SomeStruct: struct {
  a: u128, // Comments are allowed here
  b: bool,
  third_member: ^^^^^^^^^^^^^^u8 // Absurdly nested pointer
} // No semicolon needed afterwards

// Declare a new enum type `SomeEnum` w/ 4 options
let SomeEnum: enum {
  first: bool,
  second: ^i8,
  third: SomeStruct, // Composition
  fourth: [][]bool,  // Optional trailing comma on last entry
}
```

Structs have *members*, each of which has an independent value
at a given time. Under the hood, these are identical to `C`
structs. Enums, on the other hand, have *options*, only **one**
of which holds a value at a given time. This value is only
accessible within `match` statements, and is a cross between `C`
unions and enums.

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
is valid `Oak`. `Oak` also has a version of plural instantiation
for functions. The following declares a series of functions that
all take in two and return one `i8`. Note that, unlike `C`, the
function argument block applies to *all* the declared symbols.

```rust
// Defines 5 fn signatures in one line
let Add, Sub, Mult, Div, Mod (lhs: i8, rhs: i8) -> i8;

// Implements those 5 fns
let Add, Sub, Mult, Div, Mod (lhs: i8, rhs: i8) -> i8 {
  // Body goes here
}
```

Equivalent `C` code would be:

```c
// let Add, Sub, Mult, Div, Mod (lhs: i8, rhs: i8) -> i8;
i8 Add(i8, i8), Sub(i8, i8), Mult(i8, i8), Div(i8, i8),
  Mod(i8, i8);
```

This makes the implementation of large interfacial files easier.

### Functions and Statements

Much like in `C` `if (true) printf("hi");` is legal, in `Oak`
`if (true) printf!("hi");` is legal. This is because clauses
like `if`, `while`, `else`, and `case` operate on the first
statement to follow them and scopes count as a single statement.
However, unlike `C`, `Oak` extends this princible to function
bodies.

```rust
include!("std/io.oak");

let main() -> i32
  if (true)
    print("hi\n");

```

### Conditional Macros

Conditional compilation in `Oak` can be done via the `if_eq!`
and `if_neq!` macros provided by `std/if_macros.oak`.

```rust
include!("std/std.oak", "std/if_macros.oak");

let target! = "fizz";

if_eq!(target!"DNE", "fizz",
  let foo() -> void {
    print("Hi from line 11!\n");
  },
  let foo() -> void {
    print("Hi from line 14!\n");
  }
);

if_eq!(target!"DNE", "buzz",
  let fuzz() -> void {
    print("Hi from line 20!\n");
  }
);

if_neq!(SYSTEM!, "UNIX",
  compile_time::warning!("Use Linux!!!");,
  compile_time::print!("Good job using Linux.");
);
```

These macros compare their first two arguments as string
literals (after expansion), then conditionally execute a code
block based on their values. If three arguments are provided,
the third is the "then" block (with no else block). If four
arguments are provided, the third will be the "then" and the
fourth will be the "else".

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

### Literal concatenation

To increase readability and ease of formatting, subsequent
literals of the same type seperated only by whitespace merge
together: For instance, `"Hello, "       "world!"` becomes
`"Hello, world!"` and `123 456` becomes `123456`. This comes in
handy when addressing binary and hexadecimal integer literals
(`0b...` and `0x...`, respectively). This also applies to number
literal type suffixes, which we will address next.

### Numeric literal type suffixes

It is illegal to have an untyped number literal in `Oak`: If you
want a simple `int` or `uint`, you must say so. This is done via
type suffixes, where the type desired is simply appended onto
the literal. For instance, an `i32` with value `12345` would be
`12345i32`. This works for all numeric types and all types of
numeric literals.

```rust
let main() -> i32 {

  0b 1111 0000 u8;
  0b11110000u8;
  0x00 12FFFF u32; // Note: "F" cannot follow a space
  .123 456 789f32;

  // You *can* do this, but please don't.
  3.
  1 4
  1 5 9
  2 6 5 f64;

  // An i32 literal with value 0
  return 0i32;
}
```

### "Methods"

`Oak` does not actually have methods: `a.b()` is just shorthand
for `b(a)` (and so on) enabled by the `std` ruleset. Thus, the
first argument of a so-called "method" is usually called `self`
and is made to be mutable. In this way, you can add methods to
any type at any time.

```rust
include!("std/std.oak", "std/rules.oak");
rule::use!("std");

let flimbify(self: i32) -> void {
  // Some absurd code here
}

let main() -> i32 {
  let a = 123i32;
  a.flimbify();

  return 0i32;
}
```

### Operators

Like `C++`, `Oak` has operator overloading for types. However,
`Oak` does not have any non-overloaded operators: Therefore, you
must `include!("std/operators.oak");` or
`include!("std/std.oak");` in order to perform any operations.
Similar to `python`, `Oak`'s operators have plain English names.
The infix built-in binary operators are listed below in
precedence order.

 Operator | Resolved name | Function
----------|---------------|-------------------------------------
 `&`      | `And`         | Bitwise "or"
 `\|`     | `Or`          | Bitwise "and"
 `*`      | `Mult`        | Multiplication
 `/`      | `Div`         | Division
 `%`      | `Mod`         | Modulo
 `+`      | `Add`         | Addition
 `-`      | `Sub`         | Subtraction
 `==`     | `Eq`          | Boolean negation
 `!=`     | `Neq`         | Not-equal-to
 `<`      | `Less`        | Less-than
 `>`      | `Great`       | Greater-than
 `<=`     | `Leq`         | Less-than-or-equal-to
 `>`      | `Greq`        | Greater-than-or-equal-to
 `&&`     | `Andd`        | Boolean "and"
 `\|\|`   | `Orr`         | Boolean "or"
 `=`      | `Copy`        | Copy RHS into LHS (RHS can be tuple)
 `&=`     | `AndEq`       | Bitwise "and" self by other
 `\|=`    | `OrEq`        | Bitwise "or" self by other
 `<<=`    | `LBSEq`       | Left bit shift by
 `>>=`    | `RBSEq`       | Right bit shift by
 `*=`     | `MultEq`      | Assign LHS itself times RHS
 `/=`     | `DivEq`       | Assign LHS itself divided by RHS
 `%=`     | `ModEq`       | Assign LHS itself mod RHS
 `+=`     | `PlusEq`      | Increment by
 `-=`     | `SubEq`       | Decrement by
 `&&=`    | `AnddEq`      | Binary "and" self by other
 `\|\|=`  | `OrrEq`       | Binary "or" self by other

Note that `a = (1, 2, b, "foo");` is legal as shorthand for
`Copy(a, 1, 2, b, "foo")`, even though this RHS shorthand is
otherwise invalid. There are also the unary
operators, which are **all prefix**.

 Operator | Resolved name | Function
----------|---------------|-------------------------------------
 `++`     | `Incr`        | Increment RHS by 1
 `--`     | `Decr`        | Decrement RHS by 1
 `!`      | `Not`         | Negate RHS
 `~`      | `Flip`        | Set RHS equal to its negation

In addition to these, there are several operators which have no
precedence or symbols. These are listed below.

 Resolved name | Function
---------------|------------------------------------------------
 `New`         | Constructor (1 argument, called implicitly)
 `Del`         | Destructor (1 argument, called implicitly)
 `Get`         | Array access (2 arguments, called manually)

A typical object will have `New` called upon it, then `Copy` to
initialize it, then live for some time, then have `Del` called
on it as it falls out of scope. It can be assumed that every
variable which has been declared will have had `New` called on
it, and every variable will have `Del` called on it before
control exits its scope. This allows for RAII programming, which
is `Oak`'s paradigm of memory control.

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
 `pragma!("run_should_fail")`     | Execution should fail
 `pragma!("no_run")`              | Do not run at all
 `pragma!("run_cmd")`             | Format string for execution

## Compiler / Built-in Macros

This section talks about the built-in compiler-control macros of
`Oak`.

### `include!`

This macro pauses the current context, resets to the global
scope, parses the files its string arguments are paths to, then
unpauses the current context. As such, it can be called from
anywhere (not just the global scope). If the path specifies a
file in the current working directory, that file is visited.
Otherwise, `acorn` checks its include path
(usually `/usr/include/oak/`) for a matching file. If no file
can be resolved, an error is thrown and this branch of
compilation terminates.

### `link!`

This macro adds its string arguments as link-time object file
inclusions.

### `flag!`

This macro adds its string arguments as compile and link-time
command-line compiler flags.

### `pragma!`

This is used to manage compile-time flags. Its usage is detailed
elsewhere.

### `rule::new!`, `rule::use!`, `rule::remove!`, `rule::bundle!`

These macros manage the rule subsystem. Their usage is detailed
elsewhere.

### `compile_time::system!`

When parsed, this macro attempts to execute its string argument
as a system command. Unless the `--no_confirm` command-line flag
is used, the user will be asked for confirmation before *any*
compile-time command is run.

### `compile_time::error!`

When parsed, this macro raises its string argument as an error
and shuts down this branch of compilation.

### `compile_time::warning!`

When parsed, this macro prints its string argument as a warning.

### `compile_time::print!`

When parsed, this macro prints its string argument.

### `c!`

A unique ability due to `Oak`'s status as a translated language
is its ability to have inline `C` code. This is done via the
`c!` macro. The code is usually extremely simple and enclosed
in quotes. This is best for calling external libraries, and
**terrible** for calling `Oak`-defined functions (due to the
`Oak` function name mangler).

```rust
include!("std/io.oak");
let foo(x: ^i32) -> void {
  c!(
    "*x = 123;"
  );
  print(x);
}
```

### `size!`

`size!` is replaced by the (possibly padded) compile-time `C`
size of the argument it takes. Its arguments are types or
instances, **not** string literals.

### `type!`

`type!` is replaced by the type of its argument. It takes
instances, **not** string literals.

### `alloc!`

This macro allows dynamic memory allocation into either pointers
(EG `^i32`) or unsized arrays (EG `[]i32`). A call of the form
`let a: ^i32; alloc!(a);` causes pointer `a` to point to a newly
allocated block of memory the size of one of its type.
Alternatively, a call of the form
`let b: []i32; alloc!(b, 16u64);` causes unsized array `b` to
point to a newly allocated block of memory large enough for a
contiguous array of 16 of its type.

### `free!`

`free!(a)` is called on a variable `a` when it is time to
release dynamically allocated memory. The same call works
whether `a` is a pointer or an unsized array.

### `str!` and `unstr!`

Although these macros

## Macros

There are two types of macros in Oak: Inline/alias and
functional/compiled. Inline macros are almost exactly like basic
`C` preprocessor definitions. They are defined as below.

```rust
// Declare an inline macro
let foo! = the contents go here;

// This will be replaced by the contents of the macro
foo!;
```

There are several built-in inline macros. These are listed
below.

 Name           | Domain                           | Meaning
----------------|----------------------------------|----------
`LINE!`         | Positive `u64`                   | Cur line
`FILE!`         | `[]i8` file path                 | Cur file
`oak::VERSION!` | `[]i8` package version           | Cur version
`SYSTEM!`       |"WINDOWS", "UNIX", "OSX", "OTHER" | Cur OS

Functional macros, on the other hand, are unrestricted Oak
programs which are available to run at compile-time. These
programs are run with the arguments provided at
replacement-time, and their `stdout` is lexed and inserted in
their stead. They are essentially nested translation units that
can be used elsewhere.

```rust
// They take the same form as `main`
let print_foo!(c: i32, v: [][]i8) -> i32 {
  // Functional macros do not inherit their source files
  // definitions! Therefore, you must put any inclusions you
  // want access to directly inside.
  include!("std/io.oak");

  // Print a statement that prints 'foo'
  print(
    "print(\"foo\n\");\n"
  );

  return 0i32;
}

include!("std/io.oak");
let main() -> i32 {

  // The arguments are passed as argv, but ignored in this case
  // This is replaced before compilation by `print("foo\n");`
  print_foo!("hi", 123f64, 456i32);

  return 0i32;
}
```

You can also test for macro existence: The fragment
`fizz!false` is replaced by the value of `fizz!` if it exists,
and `false` otherwise.

```rust
// If you do this at compile-time, it becomes preprocessor
// conditional compilation
if (blimbo!false) {
  print("'blimbo!' exists!\n");
} else {
  print("'blimbo!' does not exist (sad)...\n");
}
```

Note that `blimbo!` on its own will cause a compile time error
if it doesn't exist. Also note that, since inline macros take
precedence over compiled ones, this also works on the latter:
`blimbo!false()` will become `false()` if `blimbo!` doesn't
exist, and the output of `blimbo!()` if it does.

## Rules

## Dialects

An `Oak` dialect is a set of rules which is enforced from the
command-line. These rules are always active (unless
`pragma!("no_dialect")` is used) and are executed before
anything else. Dialects are effectively command-line-enforced
syntactic branches of the language. Since the rule system (as
iterated transduction more powerful than FSTs) is
Turing-complete, dialects can be any language: Indeed, they are
intended to be treated as new languages and used for syntactic
alterations and modeling thereof.

A dialect is loaded from a *dialect file* via
`acorn -D file.oak`.

## Packages

An `Oak` package is a directory with a `spec.oak` file. This
file gives installation, attribution, and versioning information
to the compiler.

```rust
// std/spec.oak

pragma!("no_dialect");

let std::ABOUT!   = "Oak standard Package";
let std::AUTHOR!  = "Jordan Dehmel";
let std::EMAIL!   = "jdehmel@outlook.com";

// The most important entry: This is required for every package
let std::VERSION! = "0.0.1";

// The second most important entry! This is the file that is
// *parsed* (but not compiled or run) when `acorn -S NAME` is
// called. This can contain compile-time system calls that
// prepare your package in-place for installation.
let std::INSTALL! = "std/install.oak";

let std::LICENSE! = "MIT";
let std::SOURCE!  = "github.com/jorbDehmel";
let std::YEAR!    = "2025";
```

Once installed, packages live in `/usr/include/oak/`, which is
marked with permissions `777` so that macros can compile.
Packages are symlinked according to their version (just like
`C` library files). The package name `foo.1.2.3` is to be read
`foo` version `1.2.3`: Indeed, any characters after the first
`.` are ignored when parsing a package name.

If we had `foo.1.2.3` and `foo.2.0.1` installed, the following
would all be valid include paths.

```
foo.1.2.3 (directory)
foo.2.0.1 (directory)
foo.1.2   -> foo.1.2.3
foo.1     -> foo.1.2.3
foo.2.0   -> foo.2.0.1
foo.2     -> foo.2.0.1
foo       -> foo
```

So `include!("foo/foo.oak");` would refer to
`/usr/include/oak/foo.2.0.1/foo.oak`, whereas
`include!("foo.1.2/foo.oak");` would refer to
`/usr/include/oak/foo.1.2.3/foo.oak`. This allows some
integrated dependency version control.

## Test Suites

An `Oak` test suite is any directory called `tests` containing
zero or more `*.oak` files. These files will be compiled and
optionally run by `acorn` when in testing mode. The following
table highlights the different options for testing mode.

 Flag(s) | Meaning
---------|------------------------------------------------------
 `-T`    | Compile all tests, even if some fail
 `-TT`   | Compile tests until one fails
 `-TE`   | Compile and run all tests, even if some fail
 `-TTE`  | Compile and run tests until a compilation fails
 `-TEE`  | Compile and run tests until an execution fails
 `-TTEE` | Compile and run tests until any failure occurs

When in testing mode, any non-flag command line inputs will be
treated as folders to search for `tests` directories in. For
instance, `acorn -TTE foo fizz buzz` will compile and run all
tests from `./foo/tests/`, `./fizz/tests/`, and `./buzz/tests/`
until a compilation fails (ignoring runtime failures).

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

## Parser classification

I* believe that the `Oak` parser is best classified as LALR,
since it uses finite-token lookahead and reads left-to-right in
a single pass with no backtracking. It is implemented without
the use of a parser generator, and thus I am not confident in
that classification.

*'I' being J Dehmel

## The Mangler / `oak2c`

One goal of `Oak` is to be easily interfaceable with the `C`
family. This, combined with type-distinguished function calls,
necessitates a function name mangler. For convenience, `Oak` has
a consistent, non-implementation-specific, mangler called
`oak2c`. This takes in (possibly name-ambiguous but
type-unambiguous) `Oak` function signatures and outputs
name-unambiguous human-readable `C` function signatures. This is
in opposition to the `C++` and `rust` manglers, which are at
best implementation-specific and non-human-readable.

The rules of `oak2c` work on the function signature as a token
stream. For instance, take the following `Oak` signatures, where
the mangled `C` signature follows as a comment.

```rust
let a() -> void;
// void a_FN_MAPS_void();

let b(_: ^i8, __: [16][]bool) -> i32;
// i32 b_FN_PTR_i8_JOIN_ARR_ARR_bool_MAPS_i32(i8 *, bool *[16]);

let c::d(_: ^(_: i32, _: [][]i8) -> i32) -> void;
// void c_d_FN_PTR_FN_i32_JOIN_ARR_ARR_i8_MAPS_i32_MAPS_void(
//     i32 (*)(i32, i8 **));
```

These mangled names can be arbitrarily long, but they will
always be unambiguous. There is a special case for templated
structs used as arguments, demonstrated below.

```rust
let Node<T>: struct {
  data: T,
  next: ^Node<T>
}

let New<T>(self: ^Node<T>) -> void {
  // ...
}

let main() -> i32 {
  // Creates `New(self: ^Node<i32>) -> void`
  let a: Node<i32>;

  // Creates `New(self: ^Node<bool>) -> void`
  let b: Node<bool>;

  return 0i32;
}

/*
// Produces the signatures:
struct Node_GEN_i32_ENDGEN;
struct Node_GEN_bool_ENDGEN;
void New_FN_PTR_node_GEN_i32_ENDGEN_MAPS_void(Node_GEN_i32_ENDGEN *);
void New_FN_PTR_node_GEN_bool_ENDGEN_MAPS_void(Node_GEN_bool_ENDGEN *);
*/
```

The following table demonstrates the rules.

 Signature token | Mangled version
-----------------|-----------------
 `^`             | `PTR`
 `[]`            | `ARR`
 `[...]`         | `ARR`
 `(` (fn start)  | `FN`
 `,` (fn delim)  | `JOIN`
 `) ->` (fn end) | `MAPS`
 `::`            | `_` (as usual)
 (anything else) | (itself)

## Macro Argument String Literalization

```rust
fizz!(foo);   // Passed as "foo"

fizz!("foo"); // Passed as "\"foo\""
fizz!('foo'); // Passed as "\"foo\""
fizz!(`foo`); // Passed as "\"foo\""

fizz!("\"foo\""); // Passed as "\"\\\"foo\\\"\""
fizz!("'foo'");   // Passed as "\"'foo'\""
fizz!('"foo"');   // Passed as "\"\\\"foo\\\"\""
```

This becomes tricky when creating macros, especially those which
must use both string-enclosed and non-string-enclosed values
(EG assert, printf).

```rust
printf!(
  "%: %\n \"%\"", // "\"%: %\\n \\\"%\\\"\""
  foo,            // "foo"
  fizz,           // "fizz"
  "buzz"          // "\"buzz\""
);

/*
print(foo);
print(": ");
print(fizz);
print("\n \"");
print("buzz");
print("\"");
*/
```

## Passes

This section details the passes that acorn performs when
translating code.

1. Load entry point file
2. Lex / tokenize
3. Preprocess: Repeat until no changes are made or some max
    number of passes is exceeded
    1. Macro definitions: Compile any that are functional
    2. Fetch and recurse on all `include!`-ed files
    3. Process all linkages, pragmas, flags, rule additions, new
        rules, rule removals, rule bundles, compile-time system
        commands, errors, warning
    4. Resolve inline and functional macro calls
    5. Turn math into overloadable operator calls
    6. Apply ruleset once
4. Syntax check
5. Parse into AST
6. (Optional) Translate to `C`
7. (Optional) Call `gcc` from `C` to object file
8. (Optional) Call `g++` on object file(s) to executable
9. (Optional) Execute

Note that the rule system, as iterated non-length-preserving
transduction, is Turing-complete. Thus, the question of
compilation halting is undecidable (except for the imposed max
passes limit).

## Pass 2: Lexing

The `Oak` lexer is **not** DFA-based: It is a series of nested
`if` statements. This allows for easier special cases and
modifiability, at the cost of an arguably slower runtime.

## Pass 3. Preprocess

This is the bulk of the `Oak` language.

1. Macro definitions: Compile any that are functional
2. Fetch and recurse on all `include!`-ed files
3. Process all linkages, pragmas, flags, rule additions, new
    rules, rule removals, rule bundles, compile-time system
    commands, errors, warning
4. Resolve inline and functional macro calls
5. Turn math into overloadable operator calls
6. Apply ruleset once

## Pass 5: Grammar

`Oak` is not defined via EBNF: However, it is a useful tool for
visualizing parsing. Therefore, this section attempts to detail
the **post-preprocessing** (no macros, includes, or rules)
parsing process via EBNF / `bison` notation.

```yacc
global_scope:
  global_scope_entry
| global_scope_entry global_scope
;

global_scope_entry:
  let_struct
| let_enum
| let_fn
| '\n'
;

let_struct:
  'let' ID ':' 'struct' ';'
| 'let' ID ':' 'struct' body
;

let_enum:
  'let' ID ':' 'enum' ';'
| 'let' ID ':' 'enum' body
;

body:
  '{' variable_decls '}'
| '{' '}'
;

variable_decls:
  variable_decl
| variable_decl ',' variable_decls
;

let_fn:
  'let' ID '(' variable_decls ')' '->' type ';'
| 'let' ID '(' variable_decls ')' '->' type '{' stmts '}'
| 'let' ID '(' ')' '->' type ';'
| 'let' ID '(' ')' '->' type '{' stmts '}'
;

variable_decl:
  variable_names ':' type
;

variables_names:
  ID
| ID ',' variable_names
;

type:
  ID
| '[' ']' type
| '[' INTEGER_LITERAL ']' type
| '^' type
| '^' '(' variable_decls ')' '->' type
| '^' '(' ')' '->' type
;

stmts:
  stmt
| stmt stmts
;

stmt:
  '{' stmts '}'
| 'let' variables_names ':' type ';'
| 'if' '(' object ')' stmt
| 'if' '(' object ')' stmt 'else' stmt
| 'while' '(' object ')' stmt
| fn_call ';'
| match_stmt
;

object:
  fn_call
| obj_member
;

obj_member:
  ID '.' ID
| ID '.' obj_member
;

fn_call:
  ID '(' variable_names ')'
| ID '(' ')'
;

match_stmt:
  'match' '(' object ')' '{' match_cases '}'
;

match_cases:
  match_case
| match_else
| match_case match_cases
| match_else match_cases
;

match_case:
  'case' ID '(' variable_decl ')' stmt
;

match_else:
  'else' stmt
;
```

After the tokenized source code is parsed into an AST, the tree
is reconstructed into equivalent `C` code and written to file.
Note that there are numerous exceptions to this grammar: Many
macros must be delayed until parse- or reconstruct-time to
function.

## Pass 8: Linking via `g++`

Note that `g++` (a `C++` compiler) is used for linkage instead
of `gcc` (a `C` compiler): This allows `C++` object files to be
included at link-time. `g++` is also just generally "smarter",
so it is advantageous to use it.

## Pass 9: Execution

`acorn` retains information about the translation unit as it
runs: This allows us to use the `run_should_fail` pragma. This
pragma does *not* modify the actual runtime: It only says that,
if `acorn` is the one executing, a nonzero exit code should be
expected and a zero exit code should be treated as an error.
