
# The `Oak` Programming Language and `acorn` Compiler

J Dehmel

![The `Oak` logo: A pixelated tree](logo.png)

## Manual

[](docs/manual.md)

## Overview

The `Oak` programming language. `Oak` translates to `C`, and
provides modern generic and mangling systems. The base language
is a purely functional version of `C` with overloadable
operators and functions. However, extant "dialects" of `Oak`
theoretically encompass all recognizable languages. This is
because `Oak` has **compile-time modifiable syntax**. It allows
the user to provide "rules" (unrestricted grammars) to rewrite
the input file, targeting a centralized fixed point language
(usually called "canonical `Oak`"). The preprocessor rule system
is Turing-complete (proof excluded), implying it can bring any
language to canonical `Oak`.

Not canonical `Oak`:
```rust
let main() -> i32 {
    let a = 5i32;
    a += 5i32 * 2i32 + 4i32;
    return 0i32;
}
```

Canonical `Oak`:
```rust
// Note: The only whitespace which is syntactically necessary is
// "let "
let main() -> i32 {
    let a: i32;
    Copy(a, 5i32);
    AddEq(a, Add(Mult(5i32, 2i32), 4i32));
    return 0i32;
}
```

The aforementioned unrestricted grammatical changes take the
form of **preprocessor rules.**

```rust
// Replace any instances of `a.b(` with `b(a,`, where a and b
// are any symbols. This rule is named "fizz", and is off by
// default.
new_rule!("fizz", "$a . $b (", "$b ( $a ,");

// Enable the "fizz" rule
use_rule!("fizz");

let add(a: i32, b: i32) -> i32 {
    return a + b;
}

let main() -> i32 {
    let c, d: i32;
    Copy(c, 123i32);
    Copy(d, 321i32);

    // This would not normally be allowed, but the rule causes
    // it to be ok!
    c.add(d);

    // It is shorthand for:
    add(c, d);

    return 0i32;
}
```

Infinite compile-time reflection, self-modification

## Installation

To launch into a development container in the current directory:

```sh
make docker
# Or
make podman
```

Note that this does not set up Oak, it just launches a
container. To check your system for requirements:

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
