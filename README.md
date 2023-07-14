# The Oak Programming Language
Jordan Dehmel, jdehmel@outlook.com, github.com/jorbDehmel/oak

## Overview
Oak is a modern, Rust-adjacent programming language. It uses
near-standard Rust typing, without any form of memory protections.
The motto of Oak is "you're the programmer, do it yourself". It
is analogous to C++ with stronger macro support, modern typing,
and integrated package management.

## Syntax

In Oak, a variable is declared as follows.

`let NAME: TYPE;`

For instance, a boolean named  `a` would be declared:

`let a: bool;`

If you want to initialize it at the same time, you would instead
do:

`let a: bool = false;`

Functions are declared as follows.

`let NAME(ARG_1: ARG_1_TYPE, ...) -> RETURN_TYPE;`

For instance, a function that in `C++` would be declared
`bool isBiggerThanFive(int a);` would in `Oak` be declared
`let is_bigger_than_five(a: i32) -> bool;`. See later for atomic
type conversions from `C` (IE how `int` became `i32` here).

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
`return` statement. Oak has no `return` keyword.

## Main Function

The main function must take one of the following forms
(you can change the argument names). `argc` is the number
of command line arguments, at minimum 1. `argv` is a pointer
to an array of strings, each of which is a command line
argument. The first item of this, `argv[0]`, is the name
of the executable that was run.

```
let main() -> i32;
let main(argc: i32, argv: *str) -> i32;
let main(argc: i32, argv: **i8) -> i32;
```

## Demo

File: `main.oak`
```
// Import the Oak standard package
package!(std);

let main(argc: i32, argv: *str)
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

C++                | Oak
-------------------|-------
unsigned char      | u8
char               | i8
unsigned short     | u16
short              | i16
unsigned int       | u32
int                | i32
unsigned long      | u64
long               | i64
unsigned long long | u128
long long          | i128
float              | f32
double             | f64
char *             | str
bool               | bool


## Acorn
Acorn is the Oak translator, compiler, and linker. Oak is first
translated into C++, which is then compiled and linked.

Acorn command line arguments:

Name | Verbose     | Function
-----|-------------|----------------------
 -h  | --help      | Show help (this)
 -v  | --version   | Show Acorn version
 -d  | --debug     | Toggle debug mode
 -o  | --output    | Set the output file
 -n  | --no_save   | Produce nothing
 -t  | --translate | Produce C++ files
 -c  | --compile   | Produce object files
 -l  | --link      | Produce executables
 -e  | --clean     | Work from scratch
 -q  | --quit      | Quit immediately
 -p  | --pretty    | Prettify C++ files
 -i  | --install   | Install a package
 -r  | --reinstall | Reinstall a package
 -s  | --size      | Show Oak disk usage

## Macros

The following are the standard macros associated with Acorn.

`include!(WHAT, WHAT, ...)` causes the translator to also use
the file(s) within. It can take `.oak`, `.cpp`, `.hpp`, `.h`
or `.c` files.

`package!(WHAT, WHAT, ...)` is a more advanced version of the
above. See the packages section below for more details.

`link!(WHAT, WHAT, ...)` causes the compiler to also use
the file(s) within. It takes `.o` files, or anything that can
be used by a standard `C` linker. You can, for example, write
a function header with no body in a `.oak` file, and add a
`link!` macro containing the function definition, perhaps
compiled in another language. This makes integration with
`C / C++` files extremely easy.

You can also define your own macros as follows. If you
wanted to create a macro named `hi!` which expands to
`print("hello world")`, you would do the following.

```
let hi!(argc: i32, argv: *str) -> i32
{
    include!(/usr/include/oak/std/std_io.oak);

    print("print(\"hello world\")");

    0
}
```

A macro is compiled separately, so if you want to use a
included file within, you will have to call its inclusion
macro again. You will note that a macro has the same type
as a main function; This is why.linked

If you wanted to create a macro named `print_five_times!()`
which prints the name of a symbol 5 types, you would do
the following.

```
let print_five_times!(argc: i32, argv: *str)
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

If you were to call `print_five_times!(a)`, it would expand
into `aaaaa`. If you were to call `print_five_times!(a[b].c)`,
you would receive `a[b].ca[b].ca[b].ca[b].ca[b].c`.

## Packages
The `package!(WHAT)` macro imports a package. If it is not yet
installed on the compiling device, it can be cloned via Git
by using `acorn --install PACKAGE_URL`.

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
```

`NAME` is the name of the package. `VERSION` is the version number
of the package (For instance, major release 1, minor release 15,
path 12 would be `1.15.12`). `DATE` is the date this version was
last updated. `SOURCE` is the URL where the package's Git repo can
be found. `AUTHOR` is the string which contains all the package
author names, separated by commas. `EMAIL` is the same, but for
emails. `ABOUT` is the package description.

`INCLUDE` is the most important field- It contains the filepaths
(relative to the main folder of the repo, IE `./`) of all files
which will be directly `include!()`-ed by Acorn at compile-time.

For instance, the `STD` (standard) library has a central linking
file named `std.oak`. This file contains `include!` macros for
all other files in the package. Thus, only `std.oak` needs to
linked at compile time- the rest will be taken care of
automatically. Thus, `std/oak_package_info.txt` has a line reading
`INCLUDE = "std.oak"`.

## License
Oak is protected by the GPLv3.
