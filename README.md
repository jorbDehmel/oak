
# The Oak Programming Language

Jordan Dehmel, jdehmel@outlook.com, github.com/jorbDehmel/oak

----------------------------------------------------------------

## Overview

`Oak` is a modern, compiled, low-level, statically-typed
programming language. It uses `Rust`-like typing, without
`Rust`'s lifetimes system. It is analogous to `C++` with
stronger macro support, modern typing, compile-time syntax
modification and integrated package management.

This document outlines the basics of `Oak`, as well as the
canonical formatting of `Oak` code. Deviation from this
formatting style is unadvisable, and considered bad form.

`Oak` is, as of now, a translated language; `Oak` code is
translated via the `acorn` utility (see later) into `C++`.
`acorn` can also compile `Oak` into object code, or link it to
create executables.

`Oak` has compile-time-modifiable syntax (see the section on
preprocessor rules), making it highly customizable and flexible
in a way that no other mainstream languages are. It supports the
creation of "dialects", which are `Oak` variants which use
preprocessor rules to support independent syntactical
structures.

## Names

Name      | Meaning
----------|-----------------------------------------------------
`Oak`     | The base `Oak` programming language.
`Acorn`   | The `Oak` translator / compiler.
`Sapling` | The sub-language for creating `Oak` dialects/rules.
Dialect   | A syntactically-modified branch of `Oak`.

## Compilation, Installation, and Uninstallation

To install, open a terminal in this folder and run
`make install`. This repo is only compatible with Linux. This
will compile and install `Oak`, as well as the standard `Oak`
package. To uninstall, open this folder in terminal and run
`make uninstall`. If you've already deleted this folder, you can
run `sudo rm -rf /usr/include/oak /usr/bin/acorn`. Both of these
will accomplish the same thing.

There is also a `PKGBUILD` file included in this directory. If
you use Arch Linux, you can just download this and install it
via `makepkg -si`.

## For More Help

For examples on the concepts presented in this document, see the
`./oak_demos/` directory included in this `git` repo. The files
therein are labeled `CONCEPT_test.oak`, where `CONCEPT` is the
concept covered within. These files are both demos and unit
tests for `Oak` and `acorn`. Consequently, you can compile all
demos via `make test`.

## Syntax Overview

In `Oak`, a variable is declared as follows.

`let NAME: TYPE;`

For instance, a boolean named  `a` would be declared:

`let a: bool;`

You should read the `:` operator as "be of type". Thus, the
above statement reads "let a be of type bool". Functions are
declared as follows.

`let NAME(ARG_1: ARG_1_TYPE, ..) -> RETURN_TYPE;`

For instance, a function that in `C++` would be declared
`bool isBiggerThanFive(int a);` would in `Oak` be declared
`let is_bigger_than_five(a: i32) -> bool;`. See later for atomic
type conversions from `C` (IE how `int` became `i32` here). You
should read the `->` operator as "yield". Thus, this function
should be read "let is_bigger_than_five be a function taking
parameter a of type i32 and yielding a bool".

Additionally, the `C++` code

```
bool isBiggerThanFive(int a)
{
    return a > 5;
}
```

would convert to

```
let is_bigger_than_five(a: i32) -> bool
{
    a > 5
}
```

Note that leaving off a semicolon is equivalent to `C++`'s
`return` statement. `Oak` has no `return` keyword.

Pointers are `^` (IE a pointer to a bool is `^bool`). The "get
address" operator is `@`.

Syntax is fluid and user-modifiable in `Oak`. For more
information about this aspect, see the `Preproc Rules` section
below.

The `:` and `->` operators come from mathematics. For
instance, the mathematical statement

$$
    f: \mathbb{R} \to \mathbb{R}
$$

is equivalent to the code

```
let f(_: f64) -> f64;
```

Both of these statements can be read "let f be a function
mapping a single real number to another".

As a final example for this section, examine the "Hello world"
program below.

