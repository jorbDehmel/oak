
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
let s<T>: template {
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
