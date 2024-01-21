
# The Oak Programming Language
## Version 0.4.15

![](./logo_trimmed.png)

Jordan Dehmel, jdehmel@outlook.com \
github.com/jorbDehmel/oak

# Programmer Introduction

## Overview

`Oak` is a modern low-level functional programming language with
compile-time modifiable syntax. Its usecase is in low-level
language and compiler design (see the later section on
dialects). `Oak` also aims to have an integrated build system,
necessitating only one call to the compiler per executable.

`Oak` is most closely related to `C`, but also has strong
macro support, modern typing, generics, compile-time syntax
modification and integrated package management. An interesting
consequence of `Oak`'s compile-time modifiable syntax is that
**`Oak` can be a high or low level language**, depending on the
rules / dialect applied.

This document outlines the basics of `Oak`, as well as the
canonical formatting of `Oak` code. Deviation from this
formatting style is unadvisable, and considered bad form.

`Oak` is, as of now, a translated language; `Oak` code is
translated via the `acorn` utility (see later) into `C`.
`acorn` can also compile `Oak` into object code and link it to
create executables via `clang` and `clang++`, respectively.

## Did You Say "Compile-Time-Modifiable Syntax"?

`Oak` has compile-time-modifiable syntax (see the section on
preprocessor rules), making it highly customizable and flexible
in a way that no other mainstream languages are. In `Oak`, if
you think the language should have a certain shorthand, you can
create it. `Oak` can similarly be a testing ground for new
language syntaxes. It supports the creation of "dialects", which
are `Oak` variants which use preprocessor rules to support
independent syntactical structures.

**Note: `Oak` has a default syntax, called canonical `Oak`.**
It also has the `std` dialect, provided by the `std` package.
The later is more commonly used. Syntax modifications must be
opted into on a file-by-file basis, or specified by the compiler
via the use of dialect files.

## Names and Prerequisites

Name      | Meaning
----------|-----------------------------------------------------
`Oak`     | The base `Oak` programming language.
`Acorn`   | The `Oak` translator / compiler.
`std`     | Short for `standard`.
`Sapling` | The sub-language for creating `Oak` dialects/rules.
Dialect   | A syntactically-modified branch of `Oak`.
`TARM`    | Prototype for more advanced rule system.

This document assumes a moderate level of knowledge about
programming languages and compiler design. A thorough
understanding of `C++` and `C`, as well as a cursory knowledge
of `Rust`, `Python` or `Carbon` are recommended (as these
languages use the same type specification syntax as `Oak`).

## Compilation, Installation, and Uninstallation

**`Oak` is not compatible with Windows.** `Oak` assumes a UNIX
system.

To install, open a terminal in this folder and run
`make install`. This will compile and install `Oak`, as well as
the standard `Oak` package. To uninstall, call `acorn -A`. To
update, call `acorn -a`.

## Versions of `Oak`

`Oak` versions follow the following format.

`major release`.`minor release`.`patch`

As of the time of this writing, `Oak` is still on major release
`0`. During this phase of development, `Oak` is **not stable**.
This means that code written in one minor release or patch is
**not guaranteed to work in another**.

Once `Oak` moves to major release `1`, code written in one patch
version will be guaranteed to work in another. Code written in
one minor release will most likely work in another. Code written
in one major release will have no guarantees about any other.

## For More Help

For examples on the concepts presented in this document, see the
`../src/tests/` directory included in this `git` repo. The files
therein are labeled `CONCEPT_test.oak`, where `CONCEPT` is the
concept covered within. These files are both demos and unit
tests for `Oak` and `acorn`. Consequently, you can compile all
demos via `make test` or `acorn -T`.

## Quirks of `Oak`

**If you read only one section, read this one.**

This section outlines things which make `Oak` syntax unlike
other common syntaxes. As mentioned above, this document
requires a moderate understanding of `C`/`C++` syntax, and thus
we will mostly focus on differences of this.

### Reference and Dereference Operators

In `C++` and `Rust`, the reference operator is `&` and the 
dereference operator is `*`. In `Oak`, the reference operator is
`@` and the dereference operator is `^`.

 Operator Name | `C` | `Oak`
---------------|-----|-------
 Reference     | `&` | `@`
 Dereference   | `*` | `^`

### Keywords For Variable/Function/Class/Enum Creation

In most languages, there are several keywords to denote the
creation of a function, enumeration, structure, or local
variable. `Python` has `def` and `class`, rust has `let`,
`fn`, `struct`, and `enum`, `JavaScript` has `let`, `function`,
and `var`, etcetera. `Oak`, unlike these languages, has only one
such keyword: `let`.

```rust
// Create a new struct
let name: struct {}

// Create a new enum
let name_2: enum {}

// Create a new function
let main() -> i32
{
    // Create a new variable
    let name_3: bool;

    0
}
```

### Array Order

In `Oak`, types are read strictly left-to-right. This means that
an array of integers is `[]i32`, not `i32 NAME[]` or `[i32]`.
`Oak` arrays always come immediately before the thing they
contain in `Oak` type syntax.

For example:
```rust
let arr_of_bools: []bool;
let sized_arr_of_bools: [123]bool;
let arr_of_bool_pointers: []^bool;
let pointer_to_arr_of_bools: ^[]bool;

let misc_1: [123][]i32;
let misc_2: []^[]^^^[][][]f64;
```

A sized array looks like this: `[size_here]i32`, where
`size_here` is a compile-time positive integer constant. An
unsized array, on the other hand, looks like this: `[]i32`.
Unsized arrays are assigned via the `alloc!` macro.

### Modern Generics / Templates

In `C++`, a template is verbosely defined as below.

```c++
template <class T>
void foobar(T what)
{
    // ...
}

```

In `Oak`, the template line `template <class T>` is replaced by
simply `<t>` (generic names must be lowercase).

```rust
let foobar<t>(what: T) -> void
{
    // ... 
}
```

### Explicit Template Instantiation calls

Unlike `C++` and `Rust`, `Oak` does not have implied template
instantiation calls. The template instantiator must be called
explicitly. For instance, the `C++` code

```c++
// Create foobar<i32> and call it
foobar<i32>(123);
```

would become the `Oak` code

```rust
// Ask the instantiator to create a function with this signature
foobar<i32>(_: i32);

// Call that function
foobar(123);
```

### Classes and `public` / `private` / `protected`

`Oak` does not have classes (only structs). Additionally, `Oak`
does not have any form of member protection. This means that all
struct member variables are always `public`. For a demonstration
of why this is the case, see `extra/tests/mischief.oak`.

**Note:** Informal private members are denoted as in `Python`,
with two underscores as a prefix. This is the same for local
helper functions in libraries.

```rust
let library_name::public_fn() -> void {}
let library_name::__private_fn() -> void {}

let library_name::struct_name: struct
{
    public_member: u128,
    __private_member: u128,
}
```

These members are **not actually private**, but are informally
treated as such. That is to say, you *shouldn't* modify them,
but you still *can*.

### Class Methods

In `Oak`, class member functions (methods) can (and must) be
defined outside of the class itself. This means that any coder
can add a new method to a struct at any point. For instance, you
could import the `string` library and add some new method to
turn a given string blue when printed.

`Oak` methods are shorthand in a way similar to `Python`.

```rust
let a: i32;

// This call
a.b();

// Is shorthand for
b(a);
```

### Mutability

`Oak` does not have the `const` keyword. Instead, everything is
`const` by default, and mutability is specified by passing the
address of something. Thus, `^i32` in `Oak` is equivalent to
`int` in `C`, and `Oak`'s `i32` is equivalent to `C`'s
`const int`.

## Syntax Overview

In `Oak`, a variable is declared as follows.

`let NAME: TYPE;`

For instance, a boolean named  `a` would be declared:

`let a: bool;`

You should read the `:` operator as "be of type". Thus, the
above statement reads "let a be of type bool". Functions are
declared as follows.

`let NAME(ARG_1: ARG_1_TYPE, ...) -> RETURN_TYPE;`

For instance, a function that in `C++` would be declared
`bool isBiggerThanFive(int a);` would in `Oak` be declared
`let is_bigger_than_five(a: i32) -> bool;`. See later for atomic
type conversions from `C` (IE how `int` became `i32` here). You
should read the `->` operator as "yield". Thus, this function
should be read "let is_bigger_than_five be a function taking
parameter a of type i32 and yielding a bool".

Additionally, the `C++` code

```cpp
bool isBiggerThanFive(int a)
{
    return a > 5;
}
```

would convert to

```rust
let is_bigger_than_five(a: i32) -> bool
{
    a > 5
}
```

Note that leaving off a semicolon is equivalent to `C++`'s
`return` statement. Equivalently, you can use the `return`
keyword. **You cannot have any form of `void` return in `Oak`.**
Void functions will only return at the end of their code scopes.

Pointers are `^` (IE a pointer to a bool is `^bool`). The "get
address" operator is @.

Syntax is fluid and user-modifiable in `Oak`. For more
information about this aspect, see the `Preproc Rules` section
below.

The `:`, `let` and `->` operators come from mathematics. For
instance, the mathematical statement

$$
    \verb|let | f: \mathbb{R} \to \mathbb{R}
$$

is equivalent to the code

```rust
let f(_: f64) -> f64;
```

Both of these statements can be read "let f be a function
mapping a single real number to another" (although `f64`s are an
extremely limited representation of the real numbers).

As a final example for this section, examine the "Hello world"
program below.

```rust
// Include the standard (std) Oak package, which contains print.
package!("std");

// The main function: The program will start by calling this.
let main() -> i32
{
    // Print "Hello, World!" to the terminal, followed by a
    // newline.
    print("Hello, World!\n");

    // Return a zero exit code, meaning no error.
    0
}

```

## Demo

```rust
// main.oak

// Import the `Oak` standard package
package!("std");

// Use the standard `Oak` rule set
use_rule!("std");

let main(argc: i32, argv: []str) -> i32
{
    print("This program was started with the command: ");

    print(ptrarr!(argv, 0));
    print("\n");

    print("Hello, world!\n");

    0
}

```

Commands (bash):

```bash
acorn main.oak -o hello.out
./hello.out
```

Output:

```bash
<Compiler output here>
This program was started with the command: .oak_build/a.out
Hello, world!
<Process finished with exit code 0>
```

## The `let` Keyword

`let` is one of `Oak`'s few keywords. It represents the adding
of something to the symbol table. This could be a variable, a
function, a struct, or an enumeration. It can even be templated.
`Oak`, unlike languages like `Rust` and `Python`, does not have
separate keywords for these actions. The possible usecases of
the `let` keyword are outlined below.

```rust
// Register `fn` as a function mapping nothing to nothing
let fn() -> void
{
}

// Register `var` as an instance of i32
let var: i32;

// Register `structure` as a struct
let structure: struct
{
    ,
}

// Register `enumeration` as an enum
let enumeration: enum
{
    ,
}

// Register `templated_fn` as a templated function
let templated_fn<t>() -> void
{
}

// Register `templated_structure` as a templated struct
let templated_structure<t>: struct
{
    ,
}

// Register `templated_enumeration` as a templated enum
let templated_enumeration<t>: enum
{
    ,
}

```

**Note:** Variables represent instances of a given type.
Instances, by nature, cannot be templated. Thus, there is no
templated case for the variable declaration syntax of `let`. All
other variants of `let` are considered abstract (they only tell
the compiler how to add something), but variable declaration is
considered concrete (it uses abstract information to create a
non-abstract instance).

## Pointers

Pointers are variables which hold a memory address. This address
could refer to a single object in memory, or the start of a
contiguous block of like-typed objects. To get the address of a
variable in `Oak`, use the `@` symbol. To get the value that a 
pointer references, use the `^` symbol. Pointer types are the
`^` symbol, followed by the type they point to. You can have
pointers to pointers.

The `alloc!` macro for allocating new memory on the heap returns
a pointer to the memory it allocated, should it be successful.
Similarly, the `free!` macro frees the memory of the pointer it
is passed.

Mutability (ability to change data) is determined by
referencing. If an object is passed into a function as a
parameter, it will only be modifiable if the function signature
designates it a pointer. When calling a function, a single
reference can be automatically added by the compiler, but no
more than  that. For example,

```rust
let fn_0(what: i32) -> void
{
    ;
}

let fn_1(what: ^i32) -> void
{
    ;
}

let fn_2(what: ^^i32) -> void
{
    ;
}

let main() -> i32
{
    let arg: i32;

    // Legal call
    fn_0(arg);

    // Also legal
    fn_1(arg);

    // NOT legal- will cause compiler error
    fn_2(arg);

    0
}

```

The address-of operator @ should be avoided unless strictly
necessary.

A pointer to an object can be used just like the object itself
with respect to its members. For instance, all the following are
legal calls.

```rust
let item: struct
{
    a: i32,
}

let main() -> i32
{
    let obj_1: item;
    let obj_2: ^item;
    let obj_3: ^^item;
    let obj_4: ^^^item;

    // All the following are legal
    obj_1.a;
    obj_2.a;
    obj_3.a;
    obj_4.a;

    0
}

```

## Arrays

`Oak` has a unique syntax for arrays that may be unfamiliar to
users of `C`, `C++` and `Rust`. In `Oak`, arrays prefix the type
they refer to. For instance, an array of integers is `[]i32`
instead of `i32[]`. `Oak` has two types of arrays: Sized and
unsized. Unsized arrays are syntactically equivalent to pointers
internally, and can point to an array of any size. These are the
only kind of array which can be passed as parameters, and the
kind that are assigned by `alloc!`. They are denoted with the
`[]` operator. Sized arrays are **not** pointers- They are the
actual arrays. These are denoted with the `[n]` operator, where
`n` is a compile-time constant integer. This means that an
integer variable **cannot** be used as the size of an array.

`Oak` types are **always** read left-to-right. Arrays are not
an exception (unlike in other languages). This makes it slightly
easier to decode exactly what an `Oak` array is of.

### Array Examples

`C`:
```c
i32 a[12];
i32 *b;
i32 *c[10];
i32 d[10][20];
```

`Oak`:
```rust
let a: [12]i32;
let b: []i32;
let c: [10]^i32;
let d: [20][10]i32;
```

In `Oak`, a pointer to a sized array is an illegal type.
However, a pointer to an unsized array is fine.

```rust
// Illegal!
let a: ^[5]i32;

// Fine
let b: ^[]i32;
```

You can also have arrays of function pointers as shown below.

```rust
// Array of function pointers
let l: []()->void;
```

Here are some more miscellaneous examples.

```rust
// Array of i32
let i: []i32;

// Array of i32 pointers
let j: []^i32;

// Pointer to array of i32 pointers
let k: ^[]^i32;

// Sized array of i32
let l: [5]i32;

// Sized array of i32 pointers
let m: [10]^i32;

// Pointer to a pointer to an array of pointers to arrays of
// pointers to pointers to arrays of arrays of pointers to i32s
let absurd: ^^[]^[]^^[][]^i32;

// Array of 10 of the above
let absurd_arr: [10]type!(absurd);
```

### Array Initialization

`Oak` cannot have uninitialized variables. Thus, arrays (sized
and unsized) have default values.

Unsized arrays, since they are just pointers to contiguous
arrays in memory somewhere, are initialized to the null pointer
`0`. This is how all pointers in `Oak` are initialized.

Sized arrays are **not** pointers- They are the actual
contiguous memory positions which unsized arrays point to.
**Their memory is initialized with the native `New` function.**

## Formatting

This section lists guidelines / rules for writing `Oak` code.
Deviation from the following is considered **non-canonical**
`Oak` formatting, and is unadvised. Deviation, in fact, could
easily be considered an error by the compiler.

Code scopes go like this:

```rust
// Good

if (true)
{
    // ..
}

let fn() -> void
{
    // ..
}
```

NEVER like this:

```rust
// Bad

if (true) {
    // ..
}

let fn() -> void {
    // ..
}
```

A rare exception is single-line functions. These are functions
which are so short that they can fit in a standard 96-character
`Oak` line. These are visually similar to `Python`'s lambda
functions. They will usually not use the `return` keyword.

```rust
// A valid single-line function:
let a_times_b(a: i32, b: i32) -> i32 { a * b }
```

Ensure you always have some sort of return statement (except
`-> void` functions), even when it is unreachable. Principles
take precedent over literal compiler interpretation.

`Oak` exclusively uses underscored variable names. Camelcase
variable names should never be used, and capitalized variables
are reserved for special cases.

```rust
// Good
let long_variable_name_with_multiple_words: i32;

// Bad
let longVariableNameWithMultipleWords: i32;
let longvariablenamewithmultiplewords: i32;

// Only allowed for methods which use dynamic allocation
// or for operator method aliases
let LongVariableNameWithMultipleWords: i32;

// Why? No?!?
let Long_variable_name_with_multiple_words: i32;
```

The same naming tips go for structs and enums. Structs and enums
should not have any special name conventions. The only symbols
which have different naming conventions are the operator aliases
(see later), which use upper camelcase.

These naming conventions are also extended to package names
(unless for some reason incompatible with the git host website
they are on) and file names. Unlike standard practice in `C++`
and most other languages, generics types (see later) should
still be lower-underscore-case.

Single-line comments should start with "`// `" (slash, slash,
space), and multi-line comments should start with `/*`
(slash-star-newline) and end with `*/` (newline-star-slash)
(just like `C++`).

