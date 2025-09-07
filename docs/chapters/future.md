
# Future

## Rules, Revision 3

```rust
let rules::main!(tokens: ^TokenList) -> i32 {
  return 0i32;
}

rules::add!(
  "name"
  rules::main!,
  "dep1", "dep2", "dep3"
);

rules::use!("name");

erase!("name");
```

## Generic Scopes

This is easier for bundling purposes: No pre/post blocks are
needed.

```rust
let s<T> {
  // Stuff failing here will just eliminate a candidate
  is_type!(T);
  is!(T, i32);

  // Provides with prefix s<T>
  let Foo: struct {
    guts: T,
  }
}

let main() -> i32 {
  // s_GEN_i32_ENDGEN_Foo
  let inst: s<i32>::Foo;

  // stl_list_GEN_i32_ENDGEN_List
  let l: stl::list<i32>::List;

  // stl_map_GEN_i32_JOIN_str_ENDGEN_Map
  let m: stl::map<i32, str>::Map;

  // stl_map_GEN_stl_list_GEN_i32_ENDGEN_List_JOIN_u8_ENDGEN_Map
  let ml: stl::map<stl::list<i32>::List, u8>::Map;

  return 0i32;
}
```

## Typed Type Systems

`const`, `noexcept`, taint analysis, constraints,
and traditional type systems are all instances
of a wider Turing-complete type-typing system (static
satisfiability analysis) which can be extended into an infinite
hierarchy. Recursive type math should be accessible within an
improper language: There should be a way to describe the rules
by which arbitrary flags are propagated and type-restricted.

## Better Internal Token Stream Operations

Imposing substitution patterns upon the token stream would be
more maintainable then whatever we have going on right now.

## Universal Programming Language / UPL

A kernel-style programming language which can add, remove, and
modify its own analysis/compilation/interpretation abilities
based on the source code. "Modules" would be constructed and
loaded via the existing language, but would support additional
functionality. Given sufficient structure, it would be a native
and particularly extensible way to construct such a system. The
"unmodified language" would just be a module loader with an
interface, and the variable manager, package manager, lexer,
parser, rule system, preprocessor, templating, etc could be
loaded upon it. Basically a large interpreted language where
statements are syntactic only unless needed as semantic, with
the end goal of compiling a program / writing an output.

```rust
let bvm = module::new(
  {
    include("upl/types.upl");
    include("stl/map.upl");
    let upl::vars::inst(
        name: upl::String,
        type: upl::Type) -> void {
      // ...
    }
    let upl::vars::del(name: upl::String) -> void {
      // ...
    }
  }
);

if (module::loaded("varman")) {
  module::remove("varman");
}
module::add("better_varman", bvm);
```