```
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

## Pointers

Pointers are variables which hold a memory address. This address
could refer to a single object in memory, or the start of a
contiguous block of like-typed objects. To get the address of a
variable in `Oak`, use the `@` symbol. To get the value that a 
pointer references, use the `^` symbol. Pointer types are the
`^` symbol, followed by the type they point to. You can have
pointers.

The `alloc!` macro for allocating new memory on the heap returns
a pointer to the memory it allocated, should it be successful.
Similarly, the `free!` and `free_arr!` macros free the memory
of the pointer they are passed.

Mutability (ability to change data) is determined by
referencing. If an object is passed into a function as a
parameter, it will only be modifiable if the function signature
designates it a pointer. When calling a function, a single
reference can be automatically added by the compiler, but no
more than  that. For example,

```
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

The address-of operator `@` should be avoided unless strictly
necessary.

A pointer to an object can be used just like the object itself
with respect to its members. For instance, all the following are
legal calls.

```
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

## Formatting

This section lists guidelines / rules for writing `Oak` code.
Deviation from the following is considered **non-canonical**
`Oak` formatting, and is unadvised. Deviation, in fact, could
easily be considered an error by the compiler.

Code scopes go like this:

```
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

```
// Bad

if (true) {
    // ..
}

let fn() -> void {
    // ..
}
```

Ensure you always have some sort of return statement (except
`-> void` functions), even when it is unreachable. Principles
take precedent over literal compiler interpretation.

`Oak` exclusively uses underscored variable names. Camelcase
variable names should never be used, and capitalized variables
are reserved for special cases.

```
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

```
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
```

Also, all files should always end with a newline.

Comment blocks before global definitions (those not within a
code scope) will be included in automatic manual generation via
`acorn -m`. These manuals use standard **markdown**, and by
extension all comments should as well.

Any sequence starting with `#` is ignored completely. This is to
allow for shebangs. This symbol should **never** be used for
commenting.

Tabbing should follow standard `C++` rules. Tabs and
quadruple-spaces are both acceptable, but double-spaces are not.
Not using tabbing is not acceptable.

No line of `Oak` should be longer than 64 standard-width chars
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

```
// Proper formatting:
let long_function_name(long_argument_name_one: i32,
    long_argument_name_two: i32,
    long_argument_name_three: i32,
    long_argument_name_four: i32) -> i32
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
` -> cpp_version.cpp -> executable.exe`

## Main Functions

The main function must take one of the following forms (you can
change the argument names). `argc` is the number of command line
arguments, at minimum 1. `argv` is a pointer to an array of
strings, each of which is a command line argument. The first
item of this, `argv[0]`, is the name of the executable that was
run.

```
let main() -> i32;
let main(argc: i32, argv: ^str) -> i32;
let main(argc: i32, argv: ^^i8) -> i32;
```

All macros will use this third form
(`(argc: i32, argv: ^^i8) -> i32`). Note that these two
variables can be named anything. A common form is
`(c: i32, v: ^^i8) -> i32`.

## Object Oriented Programming

`Oak` does not have classes, nor does it have internally defined
methods. Methods are converted into equivalent function calls
during translation as follows.

```
OBJ.METHOD_NAME(ARG, ARG, ..);
```

Becomes

```
METHOD_NAME(@OBJ, ARG, ARG, ..);
```

Thus, you can define a method as follows.

```
let OBJ_TYPE.METHOD_NAME(self: ^OBJ_TYPE, ..) -> RETURN_TYPE;
```

For instance, if you wanted to define a method that converts an
integer to a double, its signature would be:

```
let i32.to_double(self: ^i32) -> f64;
```

This frees the programmer from the bounds of the initial class
definition.

In `Oak`, you can define new data structures as structs, and
define any methods upon it later. For instance, a linked list
could be broadly defined as follows.

```
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

`Oak` does not have explicit header files like C / `C++`, but
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
correspondingly releases that memory. These two functions are
only legal in operator alias methods (see below) for memory
safety. This means that any data allocated on the heap must be
wrapped in a struct. This allows the "parent" variable to fall
out of scope, thereby calling its destructor and ensuring some
memory safety.

`Oak` is not very memory safe. It is more so than `C++`, but
less than `Rust`. It is the author's opinion that `Rust`'s
lifetime and ownership policies create countless issues and
programming "walls" that are only worth it in some scenarios. I
believe that programmers should hold the responsibility for
their own memory safety, rather than a compiler, and that belief
is reflected in `Oak`.

Example of `alloc!`, `free!`, and `free_arr!`:

