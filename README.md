
[comment]: # ( As of July 28, 2023, the project is about 6,500 lines of code )
[comment]: # ( It takes 14 seconds to compile via time make clean install )
[comment]: # ( It takes up 928 mb of disk space upon install via acorn -sq )

# The Oak Programming Language

Jordan Dehmel, jdehmel@outlook.com, github.com/jorbDehmel/oak

----------------------------------------------------------------------

## Overview

`Oak` is a modern, compiled, low-level, statically-typed programming language. It uses `Rust`-like typing, without `Rust`'s lifetimes system. It is analogous to `C++` with stronger macro support, modern typing, compile-time syntax modification and integrated package management. It's like if `C` let you shape the language to your liking. It is named Oak because nature imagery is desperately needed in the mind of a programmer.

Oak is, as of now, a translated language; Oak code is translated via the `acorn` command (see later) into `C++`. `acorn` can also compile `Oak` into object code, or link it to create executables.

Oak has modifiable syntax (see the section on preprocessor rules), making it highly customizable and flexible in a way that no other mainstream languages are. It supports the creation of "dialects", which are `Oak` variants which use preprocessor rules to support independent syntactical structures. In this way, `Oak` provides a strong central core to many branches.

The Oak programming language outlined here bears no relation nor resemblance to the Java prototype of the same name; I was not aware of it until significantly into development.

## Names

Name      | Meaning
----------|-------------------------------------------------------
`Oak`     | The base `Oak` programming language.
`Acorn`   | The `Oak` translator / compiler.
`Sapling` | The sub-language for creating `Oak` dialects / rules.
Dialect   | A syntactically-modified branch of `Oak`.

## Compilation, Installation, and Uninstallation

To install, open a terminal in this folder and run `make install`. This repo is only compatible with Linux. This will compile and install Oak, as well as the standard Oak package. To uninstall, open this folder in terminal and run `make uninstall`. If you've already deleted this folder, you can run `sudo rm -rf /usr/include/oak /usr/bin/acorn`. Both of these will accomplish the same thing.

## Syntax

In Oak, a variable is declared as follows.

`let NAME: TYPE;`

For instance, a boolean named  `a` would be declared:

`let a: bool;`

If you want to initialize it at the same time, you would instead
do:

`let a: bool = false;`

(Note: This is translated into `let a: bool; a = false;`. All types must have a default and a copy constructor.)

Functions are declared as follows.

`let NAME(ARG_1: ARG_1_TYPE, ...) -> RETURN_TYPE;`

For instance, a function that in `C++` would be declared `bool isBiggerThanFive(int a);` would in `Oak` be declared `let is_bigger_than_five(a: i32) -> bool;`. See later for atomic type conversions from `C` (IE how `int` became `i32` here).

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

Note that leaving off a semicolon is equivalent to `C++`'s `return` statement. Oak has no `return` keyword.

Pointers are `^` (IE a pointer to a bool is `^bool`). The "get address" operator is `@`.

Syntax is fluid and user-modifiable in `Oak`. For more information about this aspect, see the `Preproc Rules` section below.

## Formatting

Code scopes go like this:

```
// Good

if (true)
{
    // ...
}

let fn() -> void
{
    // ...
}
```

NEVER like this:

```
// Bad

if (true) {
    // ...
}

let fn() -> void {
    // ...
}
```

Ensure you always have some sort of return statement, even when it is unreachable. Principles take precedent over literal compiler interpretation.

For instance, do this:

```
let fn() -> void
{
    ;

    void
}

let fn2() -> void
{
    if (true)
    {
        void
    }

    void
}
```

NEVER this:

```
let fn() -> void
{
    ;
}

let fn2() -> void
{
    if (true)
    {
        void
    }
}
```

Oak exclusively uses underscored variable names. Camelcase variable names should never be used, and capitalized variables are illegal at compile time.

```
// Good
let long_variable_name_with_multiple_words: i32;

// Bad
let longVariableNameWithMultipleWords: i32;
let longvariablenamewithmultiplewords: i32;

// Won't compile
let LongVariableNameWithMultipleWords: i32;
let Long_variable_name_with_multiple_words: i32;
```

The same naming tips go for structs. Structs should not have any special name conventions. The only symbols which have different naming conventions are the operator aliases (see later), which use upper camelcase.