```rust
// Correct single line comment
//Do not format them like this (no leading space)

// single-line comments should NEVER start with a lower-case

// Except in this case; When
// it is a continuation of 
// a previous line for
// formatting reasons, a
// lowercase start is fine.

/*
Correct multi-line comment.

The same wrap-over capitalization
rules apply.
*/

/*
incorrect multi-line comment
(no leading capitalization)
*/

/*Don't do this (why would you do this?)*/

/*Also do not
do this*/

/*
    If this is prettier in context, it is also valid.
    However, it is not the default.
*/

// The following is a valid (and encouraged) section break:
////////////////////////////////////////////////////////////////
// (Even though it does not have a leading space)
```

Also, all files should always end with a newline.

Comment blocks before global definitions (those not within a
code scope) will be included in automatic manual generation via
`acorn -m`. These manuals use standard **markdown**, and by
extension all comments should as well.

Any sequence starting with `#` is ignored completely. This is to
allow for shebangs. This symbol should **never** be used for
commenting.

```rust
// This is fine:
#!/usr/bin/acorn -E

// This is bad:
# Hello!
```

Tabbing should follow standard `C++` rules. Tabs and
quadruple-spaces are both acceptable, but double-spaces are not.
Not using tabbing is not acceptable.

No line of `Oak` should be longer than 96 standard-width chars
wide. Since newlines are erased by the lexer, one can be
inserted at any point without disturbing syntax.

If possible you should use a spellchecker when writing code.
If a variable name does not pass a spell checker, it is **not**
considered a valid name and should be changed.

Global variables are illegal. Anything achieved by globals can
be done better by passing arguments.

All `Oak` should be as minimalist as possible, and should be
split into as many files as it makes sense to do.

Variable, package, and file names may be abbreviated within the
sake of reason. An abbreviation is disallowed if it conflicts
with another name or if the original name cannot be reasonably
extracted from it (for instance, the abbreviation `regex` is
fine, but `re` is not, because `re` could realistically refer to
anything).

Interfacial files should take the form `NAME_inter.oak` and
`NAME_inter.other_language_suffix_here` (see later for more
information on formatting `Oak` interface files). For instance,
`sdl_inter.oak` and `sdl_inter.cpp`.

If multiple lines are required within a function call, the
following lines should be tabbed exactly one further than the
beginning line. Whenever possible, you should include exactly
one argument per line in such a case.

```rust
// Proper formatting:
let long_function_name(long_argument_name_one: i32,
    long_argument_name_two: i32,
    long_argument_name_three: i32,
    long_argument_name_four: i32) -> i32
{
    0
}

// Also fine:
let long_function_name(
    long_argument_name_one: i32,
    long_argument_name_two: i32,
    long_argument_name_three: i32,
    long_argument_name_four: i32
    ) -> i32
{
    0
}

```

`Oak` code can technically be written using unicode encoding,
but ASCII is recommended for any programmer whose spoken
language can be expressed within it.

**Note: `Oak` is hypothetically especially translatable to**
**other human languages due to the preprocessor rule system.**
It is trivial to implement a dialect file which replaces all
`Oak` keywords with any spoken language's equivalent versions,
and still fairly easy to translate the `std` package. For
example, an `Oak` program written in Hindi would experience a
translation process as follows:

`oak_hindi_dialect.oak -> oak_canonical_dialect.oak`
` -> c_version.cpp -> executable.exe`

## Unicode

Unicode works perfectly fine in `Oak`, although the later was
not designed with the former in mind. International coders
should have no troubles.

If `Oak` reports unicode characters as illegal, please report it
as a bug.

## Main Functions

The main function must take one of the following forms (you can
change the argument names). `argc` is the number of command line
arguments, at minimum 1. `argv` is a pointer to an array of
strings, each of which is a command line argument. The first
item of this, `argv[0]`, is the name of the executable that was
run.

```rust
let main() -> i32;
let main(argc: i32, argv: []str) -> i32;
let main(argc: i32, argv: [][]i8) -> i32;
```

Note that these two arguments can be named anything. A common
form is `(c: i32, v: [][]i8) -> i32`.

## Canonical `Oak` Vs `std`-dialect `Oak`

Canonical `Oak` is fairly limited. It has very few "creature
comforts". However, it compiles faster. `std` `Oak`, on the
other hand, has many extra conveniences, but compiles slower.
`std` `Oak` is activated by the code line `use_rule!("std");`
(provided that the `"std"` package has already been imported).
The following outlines some differences between canonical and
`std` `Oak`.

### Implicit void return

Without `std`:
```rust
let fn_which_returns_void() -> void
{
}

```

With `std`:
```rust
// `-> void` is implied
let fn_which_returns_void()
{

}
```

### Automatic parentheses for `if`, `while`, and `match` statements

Without `std`:
```rust
if (condition)
{
    while (condition)
    {
        match (enum_instance)
        {
            // ...
        }
    }
}
```

With `std`:
```rust
if condition
{
    while condition
    {
        match enum_instance
        {
            // ...
        }
    }
}
```

### Object method "dot" call notation

Without `std`:
```rust
let obj: struct
{
    member: i32,
}

let fn(self: ^obj) -> void
{
    print(self.member);
}

let main() -> i32
{
    let inst: obj;

    fn(inst);
    
    0
}
```

With `std`:
```rust
// <as above>

let main() -> i32
{
    let inst: obj;

    inst.fn();
    
    0
}
```

### "Easy" method declarations

Without `std`:
```rust
let obj: struct
{
    // ...
}

let fn(self: ^obj) -> void
{
    // ...
}
```

With `std`:
```rust
let obj: struct
{
    // ...
}

let obj.fn() -> void
{
    // ...
}
```

### Inline instantiation

Without `std`:
```rust
let a: i128;
a = 123;
```

With `std`:
```rust
let a: i128 = 123;
```

### Implied typing

Without `std`:
```rust
let fn() -> i32
{
    5
}

let main() -> i32
{
    let a: i32;
    a = fn();
    
    // Or
    let a: type!(fn());
    a = fn();

    0
}
```

With `std`:
```rust
let fn() -> i32
{
    5
}

let main() -> i32
{
    // Automatically types `a` as an i32
    let a = fn();

    0
}
```

### `for` loops

Without `std`:
```rust
let i: i32;
i = 0;
while (i < 100)
{
    // ...
    i += 1;
}
```

With `std`:
```rust
for (let i: i32 = 0; i < 100; i += 1)
{
    // ...
}
```

### Namespaces

Without `std`:
```rust
let library_fn() -> void;
```

With `std`:
```rust
// `::` becomes `_`
let library::fn() -> void;
```

**Note:** There is no `using` or `use` in `Oak`. If namespaces
are to be used, they must always be fully qualified. Namespaces
are **required** when developing non-`std` packages.

## Object Oriented Programming

`Oak` does not have classes, nor does it have internally defined
methods. Methods are converted into equivalent function calls
during translation as follows.

```rust
OBJ.METHOD_NAME(ARG, ARG, ...);
```

Becomes

```rust
METHOD_NAME(@OBJ, ARG, ARG, ...);
```

Thus, you can define a method as follows.

```rust
let OBJ_TYPE.METHOD_NAME(...) -> RETURN_TYPE;
```

For instance, if you wanted to define a method that converts an
integer to a double, its signature would be:

```rust
let i32.to_double(self: ^i32) -> f64;
```

This frees the programmer from the bounds of the initial class
definition.

In `Oak`, you can define new data structures as structs, and
define any methods upon it later. For instance, a linked list
could be broadly defined as follows.

```rust
// Alloc is defined in std's std_mem.oak
package!("std");

let linked_list<t>: struct
{
    data: t,
    next: ^linked_list<t>,
}

let append(self: ^linked_list<t>, what: t) -> void
{
    // Alloc is equivalent to `C++`'s new keyword
    alloc!(next);
    next.data = what;
}
```

`Oak` does not have private members, nor does it have
inheritance.

## Division Of Labor

`Oak` does not have explicit header files like `C` / `C++`, but
there is no reason why you could not use a `.oak` file like a
`.hpp / .h`. For example, a `.oak` can establish function
signatures without explicitly defining them, allowing another
`.oak` to define them. This allows easy division of labor, as in
`C / C++`. This is obviously vital for any project of scale.
Additionally, the translator will detect and prevent circular
dependencies, eliminating any analogy to `pragma once`.

## Memory Safety and Heap Memory Allocation

The `alloc!` and `free!` functions are akin to `C++`'s `new` and
`delete` keywords, respectively. Alloc requests a memory
position on the heap with the size of `t`, and free
correspondingly releases that memory. `Oak` is not memory safe,
in that it does not automatically clean up `alloc!`-ed memory
like `Java` or `Rust`. In this way it is more similar to `C++`
or `C`.

Example of `alloc!` and `free!`:

```rust
let New<t>(self: ^node<t>) -> void
{
    // Allocate a single instance on the heap
    alloc!(self.data);
    free!(self.data);

    // Allocate an array of size 5 on the heap
    alloc!(self.data, 5);

    // Free a dynamically allocated array
    free!(self.data);
}

let Del<t>(self: ^node<t>) -> void
{
    free!(self.data);
}

let main() -> i32
{
    let i: ^bool;
    alloc!(i);
}
```

If a number is provided as a second argument to `alloc!`, it
returns an **unsized array** of the newly allocated values.
Otherwise, it returns a **pointer** to the single newly
allocated space.

## Interoperability With C

You can declare only the function signatures in `Oak` and define
them in `C`, as is done in the `Oak` `std` (standard) package.
This is called **interfacing**. These pairs of dual-language
files are **interfacial files**, and any package primarily
porting one language's features to `Oak` is an
**interfacial package**.

Since `Oak` is translated into `C` for compilation, there is
also the `raw_c!()` macro, which writes the quote-enclosed
contents within as raw `C` code. This is useful for more
advanced rules and dialects.

**Note:** You will have to follow the `Oak` mangling process, as
detailed below.

## Conforming to the Oak Mangler

The `Oak` function signatures

```rust
let foo_bar(self: ^data, what: data, hello: ^^data) -> data;
let hello_world() -> void;
let fn_ptr_thing(fn: ^() -> void) -> void;
```

Will, upon translation to `C`, become

```c
// Note how the new "mangled" function name contains the entire type
// Argument types are separated by JOIN, and the argument section ends
// with MAPS, followed by the return type.
struct data foo_bar_FN_PTR_data_JOIN_data_JOIN_PTR_PTR_data_MAPS_data(
    struct data *self, struct data what, struct data **hello);

void hello_world_FN_MAPS_void(void);

// This is how function pointers get mangled
void fn_ptr_thing_FN_PTR_FN_MAPS_void_MAPS_void(void (*fn)(void));
```

Thus, in an interfacial file, you should follow the
above pattern. For higher-level languages like `C++` and `Rust`,
you will have to manually disable the mangler upon compile time
in order for interfacing to be successful. For `C++`, this
comes in the form of the `extern "C"` statement.

**Note:** `Oak` cannot see templates of other languages, nor can
other languages see `Oak` templates. `Oak` namespaces are
shorthand for underscores, so they can be interfaced at a low
level like anything else. For example, the `Oak` interfacial
code:

```rust
let math::sin(x: f64) -> f64;

// Is the same as

let math_sin(x: f64) -> f64;
```

Can interface with the `C++` code:
```cpp
f64 math_sin_FN_f64_MAPS_f64(f64 x)
{
    // ...
}
```

## Operator Overloading / Aliases

Where in `C++` you would write

```cpp
class example
{
public:
    void operator=(..);
    bool operator==(..);
    // .. etc
};
```

In `Oak` you would write

```rust
// The empty struct; No members
let example: struct
{
    ,
}

let Copy(self: ^example, ..) -> void;
let Eq(self: ^example, ..) -> bool;
```

There are many so-called "operator aliases" which are listed
below.

`Oak`  | `C++`     | Description
-------|-----------|------------------------
Less   | <         | Less than
Great  | >         | Greater than
Leq    | <=        | Less that or equal
Greq   | >=        | Greater than or equal
Eq     | ==        | Equal to
Neq    | !=        | Not equal to
And    | &         | Bitwise and
Andd   | &&        | Boolean and
Or     | \|        | Bitwise or
Orr    | \|\|      | Boolean or
Not    | !         | Boolean negation
Copy   | =         | Copy constructor
Del    | ~         | Deletion
Add    | +         | Addition
Sub    | -         | Subtraction
Mult   | *         | Multiplication
Div    | /         | Division
Mod    | %         | Modulo / remainder
AddEq  | +=        | Increment by a number
SubEq  | -=        | Decrement by a number
MultEq | *=        | Multiply and assign
DivEq  | /=        | Divide and assign
ModEq  | %=        | Modulo and assign
AndEq  | &=        | Bitwise AND and assign
OrEq   | \|=       | Bitwise OR and assign
Lbs    | <<        | Left bitshift
Rbs    | >>        | Right bitshift
New    | TYPE_NAME | Instantiation
Del    | N/A       | Deletion

**Note A:** There is no `Oak` equivalent to `C++`'s increment
and decrement operators, although such a rule would be trivial
to write.

**Note B:** There is not a set return type for many of these.
For instance, it is common to see `copy` return the type of the
item copied into, the type of the item copied from, or `void`.

**Note C:** No `std` `Oak` files use `C++`-style streams, but
there is nothing in the language stopping them from working.
However, the obfuscation they add to programmers only familiar
with modern languages most likely outweighs any benefit they
might provide.

## Atomic Types

The following are the built-in types of `Oak`, with their `C++`
equivalents.

`C++`               | `Oak`
--------------------|-------
unsigned char       | u8
char                | i8
unsigned short      | u16
short               | i16
unsigned int        | u32
int                 | i32
unsigned long       | u64
long                | i64
unsigned long long  | u128
long long           | i128
float               | f32
double              | f64
const char *        | str
bool                | bool
T&                  | ^t
T[]                 | []t
T[123]              | [123]t
T*                  | ^t
const T             | t
template\<class T\> | \<t\>

The last few entries show two things; That `Oak` does not have
references (they are replaced with pointers), and that `Oak` has
smart templating. Additionally note that there are no multi-word
types (IE unsigned long long int) in `Oak`, and that pointers
and arrays are moved to before the type they point to.

For instance,

```cpp
template <class T, class F>
unsigned long long int **doThing(T &a, T b[]);
```

Becomes

```rust
let do_thing<t, f>(a: ^t, b: []f) -> ^^u128;
```

## Advanced Literals

`Oak` has built-in support for hexadecimal, decimal, and binary
integer literals. Hex numbers are prefixed by `0x`, binary by
`0b`, and decimal by nothing. Additionally, the `Oak` lexer will
merge successive integer literals together, so spacing can be
inserted anywhere within an integer literal. This is extremely
necessary for readability.

The following code segment demonstrates these ideas.

```rust
package!("std");

let main() -> i32
{
    let j: u32;

    // Binary
    j = 0b10101010101010101010101010101010;

    // Hex
    j = 0x01234567;

    // Separation: All of the following are valid.

    j = 123 456;

    j = 0b 0000 1111 0101 1010 0000 1111 0000 1111;
    
    j = 0b11111111111111111111111111111111;
    
    j = 0x 01 23 45 67;

    printf!("j: %\n\n", j);

    0
}

```

Similarly, strings are automatically concatenated over spaces or
lines.

```rust
let long_string! = "hello! This is a very long string which it"
                   "is necessary to split over several lines.";

let split_string! = "we can also" "do this";

```

## Generics

You can declare a generic function (a function which can operate
on a generic type, rather than a specific, defined type) by
using the `<t>` notation as below.

```rust
// Where t is the generic type to be inserted later
let generic_fn_demo<t>(arg1: t, arg2: bool, arg3: *t) -> t;
```

This allows a generic type `t` (you can use whatever name you
want) to enter the function's scope temporarily. On a compiler
level, generic functions do not exist until they are called.
Upon compiler detection of a call, it is ensured that an
appropriately-typed function exists (otherwise, such a function
is **instantiated**, or created based on the given template).

`Oak` does not have automatic instantiation of generic functions
via argument type analysis. You must manually call the template
instantiation unit as follows.

```rust
let gen_fn<t>(what: t) -> void
{
    print(t);
}

let main() -> i32
{
    // Call the template instantiator
    // Request the instantiation of the generic block
    // identified by the signature `gen_fn<i32>(what: i32)`
    // which is our gen_fn for the i32 type
    gen_fn<i32>(what: i32);

    // Only now that the function has been instantiated is this
    // call valid.
    gen_fn(123);

    0
}
```

**Note:** It is common to replace the names of any and all
argument in the instantiator call with the null-name token `_`.
This is equivalent to writing anything else there, but signals
to the programmer that it is an instantiation call.

Similarly, you can define **generic structs** as follows.

```rust
let node<t>: struct
{
    data: t,
    next: ^node<t>,
}

let main() -> i32
{
    // Generic instantiation call
    node<i32>;

    let head: node<i32>;
}
```

Generic enumerations are defined exactly like structs (see later
for more information on enumerations). See later in this
document for more about instantiating generic structs.

**Note:** Generic structs and enumerations *can* be
automatically instantiated; That is to say, the `node<i32>;`
line in the above code is redundant. This is not the case for
generic functions, as mentioned earlier.

## Acorn

