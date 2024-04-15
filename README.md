
# The Oak Programming Language
## Summary README: See `docs/manual.md` for full documentation.

![Test Badge](https://github.com/jorbDehmel/oak/actions/workflows/ci-test.yml/badge.svg)

![](docs/logo_trimmed.png)

Jordan Dehmel, jdehmel@outlook.com \
github.com/jorbDehmel/oak

## Overview

`Oak` is a modern low-level functional programming language with
compile-time modifiable syntax. Its usecase is in low-level
language and compiler design (see the later section on
dialects). `Oak` also aims to have an integrated build system,
necessitating only one call to the compiler per executable.

This `git` repository contains the source code for the `acorn`
`Oak` compiler, as well as some standard packages (`extra`,
`std`, `stl`, and `turtle`) and tests suites.

`Oak` is most closely related to `C`, but also has strong
macro support, modern typing, generics, compile-time syntax
modification and integrated package management. An interesting
consequence of `Oak`'s compile-time modifiable syntax is that
**`Oak` can be a high or low level language**, depending on the
rules / dialect applied.

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

## Compilation, Installation, and Uninstallation

**`Oak` is not compatible with Windows.** `Oak` assumes a UNIX
system.

To install, open a terminal in this folder and run
`make install`. This will compile and install `Oak`, as well as
the standard `Oak` package. To uninstall, call `acorn -A`. To
update, call `acorn -a`.

To install *all* the default `Oak` packages, run `make packages`
in this folder. If this is not run, only the `std` package will
be installed. More information about the included packages can
be found in `docs/manual.md`.

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

More help can be found in the full `Oak` manual, which can be
found at `./docs/manual.md`. A suite of examples can be found in
`./src/tests`. Please report any bugs on and forward any
specific questions to `github.com/jorbDehmel/oak`.
