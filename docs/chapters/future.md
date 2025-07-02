
# Future

## Rules, Revision 3

```rust
let rules::main(tokens: ^TokenList) -> i32 {
  return 0i32;
}

rules::add!(
  "name"
  rules::main,
  "dep1", "dep2", "dep3"
);

rules::use!("name");

erase!("name");
```