These naming conventions are also extended to package names (unless for some reason incompatible with the git host website they are on) and file names. Unlike standard practice in `C++` and most other languages, generics types (see later) should still be lower-underscore-case.

Single-line comments should start with "`// `" (slash, slash, space), and multi-line comments should start with `/*` and end with `*/` (just like `C++`).

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

Comment blocks before global definitions (those not within a code scope) will be included in automatic manual generation via `acorn -m`.

Any sequence starting with `#` is ignored completely. This is to allow for shebangs. This symbol should **never** be used for commenting.

## Main Functions

The main function must take one of the following forms (you can change the argument names). `argc` is the number of command line arguments, at minimum 1. `argv` is a pointer to an array of strings, each of which is a command line argument. The first item of this, `argv[0]`, is the name of the executable that was run.

```
let main() -> i32;
let main(argc: i32, argv: ^str) -> i32;
let main(argc: i32, argv: ^^i8) -> i32;
```

## Object Oriented Programming

Oak does not have classes, nor does it have internally defined methods. Methods are converted into equivalent function calls during translation as follows.

```
OBJ.METHOD_NAME(ARG, ARG, ...);
```

Becomes

```
METHOD_NAME(&OBJ, ARG, ARG, ...);
```

Thus, you can define a method as follows.

```
let OBJ_TYPE.METHOD_NAME(self: ^OBJ_TYPE, ...) -> RETURN_TYPE;
```

For instance, if you wanted to define a method that converts an integer to a double, its signature would be:

```
let i32.to_double(self: ^i32) -> f64;
```

This frees the programmer from the bounds of the initial class definition.

In Oak, you can define new data structures as structs, and define any methods upon it later. For instance, a linked list could be broadly defined as follows.

```
// Alloc is defined in std's std_mem.oak
package!("std");

let linked_list<t>: struct
{
    data: t,
    next: ^linked_list<t>,
}

let linked_list<t>.append(self: *linked_list<t>, what: t) -> void
{
    // Alloc is equivalent to `C++`'s new keyword
    alloc!(next);
    next.data = what;
}
```

Oak does not have private members, nor does it have inheritance. Eventually, traits will be implemented as in `Rust`.

## Division Of Labor

Oak does not have explicit header files like C / `C++`, but there is no reason why you could not use a `.oak` file like a `.hpp / .h`. For example, a `.oak` can establish function signatures without explicitly defining them, allowing another `.oak` to define them. This allows easy division of labor, as in `C / C++`. This is obviously vital for any project of scale. Additionally, the translator will detect and prevent circular dependencies, eliminating any analogy to `pragma once`.

## Memory Safety and Heap Memory Allocation

The `alloc!` and `free!` functions are akin to `C++`'s `new` and `delete` keywords, respectively. Alloc requests a memory position on the heap with the size of `t`, and free correspondingly releases that memory. These two functions are only legal in operator alias methods (see below) for memory safety. This means that any data allocated on the heap must be wrapped in a struct. This allows the "parent" variable to fall out of scope, thereby calling its destructor and ensuring some memory safety.

Oak is not very memory safe. It is more so than `C++`, but less than `Rust`. It is the author's opinion that `Rust`'s lifetime and ownership policies create countless issues and programming "walls" that are only worth it in some scenarios. I believe that programmers should hold the responsibility for their own memory safety, rather than a compiler, and that belief is reflected in Oak.

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

## Interchangeability With `C++`

The Acorn compiler will have some automated `C++`-to-`Oak` translation capabilities. For instance, using a `include!` macro statement on a `C`-based file will translate the function signatures within into Oak notation and add them to the symbol table. This allows the merging of the two languages to take place later, with object files. Since `Oak` is translated into `C++`, this is exceedingly simple. You can also define only the function signatures in `Oak` and define them in `C++`, as is done in the Oak `std` (standard) package.

## Operator Overloading / Aliases

Where in `C++` you would write

```
class example
{
public:
    void operator=(...);
    bool operator==(...);
    // ... etc
};
```

In Oak you would write

```
// The unit struct; No members
let example: struct
{
    ,
}

let example.Copy(...) -> void;
let example.Eq(...) -> bool;
```

There are many so-called "operator aliases" which are listed below. If `self` is not a pointer, it is a const function.