```
let New<t>(self: ^node<t>) -> void
{
    // Legal call to alloc!

    // Allocate a single instance on the heap
    alloc!(self.data);
    free!(self.data);

    // Allocate an array of size 5 on the heap
    alloc!(self.data, 5);

    // Free a dynamically allocated array
    free_arr!(self.data);
}

let Del<t>(self: ^node<t>) -> void
{
    free!(self.data);
}

let main() -> i32
{
    // ILLEGAL CALL to alloc!
    
    let i: ^bool;
    alloc!(i);
}
```

## Interchangeability With C++

The Acorn compiler will have some automated `C++`-to-`Oak`
translation capabilities. For instance, using a `include!` macro
statement on a `C`-based file will translate the function
signatures within into `Oak` notation and add them to the symbol
table. This allows the merging of the two languages to take
place later, with object files. Since `Oak` is translated into
`C++`, this is exceedingly simple. You can also define only the
function signatures in `Oak` and define them in `C++`, as is
done in the `Oak` `std` (standard) package. This is called
**interfacing**. These pairs of dual-language files are
**interfacial files**, and any package primarily porting one
language's features to `Oak` is an **interfacial package**.

## Operator Overloading / Aliases

Where in `C++` you would write

```
class example
{
public:
    void operator=(..);
    bool operator==(..);
    // .. etc
};
```

In `Oak` you would write

```
// The unit struct; No members
let example: struct
{
    ,
}

let Copy(self: ^example, ..) -> void;
let Eq(self: ^example, ..) -> bool;
```

There are many so-called "operator aliases" which are listed
below. If `self` is not a pointer, it is a const function.

`Oak`  | `C++`     | Description            | Signature for `T`
-------|-----------|------------------------|------------------------
Less   | <         | Less than              | let Less(self: T, other: T) -> bool;
Great  | >         | Greater than           | let Great(self: T, other: T) -> bool;
Leq    | <=        | Less that or equal     | let Leq(self: T, other: T) -> bool;
Greq   | >=        | Greater than or equal  | let Greq(self: T, other: T) -> bool;
Eq     | ==        | Equal to               | let Eq(self: T, other: T) -> bool;
Neq    | !=        | Not equal to           | let Neq(self: T, other: T) -> bool;
And    | &         | Bitwise and            | let And(self: T, other: T) -> T;
Andd   | &&        | Boolean and            | let Andd(self: T, other: T) -> bool;
Or     | \|        | Bitwise or             | let Or(self: T, other: T) -> T;
Orr    | \|\|      | Boolean or             | let Orr(self: T, other: T) -> bool;
Not    | !         | Boolean negation       | let Not(self: T) -> T;
Copy   | =         | Copy constructor       | let Copy(self: ^T, other: T) -> void;
Del    | ~         | Deletion               | let Del(self: ^T) -> void;
Add    | +         | Addition               | let Add(self: T, other: T) -> T;
Sub    | -         | Subtraction            | let Sub(self: T, other: T) -> T;
Mult   | *         | Multiplication         | let Mult(self: T, other: T) -> T;
Div    | /         | Division               | let Div(self: T, other: T) -> T;
Mod    | %         | Modulo / remainder     | let Mod(self: T, other: T) -> T;
AddEq  | +=        | Increment by a number  | let AddEq(self: ^T, other: T) -> T;
SubEq  | -=        | Decrement by a number  | let SubEq(self: ^T, other: T) -> T;
MultEq | *=        | Multiply and assign    | let MultEq(self: ^T, other: T) -> T;
DivEq  | /=        | Divide and assign      | let DivEq(self: ^T, other: T) -> T;
ModEq  | %=        | Modulo and assign      | let ModEq(self: ^T, other: T) -> T;
AndEq  | &=        | Bitwise AND and assign | let AndEq(self: ^T, other: T) -> T;
OrEq   | \|=       | Bitwise OR and assign  | let OrEq(self: ^T, other: T) -> T;
Lbs    | <<        | Left bitshift          | let Lbs(self: T, other: T) -> T;
Rbs    | >>        | Right bitshift         | let Rbs(self: T, other: T) -> T;
New    | TYPE_NAME | Instantiation          | let New(self: ^T) -> ^T;

There is no `Oak` equivalent to `C++`'s prefix increment or
decrement operator.

It is notable that there is not a set return type for many of
these. It is common to see `copy` return `T`, `^T`, or `void`.

The order of operations for the above operators is as follows
(with the top of the list being evaluated first and the bottom
last)