Acorn is the `Oak` translator, compiler, and linker. `Oak` is
first translated into `C`, which is then compiled and linked.

Acorn command line arguments:

Option | Verbose     | Purpose
-------|-------------|-------------------------------
 -a    |             | Update acorn
 -A    |             | Uninstall acorn
 -c    | --compile   | Produce object files
 -d    | --debug     | Toggle debug mode
 -D    | --dialect   | Uses a dialect file
 -e    | --clean     | Toggle erasure (default off)
 -E    | --execute   | Run executable when done
 -g    | --exe_debug | Use LLVM debug flag
 -h    | --help      | Show this
 -i    | --install   | Install a package
 -l    | --link      | Produce executables
 -m    | --manual    | Produce a .md doc
 -M    |             | Used for macros
 -n    | --no_save   | Produce nothing
 -o    | --output    | Set the output file
 -O    | --optimize  | Use LLVM optimization O3
 -q    | --quit      | Quit immediately
 -Q    | --query     | Query an installed package
 -r    | --reinstall | Reinstall a package
 -R    | --remove    | Uninstalls a package
 -s    | --size      | Show Oak disk usage
 -S    | --install   | Install a package
 -t    | --translate | Produce C++ files
 -T    | --test      | Compile and run tests/*.oak
 -u    | --dump      | Save dump files
 -U    |             | Save rule log files
 -v    | --version   | Show version
 -w    | --new       | Create a new package
 -x    | --syntax    | Ignore syntax errors

`acorn` can take in any number of input files, but can target
only one output (`.o`, `.c`, or `.out`) file.

## Optimization and Runtime Debugging

`Oak` has some limited support for compiler optimization and
runtime debugging via `clang`. The `acorn -O` and `acorn -g`
commands tell `clang` to use full compile-time optimization and
to allow runtime debugging respectively.

**Due to `Oak`'s mangler and rule system, runtime debugging**
**information may not be immediately useful.** For instance,
generic struct instances have compilation-time-mangled names:
`vec<i32>` will become `vec_GEN_i32_ENDGEN`, which is much less
intuitive to a human observer. Additionally, compile-time rules
may cause strange effects in the output code which may not be
present in the source. Enumerations, due to their complex
internal structure, also may be hard to parse to a human eye.
However, with practice and use of the `acorn -e` flag (to view
the translated `c++` source code) this information should become
easier to parse.

Additionally, it is worth noting that both of these options will
increase compilation time, and the `-g` option will
significantly increase the size of the output executable. It is
often also useful to examine the output `acorn` logs and `C`
target files to determine what has gone wrong during debugging.

## Macros

Macros in `Oak` are code which write code. Compiler macros in
`Oak` are macros which specifically control the compiler.

The following are the standard macros associated with Acorn.

`include!(WHAT, WHAT, ...)` causes the translator to also use
the file(s) within. It can take `.oak` files. It will first look
for local files. However, if no local file exists, it will check
the `/usr/include/oak` folder. For instance, if `include!` is
called on `std/opt.oak`, `acorn` will first look for a local
`./std/opt.oak` file. If none exists, it will check
`/usr/include/oak/std/opt.oak` (which is part of the standard
`Oak` package).

`package!(WHAT, WHAT, ..)` is a more advanced version of the
above. See the packages section below for more details.

`link!(WHAT, WHAT, ...)` causes the compiler to also use the
file(s) within. It takes `.o` files, or anything that can be
used by a standard `C` linker. You can, for example, write a
function header with no body in a `.oak` file, and add a `link!`
macro containing the function definition, perhaps compiled in
another language. This makes integration with `C / C++` files
extremely easy.

You can also define your own macros as follows. If you wanted to
create a macro named `hi!` which expands to
`print("hello world")`, you would do the following.

Macro arguments are always treated as strings. For instance,
`link!(foobar)` is equivalent to `link!("foobar")`. You are
encouraged to put quotations around any filenames.

```rust
let hi!(argc: i32, argv: [][]i8) -> i32
{
    // Each macro is its own translation unit, so compiler
    // macros must be included inside them. Compiler macros
    // called outside of here will not carry over.

    include!("std/std_io.oak");

    print("print(\"hello world\")");

    0
}
```

A macro is compiled separately, so if you want to use a included
file within, you will have to call its inclusion macro again.
You will note that a macro has the same type as a main function;
This is why.

If you wanted to create a macro named `print_five_times!()`
which prints the name of a symbol 5 types, you would do the
following.

```rust
let print_five_times!(argc: i32, argv: []str)
{
    package!("std");

    if (argc != 2)
    {
        throw(error("Invalid number of arguments."));
        1
    }

    print(argv[1]);
    print(argv[1]);
    print(argv[1]);
    print(argv[1]);
    print(argv[1]);

    0
}
```

If you were to call `print_five_times!(a)`, it would expand into
`aaaaa`. If you were to call `print_five_times!(a[b].c)`, you
would receive `a[b].ca[b].ca[b].ca[b].ca[b].c`.

## Packages

The `package!(WHAT)` macro imports a package. If it is not yet
installed on the compiling device, it can be cloned via Git by
using `acorn --install PACKAGE_URL`. You can update or reinstall
a package via `acorn --reinstall PACKAGE_URL`. A few packages
have known URLs, meaning you can just say `acorn --install NAME`
rather than the full URL. Installed packages keep their files in
`/usr/include/oak/NAME`.

**Note A:** You can use `--install` or `-S`.

**Note B:** All packages must be strictly unique in their names,
and package names must contain only characters which are valid
in UNIX directory names.

### Packages and Namespaces

While namespaces (denoted in most languages by the `::`
operator) are not native to `Oak`, the `std` dialect provides
them. **`Oak` packages (except `std`) must use these.** All
structs, enums, and non-method functions **must** begin with the
prefix `NAME_`. For instance, all such symbols in the `stl`
(standard templated library) package begin with `stl_`, followed
by their name. The `list` data structure is then `stl_list`.
When using the namespace rule provided by the `std` dialect
(or via the `std/ns.oak` file and `namespace` rule), this can
be equivalently written `stl::list`. This naming convention is
expected for all packages which are not `std`. The `std` package
is allowed to be namespace-free.

**Note:** In `Oak`, all namespaces must always be fully
qualified. While in languages like `C++` you may say
`using namespace std` to automatically include all items from
the `std` namespace, `Oak` has no such statement.

### Creating Packages

Every package must have a file named `oak_package_info.txt`.
This file takes the following form.

```sh
NAME = "name_goes_here"
VERSION = "0.0.1"
LICENSE = "license"
DATE = "July 14th, 2023"
SOURCE = "example.com/example_package"
PATH = "./path/within/source/to/package"
AUTHOR = "Jordan Dehmel, Joe Shmoe"
EMAIL = "jdehmel@outlook.com, example@example.com"
ABOUT = "A demo of Oak package creation"
INCLUDE = "main_file_to_link_to.oak,another_file.oak"
SYS_DEPS = "package1 package2"
OAK_DEPS = "some oak packages here"
```

`NAME` is the name of the package. `VERSION` is the version
number of the package (For instance, major release 1, minor
release 15, patch 12 would be `1.15.12`). `DATE` is the date
this version was last updated. `SOURCE` is the URL where the
package's Git repo can be found. `AUTHOR` is the string which
contains all the package author names, separated by commas.
`EMAIL` is the same, but for emails. `ABOUT` is the package
description.

`INCLUDE` is the most important field- It contains the filepaths
(relative to the main folder of the repo, IE `./`) of all files
which will be directly `include!()`-ed by Acorn at compile-time.
If this is left empty, the `package!()` macro will not do
anything.

For instance, the `STD` (standard) library has a central linking
file named `std.oak`. This file contains `include!` macros for
all other files in the package. Thus, only `std.oak` needs to
linked at compile time- the rest will be taken care of
automatically. Thus, `std/oak_package_info.txt` has a line
reading `INCLUDE = "std.oak"`.

`SYS_DEPS` is a string containing space-separated packages
required to be installed by your operating system. For instance,
if a package used SDL2 and Git, it would include the line
`SYS_DEPS = "SDL2 git"`.

Packages are **only** managed by `Git`. The canonical host
website is currently `github`.

## Editing Oak

If you use VSCode, you can add this to your
`.vscode/settings.json` file.

```json
"files.associations": {
        "*.oak": "rust",
    }
```

`Rust` is similar enough to `Oak` that you won't notice any
major issues, so long as you don't enable any `Rust` extensions.

## Preproc Definitions

There are several so-called preprocessor definitions which can
be called upon. These will be replaced with their values before
compilation. Each of these begin and end with two underscores.

Name             | Type | Description
-----------------|------|----------------------
line!            | i128 | The current line
comp_time!       | i128 | The UNIX time of compilation
prev_file!       | str  | The path of the previous `Oak` file
file!            | str  | The path of the current `Oak` file
sys!             | str  | The operating system compiled for

Additional preprocessor definitions can be defined as follows.

```rust
let name! = definition;
```

Following the above definition, all occurrences of `name!` will
be replaced by `definition`.

**Note:** Preproc definitions can never be redefined. This is
also true of macros.

## Preproc Rules and Sapling

### Outline

**Note:** Use with caution. Rules alter the fundamental syntax
of the language.

The following subsection describes the use of the `Sapling` rule
engine. Additional rule engines are available via minor changes
to the `acorn` source code.

Rules are a feature which allows you to add more functionality
to `Oak`. They take in a pattern of symbols, and replace matches
with a different pattern when they is detected in the code. A
pattern definition is mostly like a fragment of `Oak` code, save
for a few special features.

The default symbol type in a rule is a literal. These match only
the symbol that they are. For instance, `cat` would only match
symbols that say `cat`. It would not match a symbol that said
`catterpillar`, even though the pattern is a prefix of the
latter. This is because rules operate on symbol-wise pattern
matching, not character-wise pattern matching.

The second type of symbol in a rule is a variable. These take
the form of a dollar sign followed by a single alphanumeric
character (in regex, `\$[a-zA-Z0-9]`). Variables can only use
single-character names, so there are a maximum of `62` variables
per rule. Whatever single symbol is matched into a variable will
be accessible in the output pattern.

The final type of symbol in a rule is a wildcard, represented by
two dollar signs. A wildcard matches anything, but is not stored.

The dollar sign was chosen over the traditional "glob" `*`
operator because it is not used within usual `Oak` code. The
glob also represents multiplication, so it is not used. Using
the dollar sign means that there are no backslashes needed in an
`Oak` rule.

Output patterns do not use wildcards. Upon replacement, any
occurrence of a defined variable in the output pattern will be
replaced with that variable's match in the input pattern.

For instance, the input rule `meow $$ $m meow` would match
`meow woof bark meow`, or indeed any other two symbols
surrounded by `meow`'s. The difference comes only in the output
pattern; `$s` would map to `bark`, but `woof` would never be
accessible. As far as the output pattern is concerned, that `$$`
could have matched anything.

Input and output patterns are lexed, so spaces may or may not be
required within.

In a way, rules allow you to define whatever syntactic fluff you
want. The rule-definition language outlined here is tentatively
called `Sapling`. This may be stylized `$apling`.

### Defining and Using Rules

Rules are defined using the `new_rule!` macro. This takes three
arguments; The name of the rule, the input pattern, and the
output pattern. For instance, a rule named `foo_to_bar` with an
input pattern `foo` and an output pattern `bar` would be
declared `new_rule!("foo_to_bar", "foo", "bar");`. Note that all
arguments to rule macros **must** be strings. This is advisable
(but not strictly required) of all macros.

Once a rule is defined, it will be inactive. To make a rule 
active, you must call the `use_rule!` macro. This takes one or 
more rule names, and activates those rules from that point on. 
Note that these rules are only active *after* this macro is 
called.

The set of active rules does not roll over when a new file is 
called, but the set of all rules does. This means that a 
included file will have access to its "parent" files' rules, 
but they will always be off by default. The `use_rule!` macro 
will need to be called within every file which wants to use a 
rule.

### Removing Rules

A rule can be deactivated (but never deleted) using the 
`rem_rule!` macro. This takes zero or more arguments. If zero 
arguments are given, it disables all rules. If one or more, it 
disables the rules with the given names.

### Rule Bundles

Multiple rules can be bundled together under one name by using 
the `bundle_rule!` macro. This takes two or more arguments, with
 the first argument being the name of the bundle. The remaining 
 arguments are the names of the rules which fall within this 
 bundle. This allows you to use a single `use_rule!` call to 
 activate a hole suite of rules.

Bundles cannot be disabled all at once; You will need to call 
`rem_rule!` for each of their component rules. A bundle with a 
given name will always overwrite a rule with the same name, 
except in a `bundle_rule!` call. A rule can never overwrite a 
macro call, but can overwrite anything else. Rules cannot find 
special / comment symbols (like the secret `//__LINE__=` 
symbol).

### Example

```rust
// File "source.oak"

package!("std");

new_rule!("everything_to_meow", "$$", "meow");
new_rule!("remove_meow", "meow", "");

bundle_rule!(
    "remove_everything",
    "everything_to_meow",
    "remove_meow"
);

use_rule!("remove_everything");

these symbols will be destroyed by the meow menace

```

Any file which calls `include!("source.oak");` would have access
to the `remove_everything` rule, but it would always be off by 
default.

Here is the source code of the `std_method` bundle, which does 
automatic method reformatting.

```rust
new_rule!("argless_mut_method", "$a . . $b ( )", "$b ( @ $a )");
new_rule!("argless_method", "$a . $b ( )", "$b ( $a )");
new_rule!("mut_method", "$a . . $b (", "$b ( @ $a ,");
new_rule!("const_method", "$a . $b (", "$b ( $a ,");

bundle_rule!("std_method",
    "argless_mut_method",
    "argless_method",
    "mut_method",
    "const_method");
```

### Stored Globs

As of version `0.0.2`, `Sapling` also has several more features. 
On such feature is `$+`, which is a wildcard that matches any 
one or more symbols. It may not be the first or last symbol in a 
`Sapling` sequence, as it is terminated only by the first 
instance of the following symbol (if it were the first item, it 
would be highly inefficient). Another such, with the same rules, 
is `$*`. This symbol matches any zero or more symbols that are 
not the following symbol. With these two symbols, no occurrence 
of the following symbol may match anywhere within. Additionally, 
a variable may not occur directly after either.

`hi $* ( )` would match `hi hello hello hello hello ( )`, but 
not `hi hello hello ( hello ) ( )`.

This update also introduces a stored glob operator `$*X` with 
`X` being another single-character variable name. For instance, 
`if $*a {` -> `if ( $a ) {` would auto-parenthesize `if` 
statements.

```rust
// STD Oak dialect rules
clear

"if $*a {" "if ( $a ) {"
"while $*a {" "while ( $a ) {"

// NOT final; We want to use STD_OD as a jumping-off point
```

It also introduces the merge operator `$<` for output patterns. 
This fuses the preceding and proceeding symbols, without any 
separation. For instance, `( $t )( $*f )` -> `to $< $t ( $f )` 
would convert standard cast notation into the `Oak` equivalent.

### Rule Memory

During the matching of a rule via `Sapling`, the engine will 
keep track of the history of the attempt thus far. This history 
is called the rule **memory**. You can reset the memory with the 
`$~` card, or pipe the entire contents into a variable with the 
`$>X` card (where `X` is the variable).

### Pair Matching

Consider the following rule, taken from an old version of the 
`std` `Oak` package.

```rust
new_rule!(
    "for",
    "for ( $*a ; $*b ; $*c ) { $*d }",
    "{ $a ; while ( $b ) { $d $c ; } }"
);

```

A modified version of this still underlies the `Oak` `for` loop
system. However, what happens if we embed an `if` statement
within a for loop in the above example? The rule would stop
tracking `$d` as soon as it encountered the ending curly brace
of the `if` statement, possibly breaking all following code. We
need a card which can handle pairs of opening and closing
symbols. `$<$OPEN$CLOSE$>` is just such a card. If we replace
`OPEN` with an opening curly brace and `CLOSE` with a closing
one, this card will recognize any one or more nested set of
curly braces and move beyond them (more specifically, it will
maintain an internal counter, incrementing it upon `OPEN`,
decrementing it upon `CLOSE` and only ceasing once it reaches
zero). This can be use in conjunction with rule memory to store
the entire set of nesting braces in a variable.

Below is the modern `Oak` equivalent of this rule.

```rust
new_rule!(
    "for",
    "for ( $*a ; $*b ; $*c ) $~ $<${$}$> $>g",
    "$a ; while ( $b ) { $g $c ; }"
};

```

This version can recognize infinitely nested sets of curly
brackets with no issues.

Note: The `Sapling` interpreter is somewhere between a
deterministic finite automata and a push-down automata.

### Suites

A **suite** is a set of multiple literals, any of which can be
matched in a given position. A suite is denoted as follows.

```txt
$[$each$of$these$would$match$]
```

Similarly, a **negated suite** matches any single symbol
*except* those within.

```txt
$/[$none$of$these$would$match$]
```

For both of these, literals are delineated by the dollar sign.

### Negative Lookbehind and Lookahead

A **negative lookbehind**, denoted by

```txt
$/<$literal
```

causes a pattern fail if the previous character was the
specified literal. It does not advance the current position, so

```txt
$/<hi $/<hey hello
```

Would match any occurrence of hello not prefaced by hi **or**
hey.

Similarly, there is the **negative lookahead**, denoted by

```txt
$/>$literal
```

This works identically to the negative lookbehind, except with
the following symbol instead of the preceding one.

Note: There is no positive lookahead or lookbehind in Sapling.

## Dialects

Dialect files introduce rules that are handled before all others
and which are always enabled by the translator. This allows for
the creation of `Oak` **dialects**, or syntactically different
languages which use rules to collapse into regular `Oak` at
translation-time.

Dialect files (`.od` files, for **Oak Dialect**) consist of a
few types of line. There can be comments, marked by `// `
(slash-slash-space). A line can read `clear`, which erases all
existing dialect rules. It could also read `final`, which
disables any further dialect changes. Finally, a line could
read `"INPUT PATTERN HERE" "OUTPUT PATTERN HERE"`, with an input
and an output pattern enclosed by quotes and separated by a
space.

Dialects are loaded via the `acorn -D filename.od` argument.

Theoretically, `Oak`'s ability to construct and use dialects
means that it can be used to process other languages into
itself. A properly-constructed dialect file could instruct
`acorn` to turn a `Rust` file into `Oak` and compile it. This is
obviously a long way off, however, and will require a more
advanced language than `Sapling`.

## Translator Passes

The `Oak` translator goes through the following passes when
processing a file from `Oak` to `C`. Each of these represents
(at minimum) one full pass over the entire file. In stages after
stage 3, they represent symbol-wise iteration, but before then
they represent character-wise iteration.

1 - Syntax checking
2 - Lexicographical symbol parsing (lexing)
3 - Compiler macros (file inclusions, linker commands, etc)
4 - Rule parsing and substitution (handles rule macros)
5 - Macro definition scanning
6 - Regular macro call handling
7 - Preprocessor definition insertion
8 - Parenthesis and operator substitution
9 - Sequencing
10 - (External) Object file creation via `clang`
11 - (External) Executable linking via `clang`

## Time and Space Complexities

`Oak` translation, in theory, runs in time complexity
proportional to the sum number of characters in the included
files; `O(n)`. This analysis assumes that this number of
characters outnumbers the number of symbols, and that any
further iterative process takes fewer steps that this. The space
complexity is the same as the time complexity. These
prepositions may prove to be false, however, warranting a more
thorough analysis.

## The "std" rule

The `std` rule (included in the "std" package) allows `Oak` to
behave more like other languages. For instance, it allows the
use of `OBJ.FN()`-style method calls and `OBJ[IND]`-style array
accesses. More detail can be found by examining `std/std.oak` or
`std/std.od`.

## The std/math.oak File

The `"std/math.oak"` file provides some math functions, and is
analogous to `C++`'s `cmath`. Below is a table describing the
included functions.

Name      | Description                            | Notes
----------|----------------------------------------|------------------------
`abs`     | Returns the absolute value of a number |
`pow`     | Returns `a` to the power of `b`        | Runs in `O(b)`
`math_pi` | Returns `pi` (3.14159..)               |
`math_e`  | Returns `e` (2.71828..)                |
`fact`    | Returns the factorial of a number      |
`sin`     | Returns `sin` of a value (radians)     | Uses a 9th order Taylor polynomial
`cos`     | Returns `cos` of a value (radians)     | Uses `sin`
`tan`     | Returns `tan` of a value (radians)     | Uses `sin` and `cos`
`floor`   | Rounds a number down                   |
`ceiling` | Rounds a number up                     |
`round`   | Rounds a number                        |
`sqrt`    | Returns the square root of a number    | Uses Newtonian iteration
`f_mod`   | Returns `a` modulo `b` (floats)        |
`min`     | Returns the smaller of `a` and `b`     |
`max`     | Returns the larger of `a` and `b`      |
`even`    | Returns true if a number is even       |

## Multithreading and the std/threads.oak File

`Oak` provides some basic multithreading support as of version
0.0.4. This comes via the `"std/threads.oak"` file. A `thread`
begins a new thread when you call `start(@t, @f)`, where `t` is
the `thread` and `f` is a function mapping void to void
(`let f() -> void`). After a `thread` has begun, you must wait
for it to complete. To halt the execution of the instantiating
thread until the instantiated thread has finished, call
`join(@t)` for some `t: thread`. See later for more help on
function pointers.

## SDL2 and the sdl Package

**Simple DirectMedia Layer 2**, or **SDL2** is a framework for
accessing low-level system IO. The `Oak` git repo includes a
basic `Oak` package called `sdl` which interfaces with the
`C SDL2` library. This allows some graphics to be created by
`Oak`.

## Translation Units

Unlike the `C` family of languages, `Oak` has a built-in build
system. No `makefiles` or command-line flags are necessary (or,
for the latter, allowed).

For instance, to compile a file using `SDL2` in `C++`, you would
need to say something like

`clang++ foobar.cpp -I/usr/include/SDL2 -D_REENTRANT -L/usr/lib -lSDL2`

With proper library construction, the corresponding `Oak`
command can be simply `acorn foobar.oak`.

This auto-build system necessitates the concept of the
**translation unit**.

The term **translation unit** has been thrown around without
explanation several times in this document. A translation unit
in the context of `Oak` is the network of `.oak`, `.o`, `.od`,
`.cpp`, `.hpp`, `.h`, and `.c` files used by `acorn` during a
given command.

For instance, the command `acorn tests/macro_test.oak` would
begin with a translation unit containing only
`tests/macro_test.oak`. If that test were to open with the
line `package!("std");`, the translation unit would grow to
include the file listed under `INCLUDE=` in the `std` package.
This file (`std/std.oak`), would likely contain `link!`,
`include!`, and possibly even `package!` macros, all of which
would expand the translation unit.

When addendum VI mentioned that only one thread function may be
defined per translation unit, that means that a file is barred
from using multithreading if any of its "parent" files use it.

## Member Padding in Interface Libraries

This section is confusing and is only necessary to read if you
are developing an interfacial library.

It is sometimes required for the `C` version of a struct to
have more members than its `Oak` counterpart. For instance, the
interface for the `sdl` package defines the `sdl_window` struct.
On the `C` side of the interface, this struct contains a
`SDL_Window *` member and a `SDL_Renderer *` member. These two
items are not included in `Oak` because they would require too
many useless symbols to be loaded into memory. If they were
included, all structs and definitions which are somehow related
to them would need to be interfaced and loaded as well. `Oak`'s
solution to this is **member padding**. It is perfectly legal to
have mismatched struct members across an interface, as long as
one is not smaller than the other and padding is never touched.
This explains why the `sdl_window` struct has the confusing line
`__a, __b, __c, __d: i128,`. This adds four `i128` members,
effectively requiring 64 bytes of space at the end of the
struct. This space holds the extra members on the `C` side.

If a program were to modify the padding of a padded struct, the
other side of the interface would have its data destroyed and
made nonsensical.

```rust
// sdl/sdl_interface.c

struct sdl_window
{
    u64 width, height;

    SDL_Window *wind;
    SDL_Renderer *rend;
};

```

```rust
// sdl/sdl_interface.oak

let sdl_window: struct
{
    width, height: i32,
    __a, __b, __c, __d: i128,
}

```

## 'str' vs 'string'

The atomic `str` struct is exclusively for statically-stored
string literals. Anything that is included in your source code
surrounded by quotations is a `str`. These are of fixed size,
and are stored directly in the compiled source code. On the
other hand, the `string` struct (available through the
`std/string.oak` file) is a dynamically-resizing container for
storing strings. These structs keep their memory on the heap, so
they can be of any size. These are much more useful, as they can
be the return type of functions and `Oak` can more easily
interface with them. Internally, `strings` are
dynamically-resizing arrays of `i8` atomics (in `C++` terms, a
`vector` of `char`'s). `string` structs were introduced in `Oak`
0.0.6.

## Input and Output Files

The `std/files.oak` file defines the `i_file` and `o_file`
structs, which handle input and output to and from files
respectively. `i_file`s read from files, and `o_file`s write to
files. `std/files.oak` is an interface to the `C` `FILE *`
system.

`i_file`s and `o_file`s do **not** open a file when they are
created! You must call the `open` method before they can be used
to read or write.

Here is a useful demonstration of both structs.

```rust
package!("std");
include!("std/files.oak");
include!("std/string.oak");
use_rule!("std");

let main() -> i32
{
    // Write test file
    let out: o_file;
    let to_write: string;

    out.open("hi.txt");
    
    New(@to_write, "abcdefghijklmnopqrstuvwxyz\n");
    
    out.write(to_write);
    
    out.close();
    to_write.Del();
    out.Del();

    // Read test file
    let str_a: string;
    let str_b: string;

    let inp: i_file;
    inp.open("hi.txt");
    str_a = inp.read(to_u128(5));
    str_b = inp.read(to_u128(5));

    print("a: ");
    print(str_a);
    print("\nb: ");
    print(str_b);
    print("\n");

    inp.close();
    inp.Del();
    str_a.Del();
    str_b.Del();
    
    0
}

```

## Function Pointers

A variable can store a pointer to a function. For instance, a
variable `f` can hold a pointer to a function taking an `i32`
and yielding an `i32` via the code `let f: ^(i32) -> i32;`. The
argument types go inside the parenthesis, and the return type
after the `->` yield operator. This variable can later be made
to point to any function mapping the argument list to the yield
type. It can be called in the same way as any other functions.

For example, the program

```rust
package!("std");

let a(what: i32) -> i32
{
    what
}

let b(what: i32) -> i32
{
    (2 * what)
}

let main() -> i32
{
    let f: ^(i32) -> i32;
    f = @a;

    print(f(1));
    print("\n");

    f = @b;

    print(f(1));
    print("\n");

    0
}

```

would print

```txt
1
2

```

## Enums

Enums in `Oak` are essentially ways for one type to encompass
multiple types. Enums are very similar to structs, except that
they can hold only one of their values at once. However, unlike
a `C++` union, the type of an `Oak` enum is also stored. In this
way, it is like a combination of `C++`'s enum and union. An
`Oak` enum declaration looks like this:

```rust
let example: enum
{
    option_a: i32,
    option_b: unit,
    option_c: preexisting_struct_name,
}
```

Note that an `Oak` enum looks exactly like an `Oak` struct.
Enums are handled through use of the `match` statement as below.

```rust
let main() -> i32
{
    // Assume the previous code chunk is included
    let obj: example;

    match (obj)
    {
        case option_a(data)
        {
            // `data` becomes a pointer to the i32 which
            // option_a entails
        }

        case option_b()
        {
            // In this case, no variable is captured from
            // option_b. This is almost always the case for unit
            // structs (obviously) but can also be done with
            // non-unit structs.
        }

        default
        {
            // There can be no capture in a default option,
            // because the capture's type could not be known at
            // compile-time.
        }
    }

    0
}

```

The `match` statement is one of two ways to access the data
inside enums in `Oak`, with the other being the
compiler-generated "wrap" methods. The compiler generates

`let wrap_OPTION_NAME(self: ^ENUM_NAME, data: OPTION_TYPE) -> void`

for each option `OPTION_NAME` in an enum `ENUM_NAME`. This wraps
the data variable into the enum. This all happens via a compiler
`C` interface.

Note that, if the type of an enum option is the unit struct, the 
compiler will instead generate a single-argument "wrap" method,
taking only the self.

Generally speaking, a wrap method puts data into an enum, and a
match statement accesses it.

## The printf! macro

The `printf!` (print formatted) macro is a very useful and
compact way to print things. It is available via
`include!("std/printf.oak");`. This is similar, but not exactly
like, `C`'s printf function. The first argument to the `printf!`
is a format string. Any percentage signs `%` within this format
string will be replaced by the macro arguments which follow. For
instance, `printf!("hello %\n", 5);` would print `hello 5`. This
works with literals or variables- anything for which the
single-argument `print` function is defined. The first argument
after the format string corresponds to the first `%`, the second
to the second, and so on. For instance,
`printf!("% % % % %", 1, 2, 3, 4, 5);` would print `1 2 3 4 5`.

```rust
package!("std");
include!("std/printf.oak");
use_rule!("std");

let main() -> i32
{
    printf!("The number five: % \nThe number six: % \n", 5, 6);
    printf!(
        "The following should be a literal percentage sign: \\%\n"
    );

    printf!("This test the failure system of printf % \n");

    let i: i32 = 5;
    let j: f64 = 1.234;
    printf!(
        "Variable i is %, j is %, and some text is '%'.\n",
        i,
        j,
        "some text here"
    );

    0
}

```

## NULL Values and Error Handling: `opt<t>` and `result<g, b>`

`Oak`, like any sane language, has no `NULL`. Any item which may
sometimes be `NULL` must be explicitly wrapped in an `opt` enum.
This enum has only two options: `some` and `none`. In the `some`
state, it holds exactly one instance of the generic `t`.

Similarly, `Oak` does not have a `throw` / `catch` error system.
Instead, the control flow must be explicitly controlled at
compile-time. Any function which has the possibility to error
must return its values wrapped in a `result<g, b>` enum. This
has two states, `ok` and `err`. The `ok` state holds one
instance of the `g` (for good) generic, while the `err` state
holds one instance of `b` (for bad). Error recovery must be
explicitly handled via `match` statements.

## Assertions

The `assert!` macro is an extension of the `panic!` macro, and
is available in `std/panic_inter.oak`. `assert!` takes a single
argument. If this argument evaluates to `false`, it will call
`panic!`. Otherwise, it will do nothing. The literal output of
this macro is `if (CONDITION) {} else { panic!("CONDITION"); }`.
This makes it a recursive macro, or a macro which calls other
macros.

## Boolean Expansion Rules

`Oak` has very limited functionality for boolean values by
default. For instance, `Oak` has no negation (`!`) operator,
because the operation-substitution portion of the compiler
only handles binary operations (ones which have a left and
right side). The negation operator is unary, and thus is not
handled there. The `bool` rule (available via `std/bool.oak`)
provides this functionality, as well all allowing the
`python`-style `not`, `or`, and `and` keywords.

Note: While using the `bool` rule, both `!` and `and` are legal
ways to call the negation operator.

## Efforts to Reduce Undefined Behavior

To cut down on undefined behavior (resulting from uninitialized
variables), every datatype in `Oak` has one and only one
"default" constructor. The default constructors of all struct
members are called upon instantiation. This default value is
said to be the "canonical" or "unit" value of that datatype.

As such, **`Oak` cannot have uninitialized variables.**

## Instantiation Blocks and Generic Data Structures

As should be obvious to anyone who has worked with a templated
data structure library, it is necessary for structures to
generically have not only members, but methods. However, `Oak`
as discussed thus far has no way to instantiate a group of
generic methods when a generic data structure is instantiated.
This is where the `pre` and `post` keywords come in. A `pre`
keyword means that the following code block will need to be run
before a template is instantiated. `post` is the same, but it is
run after the instantiation. Failure in either of these code
blocks means that the template is invalid, but is not treated as
a compiler error.

```rust
let list<t>: struct
{
    data: t,
    next: ^list<t>,
}

let New<t>(self: ^list<t>) -> void;
let Del<t>(self: ^list<t>) -> void;

let main() -> i32
{
    New<i32>;
    Del<i32>;

    let node: list<i32>;

    0
}

```

`Oak` as discussed so far necessitates lines 7 and 8. These tell
the compiler to instantiate the generic functions `New` and
`Del` for use with `t = i32`. However, this is obviously
cumbersome to an end user. Thus, we introduce the `post` block.

```rust
let list<t>: struct
{
    data: t,
    next: ^list<t>,
}
post
{
    New<t>(self: ^list<t>);
    Del<t>(_: ^list<t>);
}

let New<t>(self: ^list<t>) -> void;
let Del<t>(self: ^list<t>) -> void;

let main() -> i32
{
    let node: list<i32>;
    0
}

```

The above code tells the compiler to instantiate `New` and `Del`
with the `t` generic anytime the `node<t>` template is
instantiated. Thus, these are automatically instantiated by the
compiler when the struct is. Also notice that the argument name
in the `post` block doesn't matter, so long as the rest of the
instantiator call matches the candidate.

This system is in response to the fact that, in `Oak`, a struct
does not inherently "come" with any methods, and thus the exact
methods to instantiate with a generic struct are unknown. This
allows the programmer to tell the compiler exactly what is
needed for use with the struct.

## `trait`s

Traits are a shorthand for instantiating many functions at once
without specifying a type. Usually, `pre` and `post` blocks can
only be used when instantiating a function, enumeration or
struct. However, this may not always be wanted. For instance,
the `stl::arr` type does not provide any dot product function.
This, along with several other math functions, can be created at
once by instantiating `stl::math_arr`. Internally, this is a
zero-member struct with the associated functions contained in
`pre` and `post` blocks. This can be notated in two ways, which
are shown below.

Without traits:
```rust
let item<t>: struct
{
    ,
}
pre
{
    // These things must be true for this to work
}
post
{
    // Some functions to instantiate when this trait is called
}
```

With traits:
```rust
// The `std` rule provides the `trait` shorthand
package!("std");
use_rule!("std");

let item<t>: trait
pre
{
    // These things must be true for this to work
}
post
{
    // Some functions to instantiate when this trait is called
}
```

## Explicit, Implicit / Casual, and Autogen Function Definitions

It is often useful to separate the declaration of a function
from its implementation (see the earlier section on division of
labor). However, it is necessary for any function to have
exactly one implementation to avoid undefined behavior. When
creating structs, a definition for `New` and `Del` are always
required. However, it is not always practical to write these
functions explicitly each time, especially if they are to be
unit (empty) functions. For these reasons, we introduce the
concepts of **explicit**, **implicit**, and **autogen**
function definitions.

```rust
// A casual definition
let do_thing() -> void;

// An explicit definition
let do_thing() -> void
{
    print("hello\n");
}

```

All casual definitions are erased when an explicit definition is
called. This allows external definitions and interfacial files.
For instance, you could link an object file to `Oak` which
provides the definition for a casual definition. You cannot,
however, do this with an explicit definition. This will cause
multiple definitions of the symbol.

When a struct is defined, a unit explicit definition for `New`
and `Del` are added to the symbol table. These **do** provide an
explicit definition, and thus cannot be overridden. There is,
however, a way to avoid this. Casual definitions erase not only
implicit definitions, but also auto-generated unit definitions.
Thus, if you provide a casual definition for `New` or `Del`, you
can define them in external object files and link them together.

Autogen functions are marked with the `//autogen` special
symbol during sequencing. This is just a unit marker used for
detection, and is ignored for all real compilation purposes.

## Naming Conventions for Library Files

File names, especially in libraries, and **especially** in the
`Oak` `std` library, should be clear and concise. Names should
be singular, even if the file describes a class or data 
structure (IE `std/strings.oak` should become `std/string.oak`). 
Files linking `Oak` to another language should be as physically
close  as possible to their partner file as possible. These
interfacial partner files should be named as close to the `Oak`
naming scheme as is possible for their language, and their
compiled object files **must** adhere to the `Oak` naming
scheme. File names may be shortened to a reasonable degree, so
long as they do not conflict with any other package and their
full name can be reasonably extracted from the abbreviation (for
instance, `input_output.oak` to `io.oak` is a valid
abbreviation). The same can be said about struct and enum names.
If there is an addendum required to the name, it should go after
the main name (for instance, the `std` file
`std/conv_extra.oak`).

Files should provide nothing more than they have to. If you have 
the option to split something (especially interfaces) off into 
its own file, **you generally should do so.** For example, 
`std/unit.oak` is one line of code long (`let unit: struct;`) 
because it does not **need** to have anything more. `Oak` code 
and, by extension, `.oak` files should be minimalist.

## `c_print!`, `c_sys!` and `c_panic!`

`c_print!` prints a message at compile time, concatenating the 
passed strings. `c_panic!` does the same thing, but throws the 
result as a compile error. `c_sys!` runs a command at compile
time. In any of these cases, the message will be prefaced by its
originating file and line number. Since these macros are
compile-time only, any variables passed will not be evaluated;
Only their names will be printed.

The `c` refers to compilation, not to the `C` programming
language.

## Pointer Macros

Because `Oak` uses pointers to denote mutability, functions are
allowed to automatically reference or dereference arguments.
However, in maintaining interoperability with the `C` family,
`Oak` also supports pointer arrays. This combination of facts
makes `Oak` pointers uniquely hard to work with. To address
this, the `ptrarr!` and `ptrcpy!` macros are used. `ptrcpy!`
takes two pointers, and copies the second into the first. If
these are `a` and `b`, it performs `a = b`. `ptrarr!` takes the
head of a pointer array and an integer. If we are to denote
these `a` and `b`, it returns `a[(unsigned long long)b]` (that
is to say, `*(a + (unsigned long long)b)`).

Note: Pointer copying is only allowed in places where `alloc!`
and `free!` are valid, since it can cause duplicate addresses.
`ptrcpy!` can also accept `0` as its second argument, setting
the pointer to the null pointer.

## The Erase Macro

`Oak` does not have private members, but sometimes it is still 
useful to disallow the use of a type after its declaration. In 
this case, you can use the `erase!` macro. This takes the name 
of a struct, and disallows it from being used again. For 
instance, the `std/interface.oak` file provides erased structs 
of various sizes for use in struct interfaces. These items can 
never be accessed, but still take up space within a struct.

## Test Suites

A test suite is a set of unit tests that `acorn` will compile
and optionally execute in order to test the validity of a build.
A test suite is created by creating a folder named `tests`, and
putting any number of `*.oak` files within it. After this
structure has been imposed, `acorn -T` will compile the entire
test suite and report on any failures it discovers. Similarly,
`acorn -TE` will compile and execute each item in the test
suite. `acorn -TT` will test as usual, but stop going after the
first failure. The same goes for `acorn -TTE`. A compilation is
considered a success if and only if the `acorn` compile process
returns no errors. An execution is considered a success if and
only if its main function returns a `0`. Either failure case
occurring will be considered a failure and will count towards an
internal failure counter. The results of running the test suite
will be reported after the test's conclusion.

The `Oak` standard test suite is included in the installation
`git` repo. Each item within this test suite must compile in
order for an `Oak` update to be considered valid. This can be
tested via `make test` or `acorn -T`. These items do not need to
execute with a zero exit status, as some of them are
demonstrations.

Test suite executables are not individually saved. Each test
is compiled into `a.out`, which is overwritten by the next test.

## Advanced Language Augmentation Via `raw_c!` Macro

The `raw_c!` macro completely bypasses the `Oak` compiler and
directly inserts its arguments as `C` code. This can be used for
more advanced language development features which may require
efficiency or capabilities beyond that of `Oak`.

## Future Improvements to `$apling` - `TARM`

`$apling` is limited in the modifications it can make to `Oak`,
but it is nevertheless a vital resource. In addition to the
existing rule system, a more powerful rule engine is in
development. This new rule system is called `TARM`, for Token
Augmentation Register Machine. It is a type of Turing Machine
optimized for low-level token stream augmentations which will be
controlled by an assembly language. A rigorous exploration of
`TARM` will not be attempted here, especially since it is not
yet implemented into `Oak`.

`acorn` version `0.2.11` introduced the option for multiple
rule engines, allowing the future integration of `TARM`. Rule
engines are highly experimental, and are thusly not detailed in
this document as a means of discouraging their use. When they
have reached a more stable state of development, a section will
be added on them.

## List of Keywords

The following are the canonical `Oak` keywords- That is to say,
these words cannot be the names of variables, structs, enums,
or functions in canonical `Oak`. `Oak` aims to have as few
keywords as possible, keeping with its theme of baseline
minimalism.

- let
- if
- else
- while
- match
- case
- default
- pre
- post

**Note:** There is nothing stopping a rule or dialect from
adding more keywords. For instance, the `bool` rules add the
`and`, `not`, and `or` keywords.

## List of Atomic Structs

The following are atomic (built-in, indivisible) types.

- struct
- enum
- integers - u8, i8, u16, i16, u32, i32, u64, i64, u128, i128
- floats - f32, f64, f128
- str - equal to `^i8`
- bool
- void

The `unit` (memberless 1 byte) struct is provided by 
`std/unit.oak`.

## List of Keyword-Like Macros

The following are atomic (built-in, indivisible) macros.

- include!
- link!
- package!
- new_rule!
- use_rule!
- bundle_rule!
- erase!
- alloc!
- free!
- c_print!
- c_panic!
- c_sys!
- type!
- size!
- ptrcpy!
- ptrarr!
- raw_c!

This list is not inclusive, as additional compiler macros are
constantly being added.

## Misc. Notes

Some miscellaneous notes which are not long enough to warrant 
their own section in this document:

- `Oak` does **not** have default argument values.
- `Oak` does **not** have struct builder syntax.
- `Oak` was previously (0.1.*) translated to `C++`. Since 0.2.0,
    it has instead been translated to `C`.
- The `New` and `Del` operator aliases are single-argument only.
- You can call a multi-argument `=` operator (`Copy`) like this:
    `a = (b, c, d);

# Packages Included With the Standard Install

The following packages are included with the `Oak` source code
on `github`. These can be installed via the command
`make packages`.

## `extra`

This package includes a regular expression interface and some
resources for bitwise, hexadecimal, and ASCII printing of
data structures. These prints allow you to see the contents of
any arbitrary type.

## `sdl`

This package interfaces with the `C` `SDL2` library, which
allows access to graphics. This library is useful for games.

## `std`

This package includes files vital for the function of nearly all
`Oak` programs. It should be included in virtually all programs.

## `turtle`

This packages is based on the `Python` `turtle` package. It
provides a `turtle::turtle` object, which is a turtle with a pen
on a canvas. This turtle can rotate and move, lift or set down a
pen, and thus draw pictures.

## `stl`

This package is the standard templated library for `Oak`. It
includes generic implementations of the following data
structures.

- Binary Search Tree - `stl::bst` via `stl/bst.oak`
- List (doubly linked list) - `stl::list` via `stl/list.oak`
- Map (symbol table) - `stl::map` via `stl/map.oak`
- Queue - `stl::queue` via `stl/queue.oak`
- Set - `stl::set` via `stl/set.oak`
- Stack - `stl::stack` via `stl/stack.oak`
- Dynamic array / vector - `stl::vec` via `stl/vec.oak`
- Static array - `stl::arr` via `stl/arr.oak`
- Static 2D array - `stl::matrix` via `stl/matrix.oak`

The included binary search tree structure is non-self-balancing.
It can be used as a `map`, but caution should be used. It has
worst-case lookup time of `O(n)`. The included `list` is a
standard doubly linked list. It does not have random-access
lookups, but can be used for traversals (forwards or backwards).
The included `map` uses linear probing based on a given hash
function. It is hard-coded to use a maximal alpha of `0.5`. When
this is reached, it will double in size and rehash. The included
`queue`, `stack`, `matrix`, `arr` and `vec` are standard
implementations of their respective data structures. Note that,
unlike `vec`, `arr` is not self-resizing. Neither are the
included matrices, although there is no corresponding
self-resizing version of these. The included `set` is a
specialization of `map` where only the key and hash are stored,
with no lookup data.

# Advanced Usage / Esoterica

This section outlines niche situations in `Oak` which may
require further explanation. This information is not required
for a programmer-level understanding of the language, but
instead serves as a resource for niche explanations.

## Plural Instantiation - Variables

**Plural instantiation** is the use of a single `let` statement
to declare several items of the same type. Most modern languages
support this to some extant, and `Oak` is no exception. For
instance, the code

```rust
let a: i32;
let b: i32;
let c: i32;
let d: i32;
```

Could instead be written

```rust
let a, b, c, d: i32;
```

If the above usage of "items" instead of "variables" has caused
concern in you, you are correct. More will be detailed below.

**Note:** Plural definition is a feature of canonical `Oak`-
That is to say, it does **not** require the use of any rules.

## Plural Instantiation - Functions

A particularly esoteric-looking feature of `Oak` is
**function plural instantiation**. When it was stated above that
most languages support plural instantiation to some extant, this
was not included.

`Oak` is a language designed around language interfaces, and
prominently features the "interfacial" file. This is a file
which provides functions signatures which can be used within
`Oak` and specifies an object file to provide the definitions.
In such a case, it may be inconvenient to write out the full
function signatures of many similar functions. In this case,
function plural instantiation can help us tremendously.

Consider the following code.

```rust
let a() -> void;
let b() -> void;
let c() -> void;
let d() -> void;
let e() -> void;
let f() -> void;
let g() -> void;
```

In `Oak`, this could be shortened to the following, using the
same principles as variable plural instantiation.

```rust
let a, b, c, d, e, f, g() -> void;
```

This saves space, programming time, and compiler time. However,
this may seem like a trivial adjustment. For a more grounded
example, consider the following code from an earlier version of
the `std/math_int_inter.oak` file, which provides all functions
for interactions with built-in integer data types.

```rust
let Add(self: u8, other: u8) -> u8;
let Sub(self: u8, other: u8) -> u8;
let Mult(self: u8, other: u8) -> u8;
let Div(self: u8, other: u8) -> u8;
let Mod(self: u8, other: u8) -> u8;
let And(self: u8, other: u8) -> u8;
let Or(self: u8, other: u8) -> u8;
let Lbs(self: u8, other: u8) -> u8;
let Rbs(self: u8, other: u8) -> u8;

let Eq(self: u8, other: u8) -> bool;
let Neq(self: u8, other: u8) -> bool;
let Less(self: u8, other: u8) -> bool;
let Great(self: u8, other: u8) -> bool;
let Leq(self: u8, other: u8) -> bool;
let Greq(self: u8, other: u8) -> bool;

let New(self: ^u8) -> void;
let Del(self: ^u8) -> void {}

let Copy(self: ^u8, other: u8) -> u8;
let AddEq(self: ^u8, other: u8) -> u8;
let SubEq(self: ^u8, other: u8) -> u8;
let MultEq(self: ^u8, other: u8) -> u8;
let DivEq(self: ^u8, other: u8) -> u8;
let ModEq(self: ^u8, other: u8) -> u8;
let AndEq(self: ^u8, other: u8) -> u8;
let OrEq(self: ^u8, other: u8) -> u8;
```

Using function plural instantiation, this can be changed to the following (much shorter) code.

```rust
let Add, Sub, Mult, Div, Mod, And, Or, Lbs, Rbs(self: u8, other: u8) -> u8;
let Eq, Neq, Less, Great, Leq, Greq(self: u8, other: u8) -> bool;
let New(self: ^u8) -> void;
let Del(self: ^u8) -> void {}
let Copy, AddEq, SubEq, MultEq, DivEq, ModEq, AndEq, OrEq(self: ^u8, other: u8) -> u8;
```

In addition to taking up less space and being more maintainable,
the above change caused a non-trivial speed boost to all
compilation involving the `std` package, as function plural
instantiations save compiler time.

# `Oak` Compiler Structure

This section is not required for a programmer-level
understanding of the `Oak` language, but may be useful for
troubleshooting and deeper understanding. The sections detailed
below view each section of the `Oak` compiler as a discrete
object, even when that is not technically the case (for
instance, "the sequencer" and the "reconstructor" are really
tightly intertwined, and few, if any, of the underlying units
are represented by `C++` objects in code).

The following sections represent a tangled nest of calls, and
thus it should not be assumed that they are presented in the
strict order in which they occur.

## Intermediate Language Choice

`acorn` uses `C` as an intermediate representation. This is done
for the sake of portability, since `C` is very close to a
universal machine language and `C` compilers have been heavily
optimized. In early development, `acorn` instead used `C++`, but
this proved too costly and largely unnecessary. It is plausible
that `acorn` one day switches to `LLVM IR` instead of `C`, but
this is not immediately probable and should not be expected.

## Compiler Frontend and `acorn`

The `acorn` resource is a wrapper for the underlying `Oak`
compiler system. It also provides numerous other resources, such
as package management, which will not be detailed in this
section. As far as the `Oak` compilation process is concerned,
`acorn` takes in a file name, and processes all the code within
that file, recursively following inclusions therein.

It also handles higher-level calls to external systems. For
instance, `acorn` calls the `C` compiler after the translation
process, and the `C++` linker after compilation. While the
following compiler sections are usually devoted to being free of
fluff, `acorn` is allowed to have user-friendly features.

Upon taking in a file name, the compiler frontend calls the
lexer upon its contents. The lexer is detailed next.

## Lexical Tokenization and the Lexer

**Note: Within `Oak` documentation, the terms `token` and**
**`symbol` are often taken to have the same meaning.**
**Additionally, "lexical tokenization" is referred to as**
**"lexicographical symbol parsing". This is an `Oak`-specific**
**term.**

The lexer takes in a string, and returns a **token stream**
(a stream of discrete strings, each of which is the smallest it
could meaningfully be). This process is called lexical
tokenization, and these tokens will be the only thing used by
the remainder of the compilation process.

The `Oak` lexer, like most lexical tokenizers, operates using a
DFA (deterministic finite-state automata). This machine is fed
each character, and changes state based on only its current
state and the input. It has one state which represents the
splitting of a token. Because of this process, the `Oak` lexer
runs in `O(n)`, where `n` is the number of input bytes.

Tokenization is context-dependant. The same symbol set that in
one place would signify a new token could, in another, continue
on in the same token. For instance, if the lexer came across the
string `"a+8"`, it would yield the token stream `"a", "+", "8"`.
If it instead came across the string `"a8+8"`, it would yield
`"a8", "+", "8"`. In this case, the `8` caused a new token to
be started when following `+`, but not when following `a`.

During lexing, several important processes happen. The first of
these is that all comments, single-line or multi-line, are
erased. All `#`-begun lines (the only legal use of which being
the shebang `#!/path/to/program`) are also removed. Secondly,
information about the origin and line of all tokens is
inserted into the token stream.

The final pass of the lexer is called the conglomeration pass.
At this time, the tokenized input will be iterated over, and
qualifying series of tokens will be merged. For instance, `-` is
a valid token on its own, but when it precedes a number, it
should not be considered its own token. Similarly, two string
literals separated by whitespace will be merged together.

## Operation Substitution

The operation substitution phase of compilation replaces
operator calls (addition, subtraction, etc) with their formal
function call definitions. This phase is a sort of hardcoded
preprocessor rule pass. After this phase, no operators remain-
only function calls.

## Preprocessor Rules and the Rule Processor

A rule is a combination of an **input pattern** and an
**output pattern**. The input pattern represents, via a series
of **cards** (space-separated tokens which match some set of
other tokens), the set of token streams which the rule targets.
The output pattern is the content which a match will be replaced
by in the token stream. There are several special cards for
output patterns as well. More about the specifics of rules can
be found earlier in this document.

The rule processor, which is by default `$apling`, is a
specialized turing machine (or, for more limited rule
processors, deterministic finite automata) for the augmentation
of patterns of tokens (or symbols, in `Oak` terminology).

The original `$apling` is a push-down automata, meaning it has
limited functionality. However, work is in progress to replace
it with a Token-Augmentation Register Machine (`TARM`), which
will offer Turing-completeness and thus more complete
functionality.

### Side Note: The Hierarchy of Automata

The following lists several models of automata from least
powerful to most, with an example for each. Each successive
class can perform all computations of those above it and more.

- Combinatorial Logic (Boolean logic)
- Finite-State Automata (RegEx)
- Push-Down Automata (Sapling rule engine)
- Turing Machine (TARM rule engine)

### Part 0- Rule Engines

A rule engine is a function which takes in a certain portion of
the input token stream and augments it in some way. This is an
abstraction which allows `$apling` and `TARM` to function
interchangeably in the compilation process. New rule engines
can only be added by directly modifying the `C++` `acorn` source
code and recompiling. Thus, they are not to be considered a
front-facing resource for developers- Even those of dialects.

### Part 1- Rule Loading

Upon encounter of the `new_rule!` macro, the `acorn` compiler
frontend adds the rule contained within into its internal bank
of rules. Upon `use_rule!`, it copies that rule into the bank
of rules which are used in the **current file**. `use_rule!`
does **not** carry over into other files.

The standard call to `new_rule!` takes three arguments: A rule
name, an input pattern, and an output pattern. However, if a
fourth argument is provided, it will be treated as the rule
engine. If no such engine is provided, `$apling` will be
presumed.

### Part 2- Compile-Time Rule Application

During the iterative process executed by the `acorn` compiler
frontend on a given file, there is a stage wherein rules are
handled. The specific execution location of this stage, as well
as the number of times it is executed, has changed numerous
times in the lifetime of `Oak`.

During this phase of compilation, the token stream is iterated
over, and each of the active rules are applied. If a rule
matches, the matched token stream section is replaced
accordingly. The specifics of this vary according to the rule
processor's pattern language, and cannot be detailed here. More
information about these specifics can be found earlier in this
document.

**Note:** This process can occur multiple times. A rule can, for
instance, cause the inclusion of another file which must be
loaded before compilation can continue. This might, in turn,
require the application of more rules. This section ceases after
no further rules can be applied.

## Macros, Definitions and the Macro Instantiation Unit

Macros are compile-time functions. They output raw code, which
is then used by the final program. They have the same notation
as **compiler macros**, which are analogous to preprocessor
directives in other languages. Technically, however, compiler
macros operate on the compiler itself, rather than the source
code's token stream.

Preprocessor definitions are compile-time constants, as opposed
to functions. They are analogous to `C`'s `#define` directive,
except that they cannot be redefined after creation.

**Note:** Macros and definitions cannot be redefined. Attempting
to do so will cause an error.

Macro definitions are collected by the `acorn` compiler frontend
during one phase of the iterative compilation process, and
replaced with their corresponding values before sequencing.

Macro definitions are written to their own temporary `.oak` file
during the build process. After this, they are compiled into
executables by `acorn`. Input to a macro is provided as command
line arguments into these executables, and their `cout` output
is reinserted into the code at the desired spot.

## Syntax Trees and the Sequencer

Sequencing is the penultimate step of compilation, followed by
reconstruction. This is the most time-consuming step, where the
specific semantic meanings of statements are determined and they
are translated into their equivalent low-level-language code.
This is a highly recursive process, and is always evolving.

`Oak` sequencing is perhaps best detailed by the source code
itself. Thus, this author invites you to analyze `sequence.cpp`.

## Translation and the Reconstructor

The final phase of compilation is reconstruction, in which the
low-level statements generated by sequencing are assembled into
an equivalent `C` program, which is then compiled. This
essentially works as a specialized traversal of the syntax tree
where each node is replaced by its `C` statement. This `C` code
is then written to an output file and compiled via an external
`C` compiler. If so desired, a `C++` linker can also be called.

## Templating and the Template Instantiation Unit

**Note: In `Oak`, the terms *templated* and *generic* are used**
**interchangeably. The former comes from `C++`, and the later**
**from `Rust` (in the author's experience).**

The template instantiation unit is the section of the `Oak`
compiler which handles instantiation calls for generic /
templated code blocks. There are three parts to the
instantiation process, detailed below.

### Part 1- Generic Insertion

When a function, enum or struct is detected to be generic (via
the `<t>`-style generic declaration), it is inserted as a
template into the template instantiation unit's internal bank of
templates. The specific generic arguments are also stored, as
well as the type (for instance, a function or enum).

### Part 2- Instantiation Call

Later on, the sequencer will detect a call to the instantiator,
denoted `<some_type_which_exists>`. Upon hearing this, the
instantiator will search through its templates until it finds
one which suites the call (some template such that, when the
generic is substituted into the type, the resulting type matches
that used in the instantiator call). Alternatively, if this
specific template / generic combination has already been
instantiated, it will simply return. Otherwise, once the correct
template has been identified, it will move on to part 3.

### Part 3 A- Pre and Post Blocks

A `pre` block represents the things which must be true in
order for a certain template to be viable. For instance, the
choice of whether to use a tree or hash-table based lookup
table will depend on whether the data is hashable or only
comparable. A `pre` block can check for hashability, and
reject a given template if it cannot be ensured.

A `post` block represents things that should logically be
instantiated along with a given template. For instance, if we
are instantiating a struct, we would logically also want to
instantiate its constructor and destructor.

`pre` blocks are executed before the main template during an
instantiation call, and `post` blocks are executed after.

### Part 3 B- Template Substitution

Upon the selection of the proper code block, the instantiator
will begin by iterating through the tokens of the code block,
replacing any occurrence of the `i`-th generic with the `i`-th
generic specified during the instantiator call. For instance,
if the template was declared via `<t, h>` and the instantiator
call was `<i32, f64>`, it would replace all instances of `t`
with `i32` and all instances of `h` with `f64`. After this
replacement, the resulting code section would be fed to the
sequencer like any other code. This causes the symbols within
to be entered into the global namespace as if they were never
generics at all.

Note: The generic system exists exclusively within `Oak`; There
is no way to interface `C++` generics with `Oak` or vice-versa.

## Package Importing Via the `package!` Macro

The `package!` macro belongs to the compiler macros, in that it
directly controls the actions of the compiler before any other
code is evaluated (roughly equivalent to the preprocessor in
`C`). Upong a call to `package!`, the compiler looks for a
directory in the `Oak` include directory which matches the
passed name. On UNIX systems, this directory is
`/usr/include/oak`. If you wanted to import the `std` package,
`acorn` would check for the existance of `/usr/include/oak/std`.
If this folder does not exist, `acorn` throws an error.

Assuming that the aforementioned folder does exist, `acorn`
attempts to load the `oak_package_info.txt` file within it. This
is a specially formatted file which tells the compiler some
basic information about the package: Most importantly, it
contains an `INCLUDE` tag, which lists the file(s) to include
when the package is loaded by the compiler. This value is
usually a central "linkage" file which links all the others
together. Upon finding this entry point, `acorn` will begin
loading all files included by the package. After finishing, it
will return to after the original `include!` statement.

## The Mangler

The mangler is how `Oak` differentiates between functions with
identical names but different types. It takes in a type and a
function name, and returns a string which fully disambiguates
this function from any others of the same name. It does this by
appending a certain special string for each item in the type.

### Prefix - `Oak`'s Internal Type Representation

`Oak` types can be represented in a linear fashion. Consider the
following code.

```rust
let a: i32;
```

Internally, `Oak`'s symbol table will store a definition for `a`
with type `["i32"]` (where `[]` represents a `C++` vector).

```rust
let b: ^^i32;
```

`Oak` would store this type as `[PTR, PTR, "i32"]`. Similarly,

```rust
let c(arg: bool) -> void;
```

would be `[FN, "bool", MAPS, "void"]`. A more complex version
like

```rust
let d(arg_a: ^i32, arg_b: ^^bool) -> bool;
```

would become
`[FN, PTR, "i32", JOIN, PTR, PTR, "bool", MAPS, "bool"]`. This
is a way to represent unambiguous types in a linear vector
format. The mangler leverages this to turn a type vector into a
string.


### The Mangler: Easy Cases

If a symbol `a` has the type `[FN, "i32", MAPS, "void"]`, the
mangler would produce the output name `a_FN_i32_MAPS_void`.
This symbol would keep the same type, but would be unambiguous
in name. This is one of the reasons why capital letters are not
allowed to coexist with underscores in `Oak`- it could interfere
with the mangler.

The following are several more complex cases mapping input `Oak`
directly to its output `C`.

```rust
let a(arg: i32, arg_b: ^^bool) -> bool;

let foo: struct
{
    ,
}

let b(arg: foo) -> ^void;
let c(arg: ^(_: bool) -> void) -> void;
```

```C
struct foo
{
};

bool a_FN_i32_JOIN_PTR_PTR_bool_MAPS_bool(i32 arg, bool **arg_b);
void *b_FN_foo_MAPS_PTR_void(struct foo arg);
void c_FN_PTR_FN_bool_MAPS_void_MAPS_void(void (*arg)(bool));
```

This can become a pain in interfacial files, where the literal
name of the symbol defined externally must match the `Oak`
mangled version. However, many language manglers do not even
provide a consistent standard across compilers, so `Oak` is at
least better than that.

### The Mangler: Edge Cases

Consider the following `Oak` code for a singly-linked list node.

```rust
let node<t>: struct
{
    data: t,
    next: ^node<t>,
}
post
{
    New<t>(_: ^node<t>);
}

let New<t>(self: ^node<t>) -> void
{
    // ...
}

let main()
{
    node<i32>;
}

```

What does the instance of `New` created for `node<i32>` mangle
to? In fact, what does `node<i32>` mangle to? `Oak`'s target
language is `C`, which does not have generics, so it cannot
contain the `<...>` syntax.

As you may have guessed, this solution is solved similarly to
the one mentioned above. `node<i32>` simply mangles to
`node_GEN_i32_ENDGEN`, and this is used as its atomic type name
internally for mangling purposes. This means that the `Oak`

```rust
let New<i32>(self: ^node<i32>) -> void
```

will mangle to the `C`

```c
void New_FN_PTR_node_GEN_i32_ENDGEN_MAPS_void(struct node_GEN_i32_ENDGEN *self);
```

This means it is possible for the compiler (and technically the
programmer) to tell the type of a given symbol in the target `C`
code given just its mangled name.

## Function Candidate Identification

There are several ways in which a function can be matched to a
function call.

- Precision match (all types match exactly)
- Implicit casting for built-in types
- Auto referencing and dereferencing

Demonstration of the first case:

```rust
let fn(a: foo, b: bar) -> void;

let main() -> i32
{
    let first: foo;
    let second: bar;

    fn(first, second);

    0
}
```

Demonstration of the second case:

```rust
let fn(a: i128) -> void;

let main() -> i32
{
    let arg: i32;

    fn(arg);

    0
}
```

Demonstration of the third case:

```rust
let fn(a: ^i32) -> void;

let main() -> i32
{
    let arg: i32;

    fn(arg);

    let arg2: ^^i32;

    fn(arg2);

    0
}
```

If a function signature can be matched exactly, the compiler
will not look for signatures using casting. If a signature can
be found exactly or by using casting, the compiler will not look
for signatures requiring referencing or dereferencing.

**Note:** Implicit casting can cast anything from the set
`[i8, u8, i16, u16, i32, u32, i64, u64, i128, u128]` to anything
else in that set, or anything from the set `[f64, f128, f256]`
to anything else in that set. No other types are supported for
implicit casting, **including** any pointers to these types.

# Examples

The remainder of this document is dedicated to examples designed
to get a programmer comfortable with `Oak`. These examples are
largely arranged from easiest to hardest, with the final few
demonstrating the more esoteric regions of `Oak` development
(like interfaces and dialects).

## Number Guessing Game

Difficulty: Easy

This example covers:
- Terminal input and output
- Random number generation
- Preprocessor definitions
- The `printf!` macro

This example requires:
- An understanding of `Oak` syntax
- A Linux terminal
- An `acorn` compiler installation

Let's create a game where you have 5 chances to guess a randomly
selected integer. Upon an incorrect guess, the program will
inform the user if they guessed too high or too low. Let's start
by importing the needed files, which should all be contained
in the "std" package and the "std/rand_inter.oak" file. We will
also activate the "std" rule, so that we can use some common
`Oak` syntactical shortcuts like `for` loops.

```rust
package!("std");    // For printing and standard functions
use_rule!("std");   // For common syntactical shortcuts

include!("std/rand_inter.oak"); // For random numbers

// A skeleton main file, which we will write our program in
let main() -> i32
{
    0
}

```

First, we will need to generate a random number. We will do this
via the `seed_rand()` and `rand(low, high)` functions, both 
available via the "std/rand_inter.oak" file. This file
interfaces with the operating system's random number generator.

The `seed_rand()` function, when called with no arguments, seeds
the random number generator with the current time. `rand`
yields the next number generated by the RNG given its existing
seed. Thus, you must call some form of RNG seeding function
before `rand`.

We will save our random number in a variable named `chosen`.

```rust
// Needed packages
package!("std");
use_rule!("std");
include!("std/rand_inter.oak");

let main() -> i32
{
    // Without the std rule:
    // let chosen: i32;
    // chose = rand(0, 10);

    // With the std rule:
    // let chosen = rand(0, 10);

    // The std rule allows "chosen" to take the type of rand
    let chosen = rand(0, 10);
}

```

But what if we want to use numbers in a different range? We
should pull these numbers out into their own symbols. Let's use
**preprocessor definitions**, so that our symbols do not take
up runtime memory.

Upon compilation, whenever a preprocessor definition is
encountered, it is immediately replaced by its definition. They
are different from variables in that they cannot be redefined.

A preprocessor definition is defined as follows.

```rust
// Not within any code scope
let var_name! = var_definition;

```

We can use this to augment our program as follows.

```rust
// Needed packages
package!("std");
use_rule!("std");

include!("std/rand_inter.oak");

// Preprocessor definitions
let low! = 0;
let high! = 10;

let main() -> i32
{
    let chosen = rand(low!, high!);

    0
}

```

We should also add some output, so that the user knows what
range their guess should inhabit. We can do this using the
`printf!` macro, available via the `std/printf.oak` file. This
macro allows variables to be outputted in the middle of a
string according to a **format string**. Inside a format string,
any "%" will be replaced by the next variable passed. For
example,

```rust
package("std");
use_rule!("std");

include!("std/printf.oak")

let main() -> i32
{
    let a: i32 = 123;
    let b: bool = true;

    printf!(
        "The value of a is %. The value of b is %.\n",
        a,
        b
    );

    0
}
```

We will use this to modify our code as follows.

```rust
// Needed packages
package!("std");
use_rule!("std");

include!("std/printf.oak");
include!("std/rand_inter.oak");

// Preprocessor definitions
let low! = 0;
let high! = 10;

let main() -> i32
{
    let chosen = rand(low!, high!);

    printf!(
        "Guess a number between % and %.\n",
        low!,
        high!
    );

    0
}

```

Next, let's create the loop wherein the user will guess.

```rust
// Needed packages
package!("std");
use_rule!("std");

include!("std/printf.oak");
include!("std/rand_inter.oak");

// Preprocessor definitions
let low! = 0;
let high! = 10;
let num_guesses! = 5;

let main() -> i32
{
    let chosen = rand(low!, high!);

    printf!(
        "Guess a number between % and %.\n",
        low!,
        high!
    );

    // This for loop has been broken over multiple lines, but
    // they can be written within one line as well.
    for (let current_guess = 0;
        current_guess < num_guesses!;
        current_guess += 1)
    {
        // Do guessing here
    }

    0
}

```

We can read an integer from the terminal using the `get_i128`
function. We will store this in a variable called `guess`.

```rust
// Needed packages
package!("std");
use_rule!("std");

include!("std/printf.oak");
include!("std/rand_inter.oak");

// Preprocessor definitions
let low! = 0;
let high! = 10;
let num_guesses! = 5;

let main() -> i32
{
    let chosen = rand(low!, high!);

    printf!(
        "Guess a number between % and %.\n",
        low!,
        high!
    );

    for (let current_guess = 0;
        current_guess < num_guesses!;
        current_guess += 1)
    {
        let guess = to_i32(get_i128());

        if (guess < chosen)
        {
            // Guess is too low
        }
        else if (guess > chosen)
        {
            // Guess is too high
        }
        else
        {
            // Guess is correct
        }
    }

    0
}

```

We will use another variable to shore whether or not the user
has guessed correctly. If this is the case, the `for` loop
should halt. In any case, we should update the user on the
validity of their guess.

```rust
// Needed packages
package!("std");
use_rule!("std");

include!("std/printf.oak");
include!("std/rand_inter.oak");

// Preprocessor definitions
let low! = 0;
let high! = 10;
let num_guesses! = 5;

let main() -> i32
{
    let chosen = rand(low!, high!);
    let has_chosen_incorrectly = true;

    printf!(
        "Guess a number between % and %.\n",
        low!,
        high!
    );

    let current_guess: i32;
    for (current_guess = 0;
        current_guess < num_guesses! && has_chosen_incorrectly;
        current_guess += 1)
    {
        print("Guess: ");
        let guess = to_i32(get_i128());

        if (guess < chosen)
        {
            print("Too low...\n");
        }
        else if (guess > chosen)
        {
            print("Too high...\n");
        }
        else
        {
            print("You guessed it!\n");
            has_chosen_incorrectly = false;
        }
    }

    0
}

```

Let's try it out. Compiling via the command
`acorn /PATH/TO/MAIN.oak -o guessing_game.out` creates the
executable `guessing_game.out`, which can then be run via
the command `./guessing_game.out`. Here's the output from an
example run.

```sh
[user@hostname guessing_game]$ ./a.out
Guess a number between 0 and 10.
Guess: 5
Too high...
Guess: 2
Too low...
Guess: 4
Too high...
Guess: 3
You guessed it!
```

It works!

## Grep- File RegEx Search

Difficulty: Easy

This example covers:
- Terminal output
- Command line arguments
- Regular expressions
- File input and output
- Panics
- Colored terminal output
- `std/bool` and the `bool` rule

This example requires:
- An understanding of `Oak` syntax
- A Linux terminal
- An `acorn` compiler installation
- A `boost` `C++` library installation
- A basic understanding of regular expressions

Let's create an `Oak` program which will take in a filename and
a regular expression as command line arguments, and output every
line in that file which matches the expression. This is similar
to the `grep` utility in `POSIX` systems. A typical call to this
program might look something like this:

```sh
./oak_grep.out file.txt "regex+ pat?tern h*ere"
```

Let's start by reading in the two command line arguments.

```rust
package!("std");
use_rule!("std");

include!("std/panic_inter.oak");
include!("std/string.oak");
include!("std/printf.oak");

let main(argc: i32, argv: []str) -> i32
{
    // If argc != 3, panic w/ the given message.
    assert!(
        argc == 3,
        "Please provide exactly 3 command line arguments."
    );

    // Get the command line arguments
    let filepath: string = argv.Get(1);
    let pattern: string = argv.Get(2);
    
    printf!(
        "Got filepath '%' and pattern '%'\n",
        filepath,
        pattern
    );

    0
}

```

```bash
[user@host dir]$ acorn tests/oak_grep_ex.oak -o oak_grep.out
[user@host dir]$ ./oak_grep.out file.txt "regex+ pat?tern h*ere"
Got filepath 'file.txt' and pattern 'regex+ pat?tern h*ere'
```

It works! Now, lets open the given filepath as a `i_file`. This
requires the use of the `std/file_inter.oak` file, so we will
also add that to our list of inclusions.

```rust
package!("std");
use_rule!("std");

include!("std/panic_inter.oak", "std/file_inter.oak",
    "std/string.oak");

let main(argc: i32, argv: []str) -> i32
{
    // If argc != 3, panic w/ the given message.
    assert!(
        argc == 3,
        "Please provide exactly 3 command line arguments."
    );

    // Get the command line arguments
    let filepath: string = argv.Get(1);
    let pattern: string = argv.Get(2);
    
    // Open filepath
    let file: i_file;
    file.open(filepath.c_str());

    // Line variable to read the file into, line by line
    let line: string;

    // Read the first line of the file
    line = file.getline(to_u128(512));

    // Close the file
    file.close();

    0
}

```

Now we have the file, but we have no way to iterate over it! To
do this, we will include the `std/bool.oak` file in order to
use the `bool` rule, which adds the `not` keyword (as well as
boolean negation in general). Using this, we can use the
`i_file`'s `eof` function, which returns true if the end of the
file has been reached.

```rust
package!("std");
use_rule!("std");

include!("std/panic_inter.oak", "std/file_inter.oak",
    "std/string.oak");

include!("std/bool.oak");
use_rule!("bool"); // STD Oak dialect notation for use_rule!("bool")

let main(argc: i32, argv: []str) -> i32
{
    // If argc != 3, panic w/ the given message.
    assert!(
        argc == 3,
        "Please provide exactly 3 command line arguments."
    );

    // Get the command line arguments
    let filepath: string = argv.Get(1);
    let pattern: string = argv.Get(2);
    
    // Open filepath
    let file: i_file;
    let line: string;
    file.open(filepath.c_str());

    while not file.eof()
    {
        line = file.getline(to_u128(512));
    }

    file.close();

    0
}

```

Now we can iterate over the line! However, we still need to do
the actual RegEx searching. For this, let's include the
`extra/regex_inter.oak` file, and use the `regex` struct and
`regex_search` function. 

```rust
package!("std");
use_rule!("std");

include!("std/panic_inter.oak", "std/file_inter.oak",
    "std/string.oak", "extra/regex_inter.oak", "std/bool.oak");
use_rule!("bool");

let main(argc: i32, argv: []str) -> i32
{
    // If argc != 3, panic w/ the given message.
    assert!(
        argc == 3,
        "Please provide exactly 3 command line arguments."
    );

    // Get the command line arguments
    let filepath: string = argv.Get(1);
    let pattern: string = argv.Get(2);
    
    let pattern_reg: regex = pattern;

    // Open filepath
    let file: i_file;
    let line: string;
    file.open(filepath.c_str());

    while not file.eof()
    {
        line = file.getline(to_u128(512));

        if line.regex_search(pattern_reg)
        {
            print(line);
        }
    }

    file.close();

    0
}

```

It's looking great! But we can be a little fancier. Let's print
the lines which match in green text using the `std/color.oak`
file. Using this, you just have to call the `.green()` method
on any string to turn it green (and `.red()` for red, etc.).
While we're here, let's count the number of matches we find,
and output that when we finish.

```rust
package!("std");
use_rule!("std");

include!("std/panic_inter.oak", "std/file_inter.oak",
    "std/string.oak", "std/printf.oak",
    "extra/regex_inter.oak", "std/color.oak", "std/bool.oak");
use_rule!("bool");

let main(argc: i32, argv: []str) -> i32
{
    // If argc != 3, panic w/ the given message.
    assert!(
        argc == 3,
        "Please provide exactly 3 command line arguments."
    );

    // Get the command line arguments
    let filepath: string = argv.Get(1);
    let pattern: string = argv.Get(2);
    
    let pattern_reg: regex = pattern;

    let num_matches = 0;

    // Open filepath
    let file: i_file;
    let line: string;
    file.open(filepath.c_str());

    while not file.eof()
    {
        line = file.getline(to_u128(512));

        if line.regex_search(pattern_reg)
        {
            print(line.green());
            num_matches += 1;
        }
    }

    file.close();

    print("\n");

    if num_matches == 0
    {   
        print("No matches!\n".red());
    }
    else
    {
        begin_green_bold();
        printf!("% lines matched.\n", num_matches);
        end_effects();
    }

    0
}

```

Let's try it out! (Note: This document is plaintext, so the
terminal colors will not appear here)

```bash
[user@host oak]$ acorn oak_grep.oak -o oak_grep.out
[user@host oak]$ ./oak_grep.out oak_grep.oak "print"
    "std/string.oak", "std/printf.oak",
            print(line.green());
    print("\n");
        print("No matches!\n".red());
        printf!("% lines matched.\n", num_matches);

5 lines matched.
```

It works!

## Collatz Sequence

Difficulty: Moderate

This example covers:
- Terminal output
- Recursive functions

This example requires:
- An understanding of `Oak` syntax
- A Linux terminal
- An `acorn` compiler installation
- An undergraduate understanding of mathematics
- Understanding of advanced mathematical notation

For this example, we will write a short program to yield the
**Collatz sequence** of a given positive integer $n$. A (heavily
mathematical) definition of said sequence precedes the exercise.

For a given positive integer $n$, the **Collatz sequence** of
$n$ is defined as the numbers resulting from the repeated
application of the following set of rules.

- If $n_i$ is even, $n_{i + 1} = \frac{n}{2}$.
- If $n_i$ is odd, $n_{i + 1} = 3n + 1$

These rules can be translated from english to math as below.

$$
    n_{i + 1} =
    \begin{cases}
        \frac{n}{2} & n \equiv_2 0 \\
        3n + 1      & n \equiv_2 1 \\
    \end{cases}
$$

This process is iterated until the output is equal to 1. The
Collatz sequences of the first few positive integers are shown
below.

- 1
- 2, 1
- 3, 10, 5, 16, 8, 4, 2, 1
- 4, 2, 1
- 5, 16, 8, 4, 2, 1
- 6, 3, 10, 5, 16, 8, 4, 2, 1
- 7, 22, 11, 34, 17, 52, 26, 13, 40, 20, 10, 5, 16, 8, 4, 2, 1
- 8, 4, 2, 1
- 9, 28, 14, 7, 22, 11, 34, 17, 52, 26, 13, 40, 20, 10, 5, 16,
  8, 4, 2, 1
- 10, 5, 16, 8, 4, 2, 1

The **Collatz conjecture** states that, for all positive
integers, there exists some finite number of repeated iterations
which will reduce said number to 1. That is to say, that all
Collatz sequences are of finite length. Representing this claim
mathematically,

$$
\begin{aligned}
    & n_{i + 1} =
    \begin{cases}
        \frac{n}{2} & n \equiv_2 0 \\
        3n + 1      & n \equiv_2 1 \\
    \end{cases} \\
    & [ \forall n_0 \in \mathbb{Z}^+ ]
        [ \exists j \in \mathbb{Z}^+ ] : n_j = 1
\end{aligned}
$$

We will designate the mathematical function mapping some $n_i$
to its $n_{i + 1}$ the **Collatz function**. This function is
represented in `Oak` below.

```rust
package!("std");
use_rule!("std");

include!("std/math.oak");

// Takes in n_i and returns n_{i + 1}
let collatz(n: u128) -> u128
{
    let out: u128;
    out = n;

    if (even(n))
    {
        out /= to_u128(2);
    }
    else
    {
        out *= to_u128(3);
        out += to_u128(1);
    }

    out
}

```

We can subsequently create the following function to print
the Collatz sequence of a given number.

```rust
let print_collatz_sequence(n: u128) -> void
{
    print(n);
    print(" ");

    if (n > to_u128(1))
    {
        let next: u128;
        next = collatz(n);

        print_collatz_sequence(next);
    }
    else
    {
        print("\n");
    }
}

```

As a tail recursive function, this can easily be rewritten
iteratively to conserve stack space- this is left as an exercise
for the reader.

Our full program is as follows.

```rust
package!("std");
use_rule!("std");

include!("std/math.oak");

// Takes in n_i and returns n_{i + 1}
let collatz(n: u128) -> u128
{
    let out: u128;
    out = n;

    if (even(n))
    {
        out /= to_u128(2);
    }
    else
    {
        out *= to_u128(3);
        out += to_u128(1);
    }

    out
}

let print_collatz_sequence(n: u128) -> void
{
    print(n);
    print(" ");

    if (n > to_u128(1))
    {
        let next: u128;
        next = collatz(n);

        print_collatz_sequence(next);
    }
    else
    {
        print("\n");
    }
}

let main() -> i32
{
    for (let i: u128 = 1; i < to_u128(10); i += 1)
    {
        print_collatz_sequence(i);
    }

    0
}

```

The following is the output from this program.

```
1 
2 1 
3 10 5 16 8 4 2 1 
4 2 1 
5 16 8 4 2 1 
6 3 10 5 16 8 4 2 1 
7 22 11 34 17 52 26 13 40 20 10 5 16 8 4 2 1 
8 4 2 1 
9 28 14 7 22 11 34 17 52 26 13 40 20 10 5 16 8 4 2 1 
```

This is correct.

## Creating a RegEx Interface

Difficulty: Hard

This example covers:
- The `Oak` mangler
- The `link!` compiler macro
- Interfacial files

This example requires:
- An understanding of `Oak` syntax
- An understanding of `C++` syntax
- An understanding of `makefile`s.
- A Linux terminal
- An `acorn` compiler installation
- A `boost` `C++` library installation
- A basic understanding of regular expressions

In this example, we will be creating a version of the `Oak`
`extra/regex_inter.oak` interfacial file. This will allow us to
access the `C++` `boost` RegEx library from within `Oak`, and
give us a better understanding of how to create interfacial
files and libraries in `Oak`.

Let's begin by creating a folder to contain our interfacial
library. We will name our library `re`, for Regular Expressions.
The `acorn` compiler conveniently has a command for creating a
new library- `acorn -w NAME`, where `NAME` is the name of the
package. For our purposes, we will navigate to the folder where
we want our library to be, and call `acorn -w re`. This will
create the `re` folder, where we will do the rest of this
exercise.

```bash
acorn -w re     # Creates the package
cd re           # Moves into the new dir
ls              # Shows the files acorn made
```

You can fill in your details in `re/oak_package_info.txt` if you
want; This step would be required if you wanted to publish this
package. Additionally, it initializes a git repo and creates a
linkage file `re.oak`. We will need three more files here: the
two halves of the interface, and a makefile. We will use the
following command to create these.

```bash
# From the re folder
touch re_inter.oak re_inter.cpp Makefile    # Create files
```

Let's explore the `boost` RegEx functions we will be interfacing
with. First, the **regex match function**. We will focus on two
instances of this; The two-argument and the three-argument
versions.

The two-argument `boost::regex_match` function takes a string
for the text to search, and a `boost::regex` object representing
the pattern.

The three-argument `boost::regex_match` function takes a string
for the text, a `boost::smatch` object to record the results
into, and a `boost::regex` for the pattern.

Both of these functions return booleans. Clearly, we will need
an `Oak` wrapper struct for the `boost::regex` and
`boost::smatch` classes, as well as wrapper functions for each
of these functions. We will use the built-in `std/string.oak`
implementation of a string. Let's begin constructing our `C++`
file.

```cpp
// re/re_inter.cpp

#include <oak/std_oak_header.hpp>
#include <boost/regex.hpp>
#include <oak/string.c>

// An Oak wrapper class for boost::regex
struct regex
{
    boost::regex *re;   // Uses dynamically allocated memory

    // To get to 32 bytes
    char padding[32 - sizeof(boost::regex *)];
};

// An Oak wrapper class for boost::smatch
struct regex_smatch
{
    boost::smatch m;

    // To get to 96 bytes
    char padding[96 - sizeof(boost::smatch)];
};

// Ensures the C++ compiler will not mangle symbol names
extern "C"
{
    // Interfaces with the two-argument boost::regex_match
    bool regex_match(string *text, regex *pattern)
    {
        ;
    }

    // Interfaces with the three-argument boost::regex_match
    bool regex_match(string *text,
                    regex *pattern,
                    regex_smatch *into)
    {
        ;
    }
}

```

This is a solid start, but we will need to do some legwork
before we are fully able to interface. The `boost` libraries
require a `C++` `std::string` rather than the custom `Oak`
`string` which is passed as a parameter. We can avoid this by
using the underlying `data` struct member, which is a
contiguous `C`-style string. This means we can construct a
`std::string` from this raw `C`-string and pass that in. For our
interfacial structs, we will simply pass the underlying member.
This leaves us with the following code.

```cpp
// re/re_inter.cpp

#include <oak/std_oak_header.hpp>
#include <boost/regex.hpp>
#include <oak/string.c>             // Oak strings
#include <string>                   // C++ strings

// An Oak wrapper class for boost::regex
struct regex
{
    boost::regex re;

    char padding[16]; // To get to 32 bytes
};

// An Oak wrapper class for boost::smatch
struct regex_smatch
{
    boost::smatch m;

    char padding[16]; // To get to 96 bytes
};

extern "C"
{
    // Interfaces with the two-argument boost::regex_match
    bool regex_match(string *text, regex *pattern)
    {
        return boost::regex_match(std::string(text->data), pattern->re);
    }

    // Interfaces with the three-argument boost::regex_match
    bool regex_match(string *text,
                    regex *pattern,
                    regex_smatch *into)
    {
        return boost::regex_match(std::string(text->data),
                                into->m,
                                pattern->re);
    }
}

```

Now we have successfully wrapped the `boost` functions, but we
have one more hurdle: The **`Oak` mangler**. Because `Oak`
translates into `C` for compilation, it must be able to
disambiguate between functions using only their names. Note how
both of our functions are called `regex_match`: If we didn't do
anything to these names, `C` would become confused. Thus, the
`Oak` compiler **mangles** the symbol names to include their
full types. The mangler can be unintuitive, but close inspection
of the `std` interfacial files should be enough to understand
it.

For a function which in `Oak` would be written

```rust
let regex_match(text: ^string, pattern: ^regex) -> bool;
```

The corresponding mangled symbol names is

```cpp
regex_match_FN_PTR_string_JOIN_PTR_regex_MAPS_bool
```

This mangled name, **not** the original name, is what the `C`
linker will look for. Thus, we must modify our function names
to align with the mangler. Note that struct / enum names are not
mangled by `Oak`, and thus remain unmodified. The following code
demonstrates this.

```cpp
// re/re_inter.cpp

#include <oak/std_oak_header.hpp>
#include <boost/regex.hpp>
#include <oak/string.c>             // Oak strings
#include <string>                   // C++ strings

// An Oak wrapper class for boost::regex
struct regex
{
    boost::regex re;

    char padding[16]; // To get to 32 bytes
};

// An Oak wrapper class for boost::smatch
struct regex_smatch
{
    boost::smatch m;

    char padding[16]; // To get to 96 bytes
};

extern "C"
{
    // Interfaces with the two-argument boost::regex_match
    bool regex_match_FN_PTR_string_JOIN_PTR_regex_MAPS_bool(
        string *text, regex *pattern)
    {
        return boost::regex_match(std::string(text->data), pattern->re);
    }

    // Interfaces with the three-argument boost::regex_match
    bool regex_match_FN_PTR_string_JOIN_PTR_regex_JOIN_PTR_regex_smatch_MAPS_bool(
        string *text,
        regex *pattern,
        regex_smatch *into)
    {
        return boost::regex_match(std::string(text->data),
                                into->m,
                                pattern->re);
    }
}

```

Now we are finished with the `C++` portion of our interface for
these two functions. We can now create an `Oak` file to
interface with it. **Note that this is separate from the**
**re/re.oak file that `acorn` created for us. Linkage files**
**should not be interfaces.** Let's begin by creating the `Oak`
versions of the interfacial structs.

```rust
// re/re_inter.oak

package!("std");
use_rule!("std");
include!("std/string.oak");

let regex: struct
{
    ,
}

let regex_smatch: struct
{
    ,
}

```

It is very important that we match the size of our interfacial
structs to their `C++` counterparts. This is why we used padding
to make their sizes a nice number. The `std/interface.oak` file
provides immutable / hidden memory block types of varying size.
These blocks are used to build interfaces like these, where the
modification of internal data by an `Oak` program would cause
the `C++` struct to stop functioning. Using these padding
blocks, we arrive at the following code.

```rust
// re/re_inter.oak

package!("std");
use_rule!("std");

include!("std/string.oak", "std/interface.oak");

let regex: struct
{
    __a: hidden_32_bytes,
}

let regex_smatch: struct
{
    __a: hidden_64_bytes,
    __b: hidden_32_bytes,
}

```

The double underscore prefix is recommended for interfacial
padding members such as these. Now, we can add the function
signatures which will be filled in at link-time by the `C++`
implementations.

```rust
// re/re_inter.oak

package!("std");
use_rule!("std");

include!("std/string.oak", "std/interface.oak");

let regex: struct
{
    __a: hidden_32_bytes,
}

let regex_smatch: struct
{
    __a: hidden_64_bytes,
    __b: hidden_32_bytes,
}

let regex_match(text: ^string, pattern: ^regex) -> bool;
let regex_match(text: ^string, pattern: ^regex, into: ^smatch)
    -> bool;

```

Note that `Oak` allows at most 96 non-whitespace characters per
line. However, its complete disregard for all whitespace after
lexing means that all code can be split across any number of
lines.

Now, we must add a way for `acorn` to know to include the `C++`
implementations at link-time. This takes the form of a
**`link!` macro**.

```rust
// re/re_inter.oak (excerpt)

package!("std");
use_rule!("std");

include!("std/string.oak", "std/interface.oak");
link!("re/re_inter.o");

```

Now we have a basic interfacial library! However, for it to be
useful to use, we must have ways of accessing and modifying the
data in our interfacial structs safely. Since `Oak` only allows
zero-argument constructors, we must initialize the `regex`
struct via the `Copy` (`=`) operator. Additionally, there are
several fields within `smatch` which we need to be able to
access. We will also create the function signatures for these
here.

```rust
// re/re_inter.oak

package!("std");
use_rule!("std");

include!("std/string.oak", "std/interface.oak");

link!("re/re_inter.o");

let regex: struct
{
    __a: hidden_32_bytes,
}

let regex_smatch: struct
{
    __a: hidden_64_bytes,
    __b: hidden_32_bytes,
}

let regex_match(text: ^string, pattern: ^regex) -> bool;
let regex_match(
    text: ^string,
    pattern: ^regex,
    into: ^regex_smatch) -> bool;

// To be externally defined in C++
let New(self: ^regex) -> void;
let Del(self: ^regex) -> void;

let Copy(self: ^regex, pattern: string) -> void;
let Copy(self: ^regex, pattern: str) -> void;

// To be externally defined in C++
let New(self: ^regex_smatch) -> void;

let size(self: ^regex_smatch) -> u64;
let str(self: ^regex_smatch) -> string;
let Get(self: ^regex_smatch, index: u32) -> string;

```

Now, let's create the `C++` implementation of these functions.

```cpp
#include <boost/regex.hpp>
#include <string>

#include "oak/std_oak_header.h"
#include "oak/std/string.c"

struct regex
{
    boost::regex *re;

    char padding[32 - sizeof(boost::regex *)]; // To get to 32 bytes
};

struct regex_smatch
{
    boost::smatch m;

    char padding[96 - sizeof(boost::smatch)]; // To get to 96 bytes
};

extern "C"
{
    bool regex_match_FN_PTR_string_JOIN_PTR_regex_MAPS_bool(string *text, regex *pattern)
    {
        return boost::regex_match(std::string(text->data), *pattern->re);
    }

    bool regex_match_FN_PTR_string_JOIN_PTR_regex_JOIN_PTR_regex_smatch_MAPS_bool(string *text, regex *pattern, regex_smatch *into)
    {
        return boost::regex_match(std::string(text->data), into->m, *pattern->re);
    }

    //////////// Methods ////////////

    void New_FN_PTR_regex_MAPS_void(regex *self)
    {
        memset(self, 0, sizeof(regex));
        self->re = nullptr;
    }

    void Del_FN_PTR_regex_MAPS_void(regex *self)
    {
        if (self->re != nullptr)
        {
            delete self->re;
        }
    }

    void Copy_FN_PTR_regex_JOIN_PTR_string_MAPS_void(regex *self, string *pattern)
    {
        if (self->re != nullptr)
        {
            delete self->re;
        }

        self->re = new boost::regex(pattern->data);
    }

    void Copy_FN_PTR_regex_JOIN_str_MAPS_void(regex *self, str pattern)
    {
        if (self->re != nullptr)
        {
            delete self->re;
        }

        self->re = new boost::regex(pattern);
    }

    void New_FN_PTR_regex_smatch_MAPS_void(regex_smatch *self)
    {
        memset(self, 0, sizeof(regex_smatch));
    }

    u64 size_FN_PTR_regex_smatch_MAPS_u64(regex_smatch *self)
    {
        return self->m.size();
    }

    string str_FN_PTR_regex_smatch_MAPS_string(regex_smatch *self)
    {
        string out;
        out.size = self->m.str().size();

        out.data = new i8[out.size + 1];
        strcpy(out.data, self->m.str().c_str());
        out.data[out.size] = '\0';

        return out;
    }

    string Get_FN_PTR_smatch_JOIN_u32_MAPS_string(regex_smatch *self, u32 index)
    {
        string out;
        std::string from = self->m[index].str();

        out.size = from.size();

        out.data = new i8[out.size + 1];
        strcpy(out.data, from.c_str());
        out.data[out.size] = '\0';

        return out;
    }
}

```

Now we've got the `C++` and `Oak` files in a functional state,
but we still need to give `acorn` directions on how and what to
build upon installation. This is where the `makefile` comes in.
We will add the following lines to our `makefile`.

```makefile
CC := clang++
FLAGS := -pedantic -Wall
LIBS := -lboost_regex

all:    regex_inter.o

regex_inter.o:  regex_inter.cpp
    $(CC) $(FLAGS) -c -o regex_inter.o regex_inter.cpp $(LIBS)

```

Finally, we must modify the `re/re.oak` linkage file so that it
includes the `re/re_inter.oak` interfacial file.

```rust
// re/re.oak

include!("re/re_inter.oak");

```

This is all that is needed for installation. Now, we can run
the command `acorn -S re` from the folder which contains the
package folder `re` to install our package.

```bash
acorn -S re
```

Let's test it out! We'll create a test file first, and give it a
skeleton main function.

```bash
touch re_test.oak   # Create our test file
```

```rust
// re_test.oak
package!("std");
use_rule!("std");

package!("re");

let main() -> i32
{
    0
}

```

To compile this program, we will use the command

```bash
# Compile re_test.oak to re_test.out
acorn re_test.oak -o re_test.out
```

To run it after compilation, we will use

```bash
./re_test.out
```

Let's test our RegEx interface by attempting a few RegEx
matches. We will test the pattern `abra*ca?dabra+` against the
strings `abrcadabraaa`, `abraaaacdabra`, `abrcdabr`, and
`abarcdabrar`. The first two strings should be a match, and the
second two should be a fail. Here's what our test file should
look like with these tests implemented.

```rust
// Use the standard Oak dialect
package!("std");
use_rule!("std");

package!("re");
include!("std/string.oak");

let main() -> i32
{
    let pattern: regex = "abra*ca?dabra+";

    let string_one: string = "abrcadabraaa";
    let string_two: string = "abraaaacdabra";
    let string_three: string = "abrcdabr";
    let string_four: string = "abarcdabrar";

    // Test string 1
    if regex_match(string_one, pattern)
    {
        print("String one matched.\n");
    }
    else
    {
        print("String one did not match.\n");
    }

    // Test string 2
    if regex_match(string_two, pattern)
    {
        print("String two matched.\n");
    }
    else
    {
        print("String two did not match.\n");
    }

    // Test string 3
    if regex_match(string_three, pattern)
    {
        print("String three matched.\n");
    }
    else
    {
        print("String three did not match.\n");
    }

    // Test string 4
    if regex_match(string_four, pattern)
    {
        print("String four matched.\n");
    }
    else
    {
        print("String four did not match.\n");
    }

    print("Done with everything!\n");

    0
}

```

Now, lets test it out!

```bash
acorn tests/regex_ex.oak -o regex_ex.out
./regex_ex.out
```

```
<Output>
String one matched.
String two matched.
String three did not match.
String four did not match.
Done with everything!
<End output>
```

It works!

The techniques used herein can easily be adapted to create a
wide variety of interfacial files.

## A Server and Client Using Sockets, RegEx, and File IO

Difficulty: Hard

This example covers:
- Terminal input and output
- Regular expressions
- File input and output
- Socket programming

This example requires:
- An understanding of `Oak` syntax
- A Linux terminal
- An `acorn` compiler installation
- A `boost` `C++` library installation
- A basic understanding of regular expressions
- A basic understanding of socket programming

In this example, we will be using the `Oak` standard library to
create a file server and client. The client will be able to make
requests to the server, and the server will send back some file
matching the request. Let's start by creating the necessary
files.

```bash
# Create Oak files for the file server and client
touch file_server.oak file_client.oak
```

We'll add a skeleton into each.

```rust
// file_server.oak
package!("std");
use_rule!("std");

let main() -> i32
{
    0
}

```

```rust
// file_client.oak
package!("std");
use_rule!("std");

let main() -> i32
{
    0
}

```

We want our file server to take in a request for a given file
in the form of a regular expression, and send back the first
file which matches it. Thus, it will need the `string`,
`file_inter`, `regex_inter`, and `sock_inter` `Oak` files, all
of which are found in the `std` package (although not included
by default).

The client program will only need to make requests and save
files, so it will only need `string`, `file`, and `sock_inter`.

Additionally, we will include `std/printf.oak` in both files for
convenient printing. Now, the skeleton files should look like
the following.

```rust
// file_server.oak
package!("std");
use_rule!("std");

include!("std/string.oak",
        "std/file_inter.oak",
        "extra/regex_inter.oak",
        "std/sock_inter.oak",
        "std/printf.oak");

let main() -> i32
{
    0
}

```

```rust
// file_client.oak
package!("std");
use_rule!("std");

include!("std/string.oak",
        "std/file_inter.oak",
        "std/sock_inter.oak",
        "std/printf.oak");

let main() -> i32
{
    0
}

```

Now, let's focus on the server side of things. When not
connected to a client, the server should listen for one. When
one arrives, it should wait for a command from it. Upon hearing
a command from the client, it should search for a file matching
the given regular expression. If not found, it should send back
an error message. Otherwise, it should send the file.

When sending a file, we will first have the server send the size
of the file. The client will echo back this size to indicate
that it has received it. Then, the server will send the file.
The `Oak` socket interface is always TCP, so the file will be
broken apart automatically. Requests will work in the same way-
size, then message.

An error opening the file will be indicated by sending a size of
zero.

The above outline roughly translates to the code below.

```rust
// file_server.oak
package!("std");
use_rule!("std");

include!("std/string.oak",
        "std/file_inter.oak",
        "extra/regex_inter.oak",
        "std/sock_inter.oak",
        "std/printf.oak",
        "std/conv_extra.oak");

// Sizes will be sent using this many chars
let size_chars! = to_u128(8);

// Handle a single request
let do_request(server: ^server_sock, request: string) -> void;

let main() -> i32
{
    let is_listening = true;

    // Create empty server socket
    let server: server_sock;

    // Initialize server socket to localhost on port 1234
    // Save result of initialization into result
    let result = server = ("127.0.0.1", to_u16(1234));

    let size_str: string;
    let size: u128;
    let request: string;

    while is_listening
    {
        // Listen for a connection
        // save connection status in result
        result = server.listen();

        size_str = server.recv(size_chars!);
        size = to_u128(to_i128(size_str));

        if size != to_u128(0)
        {
            request = server.recv(size);

            if request == "quit"
            {
                is_listening = false;
            }
            else
            {
                do_request(server, request);
            }
        }
        else
        {
            is_listening = false;
        }
    }
    
    0
}

```

Great! Now we've got the basic outline of our server. However,
we still need to define `do_request`. A request should be a
regular expression, and our `do_request` function should send
the first file it finds matching this description over the
server socket. Once we have the exact filename to send, this
won't be so hard. However, how do we find the first filename
that matches the given regular expression? Let's do this the
"hacky" way, using the `std/sys_inter.oak` file.

We will tell the system to save a list of all local files to a
temporary `.txt` file, then read that file in. From this, we can
iterate through until we find a filename which matches the given
regular expression. Then, we will load the matched file into
memory and send it over the socket. After this, we must remember
to delete our temporary `.txt` file. Let's create a skeleton of
our `do_request` function.

```rust
// file_server.oak
// (code above hidden)

// This line will be integrated into the earlier include! block
include!("std/sys_inter.oak");

// Handle a single request
let do_request(server: ^server_sock, request: string) -> void
{
    // Make system call to list all files in this directory

    // Open file where the above output was saved

    // Iterate over lines in this file
    //      If the current line matches request
    //          Load this file
    //          Send this file
    //          Break

    // If no matches were found
    //      Send error signal
    
    // Delete temporary text file
}

// (code below hidden)

```

The `Linux` command we will be using to save a list of all local
files to a temporary text file is `ls > temp.txt`. After this,
`temp.txt` will contain all the files in the current directory.

```rust
// file_server.oak
// (code above hidden)

include!("std/panic_inter.oak");
include!("std/bool.oak");
use_rule!("bool");

// Handle a single request
// Assumes the server is running on Linux
let do_request(server: ^server_sock, request: string) -> void
{
    // Make system call to list all files in this directory
    let result = sys("ls > temp.txt");
    assert!(result == 0);

    let pattern: regex = request;

    // Open file where the above output was saved
    let file: i_file;
    file.open("temp.txt");

    let line: string;
    let found: bool = false;

    // Iterate over lines in this file
    while !file.eof() && !found
    {
        // Read a line of at most 256 chars
        line = file.getline(256);

        // If the current line matches request
        if (line.regex_match(pattern))
        {
            // Load this file
            let to_send: i_file;
            to_send.open(line);

            let contents: string;
            contents = to_send.read(to_send.size());

            // Send this file
            server.send();
            server.send(contents);

            to_send.close();
            
            // Break
            found = true;
        }
    }

    file.close();

    // If no matches were found
    if !found
    {
        // Send error signal
    }
    
    // Delete temporary text file
    sys("rm temp.txt");
}

// (code below hidden)

```

# Appendix

## `acorn` and `Oak` source code

`acorn` and the `Oak` are free and open source software. They
are available with the GPLv3 at github.com/jorbDehmel/oak.

## `acorn` Error Lookup

The following table maps a **compile-mode** acorn call return
value to its meaning. If `acorn` executes its result via the
`-E` flag, it will return the return value of that execution, no
matter what.

Code | Meaning
-----|-----------------
 0   | Success
 2   | Runtime error caught
 3   | Unknown error caught
 7   | Failed to create test suite file
 10  | Aborted Acorn update or install

Most compile-time errors will have meaningful descriptions in
their outputs, and thus do not return different values.

## `acorn` Error Classes

Error Class      | Meaning
-----------------|----------------------------------------------
runtime_error    | Miscellaneous error not attributable below
sequencing_error | Syntax error within symbol tree construction
generic_error    | Error instantiating or entering a template
package_error    | Error loading a package
rule_error       | Error processing rule
parse_error      | Error parsing token stream (depreciated)
unknown          | An unknown bug- issue w/ `acorn`

**Note:** Unknown errors represent bugs in `acorn`, and should
be reported as such. Errors without reasons are unacceptable.

## Disclaimer

The `Oak` programming language outlined here bears no relation
nor resemblance to any other programming languages, frameworks
or prototypes of the same name. Unique names are increasingly
difficult to find for programming languages. The `Oak`
referenced here was developed independently from any similarly
named items.

## License

`Oak` is protected by the GPLv3.