Oak    | `C++`       | Description            | Signature for `T`
-------|-----------|------------------------|------------------------
Get    | [ ]       | Get given an index     | let Get(self: ^T, i: i128) -> SOME_TYPE;
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
Copy   | =         | Copy constructor       | let Copy(self: ^T, other: T) -> ^T;
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
Incr   | ++        | Increment              | let Incr(self: ^T) -> T;
Decr   | --        | Decrement              | let Decr(self: ^T) -> T;
AndEq  | &=        | Bitwise AND and assign | let AndEq(self: ^T, other: T) -> T;
OrEq   | \|=       | Bitwise OR and assign  | let OrEq(self: ^T, other: T) -> T;
Lbs    | <<        | Left bitshift          | let Lbs(self: T, other: T) -> T;
Rbs    | >>        | Right bitshift         | let Rbs(self: T, other: T) -> T;
New    | TYPE_NAME | Instantiation          | let New(self: ^T) -> ^T;

There is no Oak equivalent to `C++`'s prefix increment or decrement operator.

The order of operations for the above operators is as follows (with the top of the list being evaluated first and the bottom last)

Level | Group Name | Member operators
------|------------|--------------------
0     | Misc       | (), ^, @
1     | Assignment | =, +=, -=, *=, /=, %=, ++, --, &=, \|=
2     | Brackets   | [ ]
3     | Booleans   | !, &&, \||
4     | Comparison | ==, !=, <, >, <=, >=
5     | Mult / Div | *, /, %
6     | Add / Sub  | +, -
7     | Bitwise    | &, \|, ^, <<, >>

With a few exceptions, operator alias replacement occurs within parenthesis (either as standalone blocks for evaluation or as part of a function call). However, in cases like assignment parenthesis are inferred.

## Demo

File: `main.oak`

```
// Import the Oak standard package
package!(std);

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
acorn --link main.oak && .oak_build/a.out
```

Output:

```
<Compiler output here>
This program was started with the command: .oak_build/a.out
Hello, world!
<Process finished with exit code 0>
```

## Atomic Conversions (If You Don't Know Rust)

`C++`                 | Oak
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

The last few entries show two things; That Oak does not have references or arrays (both are replaced with pointers), and that Oak has smart templating. Additionally note that there are no multi-word types (IE unsigned long long int) in Oak, and that pointers are moved to before the type they point to.

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

You can declare a generic function (a function which can operate on a generic type, rather than a specific, defined type) by using the `<...>` notation as below.

```
let generic_fn_demo<t>(arg1: t, arg2: bool, arg3: *t) -> t;
```

This allows a generic type `t` (you can use whatever name you want) to enter the function's scope temporarily. On a compiler level, generic functions do not exist until they are called. Upon compiler detection of a call, it is ensured that an appropriately-typed function exists (otherwise, such a function is instantiated).

Oak does not have automatic instantiation of generic functions via argument type analysis. For instance, you must say `println<i32>(5)`, not `println(5)`. However, this is only true for the first time a generic function is called.

```
// Always valid:
{
    // Instantiate and call
    println<i32>(5);
}

// Only valid if println<i32> has already been instantiated:
{
    // Call without instantiating
    println(5);
}

// Always valid:
{
    // Instantiate without calling
    println<i32>;
}

// Equivalence
{
    println<i32>;
    println(5);

    // Is equivalent to

    println<i32>(5);
}
```

## Acorn
Acorn is the Oak translator, compiler, and linker. Oak is first translated into `C++`, which is then compiled and linked.

Acorn command line arguments:

Name | Verbose     | Function
-----|-------------|----------------------
 -c  | --compile   | Produce object files
 -d  | --debug     | Toggle debug mode
 -D  | --dialect   | Use a dialect file
 -e  | --clean     | Work from scratch
 -h  | --help      | Show help (this)
 -i  | --install   | Install a package
 -l  | --link      | Produce executables
 -m  | --manual    | Generate auto-documentation
 -n  | --no_save   | Produce nothing
 -o  | --output    | Set the output file
 -p  | --pretty    | Prettify `C++` files
 -q  | --quit      | Quit immediately
 -r  | --reinstall | Reinstall a package
 -R  | --remove    | Uninstalls a package
 -s  | --size      | Show Oak disk usage
 -S  | --install   | Install a package
 -t  | --translate | Produce `C++` files
 -u  | --dump      | Produce dump files
 -v  | --version   | Show Acorn version
 -w  | --new       | Make a new package