Level | Group Name | Member operators
------|------------|--------------------
0     | Misc       | (), ^, @
1     | Assignment | =, +=, -=, *=, /=, %=, &=, \|=
2     | Booleans   | &&, \||
3     | Comparison | ==, !=, <, >, <=, >=
4     | Mult / Div | *, /, %
5     | Add / Sub  | +, -
6     | Bitwise    | &, \|, ^, <<, >>

With a few exceptions, operator alias replacement occurs within
parenthesis (either as standalone blocks for evaluation or as
part of a function call). However, in cases like assignment
parenthesis are inferred.

`Oak` does not have `C++` streams by default, although its
flexible syntax means that they aren't too hard to implement.

## Demo

File: `main.oak`

```
// Import the `Oak` standard package
package!("std");

// Use the standard `Oak` rule set
use_rule!("std");

let main(argc: i32, argv: ^str)
{
    print("This program was started with the command: ");
    println(argv[0]);

    println("Hello, world!");

    0
}

```

Commands (bash):

```
acorn main.oak -o hello.out
./hello.out
```

Output:

```
<Compiler output here>
This program was started with the command: .oak_build/a.out
Hello, world!
<Process finished with exit code 0>
```

## Atomic Conversions (If You Don't Know Rust)

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
char *              | str
bool                | bool
T&                  | ^t
T[]                 | ^t
T*                  | ^t
template\<class T\> | \<t\>

The last few entries show two things; That `Oak` does not have
references or arrays (both are replaced with pointers), and that
`Oak` has smart templating. Additionally note that there are no
multi-word types (IE unsigned long long int) in `Oak`, and that
pointers are moved to before the type they point to.

For instance,

```
template <class T, class F>
unsigned long long int **doThing(T &a, T b[]);
```

Becomes

```
let do_thing<t, f>(a: ^t, b: ^f) -> ^^u128;
```

## Generics

You can declare a generic function (a function which can operate
on a generic type, rather than a specific, defined type) by
using the `<..>` notation as below.

```
let generic_fn_demo<t>(arg1: t, arg2: bool, arg3: *t) -> t;
```

This allows a generic type `t` (you can use whatever name you
want) to enter the function's scope temporarily. On a compiler
level, generic functions do not exist until they are called.
Upon compiler detection of a call, it is ensured that an
appropriately-typed function exists (otherwise, such a function
is instantiated).

`Oak` does not have automatic instantiation of generic functions
via argument type analysis.

```
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

    gen_fn(123);

    0
}
```

Similarly, you can define **generic structs** as follows.

