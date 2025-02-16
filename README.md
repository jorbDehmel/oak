
# The `Oak` Programming Language and `acorn` Compiler
J Dehmel, Colorado Mesa University

## Overview

The `Oak` programming language. `Oak` translates to `C`, and
provides modern generic and mangling systems. The base language
is a purely functional version of `C` with overloadable
operators and functions. However, extant "dialects" of `Oak`
theoretically encompass all recognizable languages. This is
because `Oak` has **compile-time modifiable syntax**. It allows
the user to provide "rules" (unrestricted grammars) to rewrite
the input file, targeting a centralized fixed point language
(usually called "canonical `Oak`" or "`Oak` normal form`"). The
preprocessor rule system is Turing-complete (pf excluded),
implying it can bring any language to `Oak` normal form.

Not ONF:
```rust
let main() -> i32 {
    let a = 5i32;
    a += 5i32 * 2i32 + 4i32;
    return 0i32;
}
```

ONF:
```rust
// Note: The only whitespace which is syntactically necessary is
// "let "
let main() -> i32 {
    let a: i32;
    Copy(@a, 5i32);
    AddEq(@a, Add(Mult(5i32, 2i32), 4i32));
    0i32
}
```

## Installation

To check your system for requirements:

```sh
make check
```

To install (checking beforehand):

```sh
sudo make install
```

## Testing

To run all unit and integration tests:

```sh
make test
```