## Macros

The following are the standard macros associated with Acorn.

`include!(WHAT, WHAT, ...)` causes the translator to also use the file(s) within. It can take `.oak` files.

`package!(WHAT, WHAT, ...)` is a more advanced version of the above. See the packages section below for more details.

`link!(WHAT, WHAT, ...)` causes the compiler to also use the file(s) within. It takes `.o` files, or anything that can be used by a standard `C` linker. You can, for example, write a function header with no body in a `.oak` file, and add a `link!` macro containing the function definition, perhaps compiled in another language. This makes integration with `C / C++` files extremely easy.

You can also define your own macros as follows. If you wanted to create a macro named `hi!` which expands to `print("hello world")`, you would do the following.

Macro arguments are always treated as strings. For instance, `link!(foobar)` is equivalent to `link!("foobar")`. You are encouraged to put quotations around any filenames.

```
let hi!(argc: i32, argv: ^str) -> i32
{
    include!("/usr/include/oak/std/std_io.oak");

    print("print(\"hello world\")");

    0
}
```

A macro is compiled separately, so if you want to use a included file within, you will have to call its inclusion macro again. You will note that a macro has the same type as a main function; This is why.

If you wanted to create a macro named `print_five_times!()` which prints the name of a symbol 5 types, you would do the following.

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

If you were to call `print_five_times!(a)`, it would expand into `aaaaa`. If you were to call `print_five_times!(a[b].c)`, you would receive `a[b].ca[b].ca[b].ca[b].ca[b].c`.

## Packages

The `package!(WHAT)` macro imports a package. If it is not yet installed on the compiling device, it can be cloned via Git by using `acorn --install PACKAGE_URL`. You can update or reinstall a package via `acorn --reinstall PACKAGE_URL`.

### Creating Packages

Every package must have a file named `oak_package_info.txt`. This file takes the following form.

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

`NAME` is the name of the package. `VERSION` is the version number of the package (For instance, major release 1, minor release 15, path 12 would be `1.15.12`). `DATE` is the date this version was last updated. `SOURCE` is the URL where the package's Git repo can be found. `AUTHOR` is the string which contains all the package author names, separated by commas. `EMAIL` is the same, but for emails. `ABOUT` is the package description.

`INCLUDE` is the most important field- It contains the filepaths (relative to the main folder of the repo, IE `./`) of all files which will be directly `include!()`-ed by Acorn at compile-time.

For instance, the `STD` (standard) library has a central linking file named `std.oak`. This file contains `include!` macros for all other files in the package. Thus, only `std.oak` needs to linked at compile time- the rest will be taken care of automatically. Thus, `std/oak_package_info.txt` has a line reading `INCLUDE = "std.oak"`.

`SYS_DEPS` is a string containing space-separated packages required to be installed by your operating system. For instance, if a package used SDL2 and Git, it would include the line `SYS_DEPS = "SDL2 git"`.

## Editing Oak

If you use VSCode, you can add this to your `.vscode/settings.json` file.

```
"files.associations": {
        "*.oak": "rust",
    }
```

`Rust` is similar enough to Oak that you won't notice any major issues, so long as you don't enable any `Rust` extensions.

## Preproc Definitions

There are several so-called preproc definitions which can be called upon. These will be replaced with their values before compilation. Each of these begin and end with two underscores.

Name            | Type | Description
----------------|------|----------------------
\_\_LINE__       | i128 | The current line
\_\_COMP_TIME__  | i128 | The UNIX time of compilation
\_\_PREV_FILE__  | str  | The path of the previous Oak file
\_\_FILE__       | str  | The path of the current Oak file
\_\_CONTENTS__   | str  | The contents of the current Oak file

## Preproc Rules and Sapling

### Outline

Use with caution. Rules alter the fundamental syntax of the language.

Rules are an experimental feature that allow you to add more functionality to `Oak`. They take in a pattern of symbols, and replace it with a different one when it is detected in the code. A pattern definition is mostly like a fragment of `Oak` code, save for a few special features.

The default symbol type in a rule is a literal. These match only the symbol that they are. For instance, `hi` would only match symbols that say `hi`. It would not match a symbol that said `hi-llo`, even though the pattern is a prefix of the latter. This is because rules operate on symbol-wise pattern matching, not character-wise pattern matching.