```
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
document for more about instantiating generic structs and the
`needs` block.

## Acorn

Acorn is the `Oak` translator, compiler, and linker. `Oak` is
first translated into `C++`, which is then compiled and linked.

Acorn command line arguments:

Name | Verbose     | Function
-----|-------------|----------------------
 -c  | --compile   | Produce object files
 -d  | --debug     | Toggle debug mode
 -D  | --dialect   | Use a dialect file
 -e  | --clean     | Work from scratch
 -g  | --exe_debug | Uses LLVM -g flag
 -h  | --help      | Show help (this)
 -i  | --install   | Install a package
 -l  | --link      | Produce executables
 -m  | --manual    | Generate auto-documentation
 -n  | --no_save   | Produce nothing
 -o  | --output    | Set the output file
 -O  | --optimize  | Uses LLVM -O3 flag
 -p  | --pretty    | Prettify `C++` files
 -q  | --quit      | Quit immediately
 -r  | --reinstall | Reinstall a package
 -R  | --remove    | Uninstalls a package
 -s  | --size      | Show `Oak` disk usage
 -S  | --install   | Install a package
 -t  | --translate | Produce `C++` files
 -u  | --dump      | Produce dump files
 -v  | --version   | Show Acorn version
 -w  | --new       | Make a new package

## Optimization and Runtime Debugging

`Oak` has some limited support for compiler optimization and
runtime debugging due to its use of LLVM-IR via Clang. The
`acorn -O` and `acorn -g` commands tell Clang to use full
compile-time optimization and allow runtime debugging
respectively.

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
significantly increase the size of the output executable.

## Macros

The following are the standard macros associated with Acorn.

`include!(WHAT, WHAT, ..)` causes the translator to also use
the file(s) within. It can take `.oak` files.

`package!(WHAT, WHAT, ..)` is a more advanced version of the
above. See the packages section below for more details.

`link!(WHAT, WHAT, ..)` causes the compiler to also use the
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

```
let hi!(argc: i32, argv: ^^i8) -> i32
{
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

```
let print_five_times!(argc: i32, argv: ^str)
{
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
have known URLs, meaning you can just say 
`acorn --install NAME`, rather than the full URL. Installed
packages keep their files in `/usr/include/oak/NAME`. Note: You
can use `--install` or `-S`.

### Creating Packages

Every package must have a file named `oak_package_info.txt`.
This file takes the following form.

```
NAME = "name_goes_here"
VERSION = "0.0.1"
LICENSE = "license"
DATE = "July 14th, 2023"
SOURCE = "example.com/example_package"
AUTHOR = "Jordan Dehmel, Joe Shmoe"
EMAIL = "jdehmel@outlook.com, example@example.com"
ABOUT = "A demo of Oak package creation"
INCLUDE = "main_file_to_link_to.oak,another_file.oak"
SYS_DEPS = "package1 package2"
```

`NAME` is the name of the package. `VERSION` is the version
number of the package (For instance, major release 1, minor
release 15, path 12 would be `1.15.12`). `DATE` is the date this
version was last updated. `SOURCE` is the URL where the
package's Git repo can be found. `AUTHOR` is the string which
contains all the package author names, separated by commas.
`EMAIL` is the same, but for emails. `ABOUT` is the package
description.

`INCLUDE` is the most important field- It contains the filepaths
(relative to the main folder of the repo, IE `./`) of all files
which will be directly `include!()`-ed by Acorn at compile-time.

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

```
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
contents!        | str  | The contents of the current `Oak` file
sys!             | str  | The operating system compiled for

Additional preprocessor definitions can be defined as follows.

```
let name! = definition;
```

Following the above definition, all occurrences of `name!` will
be replaced by `definition`.

## Preproc Rules and Sapling

### Outline

Use with caution. Rules alter the fundamental syntax of the
language.

Rules are an experimental feature that allow you to add more
functionality to `Oak`. They take in a pattern of symbols, and
replace it with a different one when it is detected in the code.
A pattern definition is mostly like a fragment of `Oak` code,
save for a few special features.

The default symbol type in a rule is a literal. These match only
the symbol that they are. For instance, `hi` would only match
symbols that say `hi`. It would not match a symbol that said
`hi-llo`, even though the pattern is a prefix of the latter.
This is because rules operate on symbol-wise pattern matching,
not character-wise pattern matching.

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

```
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

```
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

```
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

```
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

```
new_rule!(
    "for",
    "for ( $*a ; $*b ; $*c ) $~ $<${$}$> $>g",
    "$a ; while ( $b ) { $g $c ; }"
};

```

This version can recognize infinitely nested sets of curly
brackets with no issues.

Note: The `Sapling` interpreter is somewhere between a
deterministic finite autonoma and a push-down autonoma.

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
processing a file from `Oak` to `C++`. Each of these represents
(at minimum) one full pass over the entire file. In stages after
stage 3, they represent symbol-wise iteration, but before then
they represent character-wise iteration.

1 - Syntax checking
2 - Text cleaning for `contents!` pre-proc definition
3 - Lexicographical symbol parsing (lexing)
4 - Macro definition scanning
5 - Preprocessor definition insertion
6 - Compiler macros (file inclusions, linker commands, etc)
7 - Rule parsing and substitution (handles rule macros)
8 - Regular macro call handling
9 - Parenthesis and operator substitution
10 - Sequencing
11 - (Optional) `C++` file prettification
12 - (External) Object file creation via `clang++`
13 - (External) Executable linking via `clang++`

## Special Symbols

Special symbols are inserted during lexing to retain metadata.
They are essentially ways for the lexer to talk to the compiler.
They are ignored by nearly every other stage of translation.
This is, for instance, how `acorn` knows which line an error
occurs on even though the `Oak` lexer erases newlines extremely
early in processing. Special symbols are prefixed with two
slashes, like a comment. The `line` special symbol looks like
this: `//__LINE__=X`, with `X` being replaced by the current
line.

As a front-end user of `acorn`, you shouldn't have to worry
about special symbols. They will appear in some parts of
`acorn`-generated dump files, but otherwise they should not
appear anywhere (even `Sapling` ignores them).

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
`math_pi` | Returns `pi` (3.14159..)              |
`math_e`  | Returns `e` (2.71828..)               |
`fact`    | Returns the factorial of a number      |
`sin`     | Returns `sin` of a value (radians)     | Uses a 9th order Taylor polynomial
`cos`     | Returns `cos` of a value (radians)     | Uses `sin`
`tan`     | Returns `tan` of a value (radians)     | Uses `sin` and `cos`
`floor`   | Rounds a number down                   |
`ceiling` | Rounds a number up                     |
`round`   | Rounds a number                        |
`sqrt`    | Returns the square root of a number    | Uses Newtonian iteration
`f_mod`   | Returns `a` modulo `b` (floats)        |

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
`C++ SDL2` library. This allows some graphics to be created by
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

For instance, the command `acorn oak_demos/macro_test.oak` would
begin with a translation unit containing only
`oak_demos/macro_test.oak`. If that test were to open with the
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

It is sometimes required for the `C++` version of a struct to
have more members than its `Oak` counterpart. For instance, the
interface for the `sdl` package defines the `sdl_window` struct.
On the `C++` side of the interface, this struct contains a
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
struct. This space holds the extra members on the `C++` side.

If a program were to modify the padding of a padded struct, the
other side of the interface would have its data destroyed and
made nonsensical.

```
// sdl/sdl_interface.cpp

struct sdl_window
{
    u64 width, height;

    SDL_Window *wind;
    SDL_Renderer *rend;
};

```

```
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
files. `std/files.oak` is an interface to the `C++` `fstream`
library.

`i_file`s and `o_file`s do **not** open a file when they are
created! You must call the `open` method before they can be used
to read or write.

Here is a useful demonstration of both structs.

```
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

```
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

```
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

```
let example: enum
{
    option_a: i32,
    option_b: unit,
    option_c: preexisting_struct_name,
}
```

Note that an `Oak` enum looks exactly like an `Oak` struct.
Enums are handled through use of the `match` statement as below.

```
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
`C++` interface.

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

```
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

## Needs- Instantiation Blocks and Generic Data Structures

As should be obvious to anyone who has worked with a templated
data structure library, it is necessary for structures to
generically have not only members, but methods. However, `Oak`
as discussed thus far has no way to instantiate a group of
generic methods when a generic data structure is instantiated.
This is where the instantiation block, or **'needs' block**
comes in. This utilizes another of the few keywords in
`Oak`- `needs`. Consider the following source code.

```
1  | let list<t>: struct
2  | {
3  |     data: t,
4  |     next: ^list<t>,
5  | }
6  | 
7  | let New<t>(self: ^list<t>) -> void;
8  | let Del<t>(self: ^list<t>) -> void;
9  | 
10 | let main() -> i32
11 | {
12 |     New<i32>;
13 |     Del<i32>;
14 | 
15 |     let node: list<i32>;
16 | 
17 |     0
18 | }
19 | 
```

`Oak` as discussed so far necessitates lines 7 and 8. These tell
the compiler to instantiate the generic functions `New` and
`Del` for use with `t = i32`. However, this is obviously
cumbersome to an end user. Thus, we introduce the `needs` block.

```
1  | let list<t>: struct
2  | {
3  |     data: t,
4  |     next: ^list<t>,
5  | }
6  | needs
7  | {
8  |     New<t>(self: ^list<t>);
9  |     Del<t>(_: ^list<t>);
10 | }
11 |
12 | let New<t>(self: ^list<t>) -> void;
13 | let Del<t>(self: ^list<t>) -> void;
14 |
15 | let main() -> i32
16 | {
17 |     let node: list<i32>;
18 |     0
19 | }
20 |
```

The above code tells the compiler to instantiate `New` and `Del`
with the `t` generic anytime the `node<t>` template is
instantiated. Thus, these are automatically instantiated by the
compiler in line 17. Also notice that the argument name in line
9 doesn't matter, so long as the rest of the instantiator call
matches the candidate.

This system is in response to the fact that, in `Oak`, a struct
does not inherently "come" with any methods, and thus the exact
methods to instantiate with a generic struct are unknown. This
allows the programmer to tell the compiler exactly what is
needed for use with the struct.

## Explicit, Implicit / Casual, and Autogen Function Definitions

It is often useful to separate the declaration of a function
from its implementation (see the earlier section on division of
labor). However, it is necessary for any function to have
exactly one implementation to avoid undefined behavior. When
creating structs, a definition for `New` and `Del` are always
required. However, it is not always practical to write these
functions explicitly each time, especially if they are to be
unit (empty) functions. For these reasons, we introduce the
concepts of
**explicit, implicit, and autogen function definitions.**

```
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
symbol.

## Naming Conventions for Library Files

File names, especially in libraries, and **especially** in the
`Oak` `std` library, should be clear and concise. Names should
be singular, even if the file describes a class or data 
structure (IE `std/strings.oak` should become `std/string.oak`). 
Files linking `Oak` to another language should have a 
pre-file-extension suffix of `_inter` and be as physically close 
as possible to their partner file as possible (IE `std/time.oak` 
and `std/time/sub/time.cpp` should become `std/time_inter.oak` 
and `std/time_inter.cpp`). These interfacial partner files 
should be named as close to the `Oak` naming scheme as is 
possible for their language, and their compiled object files 
**must** adhere to the `Oak` naming scheme. File names may be 
shortened to a reasonable degree, so long as they do not 
conflict with any other package and their full name can be 
reasonably extracted from the abbreviation (for instance, 
`input_output.oak` to `io.oak` is a valid abbreviation, while 
`regex.oak` to `re.oak` is not). The same can be said about 
struct and enum names. If there is an addendum required to the 
name, it should go after the main name (for instance, the `std`
file `std/conv_extra.oak`).

Files should provide nothing more than they have to. If you have 
the option to split something (especially interfaces) off into 
its own file, **you generally should do so.** For example, 
`std/unit.oak` is one line of code long (`let unit: struct;`) 
because it does not **need** to have anything more. `Oak` code 
and, by extension, `.oak` files should be minimalist.

## List of Keywords

The following are keywords- That is to say, these words cannot 
be the names of variables, structs, enums, or functions. `Oak` 
aims to have as few keywords as possible, keeping with its theme 
of minimalism.

- let
- if
- else
- while
- match
- case
- default
- needs

## List of Atomic Types

The following are atomic (built-in, indivisible) types.

- struct
- enum
- integers - u8, i8, u16, i16, u32, i32, u64, i64, u128, i128
- floats - f32, f64, f128
- str
- bool
- void

The `unit` (memberless 1 byte) struct is provided by 
`std/unit.oak`.

## `c_print!` and `c_panic!`

`c_print!` prints a message at compile time, concatenating the 
passed strings. `c_panic!` does the same thing, but throws the 
result as a compile error. In either case, the message will be 
prefaced by its originating file and line number. Since these 
two macros are compile-time only, any variables passed will not 
be evaluated; Only their names will be printed. The `c` refers 
to compilation, not to the `C` programming language.

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
- free_arr!
- c_print!
- c_panic!

## The Erase Macro

`Oak` does not have private members, but sometimes it is still 
useful to disallow the use of a type after its declaration. In 
this case, you can use the `erase!` macro. This takes the name 
of a struct, and disallows it from being used again. For 
instance, the `std/interface.oak` file provides erased structs 
of various sizes for use in struct interfaces. These items can 
never be accessed, but still take up space within a struct.

## Misc. Notes

Some miscellaneous notes which are not long enough to warrant 
their own section in this document:

- `Oak` does not have namespaces under the canonical dialect, 
    although the `namespace` or `std` rules provide a simulacrum
    of them.
- It is highly likely that `Oak` will one day translate to
    `LLVM IR` instead of `C++`
- `Oak` does not have stack-stored arrays; Only heap-stored
    ones. In fact, all heap-stored data is technically
    array-based.
- As of version `0.0.10`, the `New` and `Del` operator aliases
    are single-argument only. Just use `Copy` for anything else.
- You can call a multi-argument `=` operator (`Copy`) like this:
    `a = (b, c, d);` (as of `Oak` `0.0.10`)

## Disclaimer

The `Oak` programming language outlined here bears no relation
nor resemblance to the Java prototype of the same name; I was
not aware of it until significantly into development.

## License

`Oak` is protected by the GPLv3.