The second type of symbol in a rule is a variable. These take the form of a dollar sign followed by a single alphanumeric character (in regex, `\$[a-zA-Z0-9]`). Variables can only use single-character names, so there are a maximum of `62` variables per rule. Whatever single symbol is matched into a variable will be accessible in the output pattern.

The final type of symbol in a rule is a wildcard, represented by two dollar signs. A wildcard matches anything, but is not stored.

The dollar sign was chosen over the traditional "glob" `*` operator because it is not used within usual `Oak` code. The glob also represents multiplication, so it is not used. Using the dollar sign means that there are no backslashes needed in an `Oak` rule.

Output patterns do not use wildcards. Upon replacement, any occurrence of a defined variable in the output pattern will be replaced with that variable's match in the input pattern.

For instance, the input rule `meow $$ $m meow` would match `meow woof bark meow`, or indeed any other two symbols surrounded by `meow`'s. The difference comes only in the output pattern; `$s` would map to `bark`, but `woof` would never be accessible. As far as the output pattern is concerned, that `$$` could have matched anything.

Input and output patterns are lexed, so spaces may or may not be required within.

In a way, rules allow you to define whatever syntactic fluff you want. The rule-definition language outlined here is tenatively called `Sapling` (**S**ymbol **A**lteration **P**rogramming **L**anguage **I**mplemented by **N**ine **G**orillas). This may be stylized `$apling`.

### Defining and Using Rules

Rules are defined using the `new_rule!` macro. This takes three arguments; The name of the rule, the input pattern, and the output pattern. For instance, a rule named `foo_to_bar` with an input pattern `foo` and an output pattern `bar` would be declared `new_rule!("foo_to_bar", "foo", "bar");`. Note that all arguments to rule macros **must** be strings. This is advisable (but not strictly required) of all macros.

Once a rule is defined, it will be inactive. To make a rule active, you must call the `use_rule!` macro. This takes one or more rule names, and activates those rules from that point on. Note that these rules are only active *after* this macro is called.

The set of active rules does not roll over when a new file is called, but the set of all rules does. This means that a included file will have access to its "parent" files' rules, but they will always be off by default. The `use_rule!` macro will need to be called within every file which wants to use a rule.

### Removing Rules

A rule can be deactivated (but never deleted) using the `rem_rule!` macro. This takes zero or more arguments. If zero arguments are given, it disables all rules. If one or more, it disables the rules with the given names.

### Rule Bundles

Multiple rules can be bundled together under one name by using the `bundle_rule!` macro. This takes two or more arguments, with the first argument being the name of the bundle. The remaining arguments are the names of the rules which fall within this bundle. This allows you to use a single `use_rule!` call to activate a hole suite of rules.

Bundles cannot be disabled all at once; You will need to call `rem_rule!` for each of their component rules. A bundle with a given name will always overwrite a rule with the same name, except in a `bundle_rule!` call. A rule can never overwrite a macro call, but can overwrite anything else. Rules cannot find special / comment symbols (like the secret `//__LINE__=` symbol).

### Example

```
// File "source.oak"

package!("std");

new_rule!("everything_to_meow", "$$", "meow");
new_rule!("remove_meow", "meow", "");

bundle_rule!("remove_everything", "everything_to_meow", "remove_meow");

use_rule!("remove_everything");

these symbols will be destroyed by the meow menace

```

Any file which calls `include!("source.oak");` would have access to the `remove_everything` rule, but it would always be off by default.

Here is the source code of the `std_method` bundle, which does automatic method reformatting.

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

As of version `0.0.2`, `Sapling` also has several more features. On such feature is `$+`, which is a wildcard that matches any one or more symbols. It may not be the first or last symbol in a `Sapling` sequence, as it is terminated only by the first instance of the following symbol (if it were the first item, it would be highly inefficient). Another such, with the same rules, is `$*`. This symbol matches any zero or more symbols that are not the following symbol. With these two symbols, no occurrence of the following symbol may match anywhere within. Additionally, a variable may not occur directly after either.

`hi $* ( )` would match `hi hello hello hello hello ( )`, but not `hi hello hello ( hello ) ( )`.

This update also introduces a stored glob operator `$+X` with `X` being another single-character variable name. For instance, `if $+a {` -> `if ( $a ) {` would auto-parenthesize `if` statements.

```
// STD Oak dialect rules
clear

"if $+a {" "if ( $a ) {"
"while $+a {" "while ( $a ) {"

// NOT final; We want to use STD_OD as a jumping-off point
```

It may also introduce the merge operator `$<` for output patterns. This would fuse the preceding and proceeding symbols, without any separation. For instance, `( $t )( $+f )` -> `to $< $t ( $f )` would convert standard cast notation into the `Oak` equivalent.

## Dialects

Dialect files introduce rules that are handled before all others and which are always enabled by the translator. This allows for the creation of `Oak` **dialects**, or syntactically different languages which use rules to collapse into regular `Oak` at translation-time.

Dialect files (`.od` files, for **O**ak **d**ialect) consist of a few types of line. There can be comments, marked by `// ` (slash-slash-space). A line can read `clear`, which erases all existing dialect rules. It could also read `final`, which disables any further dialect changes. Finally, a line could read `"INPUT PATTERN HERE" "OUTPUT PATTERN HERE"`, with an input and an output pattern enclosed by quotes and separated by a space.

Dialects are loaded via the `acorn -D filename.od` argument.

## Addendum I: Translator Passes

The `Oak` translator goes through the following passes when processing a file from `Oak` to `C++`. Each of these represents (at minimum) one full pass over the entire file. In stages after stage 2, they represent symbol-wise iteration, but before then they represent character-wise iteration.

1 - Text cleaning for `__CONTENTS__` pre-proc definition
2 - Lexicographical symbol parsing (lexing)
4 - Preprocessor definition insertion
5 - Compiler macros (file inclusions, linker commands, package inclusions)
6 - Rule parsing and substitution (handles rule macros)
6 - Regular macro definition scanning
7 - Regular macro call handling
8 - Parenthesis and operator substitution
9 - Sequencing
10 - (Optional) `C++` file prettification
11 - (External) Object file creation via `clang++`
12 - (External) Executable linking via `clang++`

## Addendum II: Special Symbols

Special symbols are inserted during lexing to retain metadata. They are ignored by nearly every other stage of translation. This is, for instance, how `acorn` knows which line an error occurs on even though the `Oak` lexer erases newlines extremely early in processing. Special symbols are prefixed with two slashes, like a comment. The `line` special symbol looks like this: `//__LINE__=X`, with `X` being replaced by the current line.

As a front-end user of `acorn`, you shouldn't have to worry about special symbols. They will appear in some parts of `acorn`-generated dump files, but otherwise they should not appear anywhere (even `Sapling` ignores them).

## Addendum III: Time and Space Complexities

`Oak` translation, in theory, runs in time complexity proportional to the sum number of characters in the included files; `O(n)`. This analysis assumes that this number of characters outnumbers the number of symbols, and that any further iterative process takes fewer steps that this. The space complexity is the same as the time complexity. These prepositions may prove to be false, however, warranting a more thorough analysis.

## Addendum IV: The "std_method" rule

The `std_method` rule (included in the "std" package) allows method calls like other languages. There are four cases; zero-argument const methods, zero-argument mutable methods, argumented const methods, and argumented mutable methods. Mutable methods take pointer to the object, while const ones take a copy of it. Thus, const methods cannot change the members. Argumented and zero-argument methods refer to the appearance pre-rule-application; All methods take a reference to the object they are called upon. The rules allow the following shorthands.

```
package!("std");
use_rule!("std_method");

let a: struct;

let argless_const(self: a) -> void;
let argless_mut(self: ^a) -> void;
let arg_const(self: a, i32 b, i32, c, i32 d) -> void;
let arg_mut(self: ^a, i32 b, i32 c) -> void;

let main() -> i32
{
    let test: a;

    // These two calls are equivalent:
    test.argless_const();
    argless_const(test);

    // These two calls are equivalent:
    test..argless_mut();
    argless_mut(@test);

    // These two calls are equivalent:
    test.arg_const(1, 2, 3);
    arg_const(test, 1, 2, 3);

    // These two calls are equivalent:
    test..arg_mut(1, 2);
    arg_mut(@test, 1, 2);

    0
}

```

## License

Oak is protected by the GPLv3, which must be attached in any installation media for the software.
