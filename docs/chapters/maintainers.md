# Part 2: Maintainer Manual

This section details the history, implementation, and
maintenance of `Oak` and `acorn`. For usage details, see part 1.

## History

`Oak`, `acorn`, and `oak2c` were originally written and licensed
under the GPLv3 in mid-2023. In late 2024, efforts to rewrite
the codebase using best practices and under the MIT license
began. This document is associated with this "second version" of
`Oak`, and details its implementation after the rewrite.
Original documentation can be found in legacy versions of the
codebase.

## Parser classification

I* believe that the `Oak` parser is best classified as LALR,
since it uses finite-token lookahead and reads left-to-right in
a single pass with no backtracking. It is implemented without
the use of a parser generator, and thus I am not confident in
that classification.

*'I' being J Dehmel

## The Mangler / `oak2c`

One goal of `Oak` is to be easily interfaceable with the `C`
family. This, combined with type-distinguished function calls,
necessitates a function name mangler. For convenience, `Oak` has
a consistent, non-implementation-specific, mangler called
`oak2c`. This takes in (possibly name-ambiguous but
type-unambiguous) `Oak` function signatures and outputs
name-unambiguous human-readable `C` function signatures. This is
in opposition to the `C++` and `rust` manglers, which are at
best implementation-specific and non-human-readable.

The rules of `oak2c` work on the function signature as a token
stream. For instance, take the following `Oak` signatures, where
the mangled `C` signature follows as a comment.

```rust
let a() -> void;
// void a_FN_MAPS_void();

let b(_: ^i8, __: [16][]bool) -> i32;
// i32 b_FN_PTR_i8_JOIN_ARR_ARR_bool_MAPS_i32(i8 *, bool *[16]);

let c::d(_: ^(_: i32, _: [][]i8) -> i32) -> void;
// void c_d_FN_PTR_FN_i32_JOIN_ARR_ARR_i8_MAPS_i32_MAPS_void(
//     i32 (*)(i32, i8 **));
```

These mangled names can be arbitrarily long, but they will
always be unambiguous. There is a special case for templated
structs used as arguments, demonstrated below.

```rust
let Node<T>: struct {
  data: T,
  next: ^Node<T>
}

let New<T>(self: ^Node<T>) -> void {
  // ...
}

let main() -> i32 {
  // Creates `New(self: ^Node<i32>) -> void`
  let a: Node<i32>;

  // Creates `New(self: ^Node<bool>) -> void`
  let b: Node<bool>;

  return 0i32;
}

/*
// Produces the signatures:
struct Node_GEN_i32_ENDGEN;
struct Node_GEN_bool_ENDGEN;
void New_FN_PTR_node_GEN_i32_ENDGEN_MAPS_void(Node_GEN_i32_ENDGEN *);
void New_FN_PTR_node_GEN_bool_ENDGEN_MAPS_void(Node_GEN_bool_ENDGEN *);
*/
```

The following table demonstrates the rules.

 Signature token | Mangled version
-----------------|-----------------
 `^`             | `PTR`
 `[]`            | `ARR`
 `[...]`         | `ARR`
 `(` (fn start)  | `FN`
 `,` (fn delim)  | `JOIN`
 `) ->` (fn end) | `MAPS`
 `::`            | `_` (as usual)
 (anything else) | (itself)
 `<` (template)  | `GEN`
 `>` (template)  | `ENDGEN`

## Macro Argument String Literalization

```rust
fizz!(foo);   // Passed as "foo"

fizz!("foo"); // Passed as "\"foo\""
fizz!('foo'); // Passed as "\"foo\""
fizz!(`foo`); // Passed as "\"foo\""

fizz!("\"foo\""); // Passed as "\"\\\"foo\\\"\""
fizz!("'foo'");   // Passed as "\"'foo'\""
fizz!('"foo"');   // Passed as "\"\\\"foo\\\"\""
```

This becomes tricky when creating macros, especially those which
must use both string-enclosed and non-string-enclosed values
(EG assert, printf).

```rust
printf!(
  "%: %\n \"%\"", // "\"%: %\\n \\\"%\\\"\""
  foo,            // "foo"
  fizz,           // "fizz"
  "buzz"          // "\"buzz\""
);

/*
print(foo);
print(": ");
print(fizz);
print("\n \"");
print("buzz");
print("\"");
*/
```

## Passes

This section details the passes that acorn performs when
translating code.

1. Load entry point file
2. Lex / tokenize
3. Preprocess: Repeat until no changes are made or some max
    number of passes is exceeded
    1. Macro definitions: Compile any that are functional
    2. Fetch and recurse on all `include!`-ed files
    3. Process all linkages, pragmas, flags, rule additions, new
        rules, rule removals, rule bundles, compile-time system
        commands, errors, warning
    4. Resolve inline and functional macro calls
    5. Turn math into overloadable operator calls
    6. Apply ruleset once
4. Syntax check
5. Parse into AST
6. (Optional) Translate to `C`
7. (Optional) Call `gcc` from `C` to object file
8. (Optional) Call `g++` on object file(s) to executable
9. (Optional) Execute

Note that the rule system, as iterated non-length-preserving
transduction, is Turing-complete. Thus, the question of
compilation halting is undecidable (except for the imposed max
passes limit).

## Pass 2: Lexing

The `Oak` lexer is **not** DFA-based: It is a series of nested
`if` statements. This allows for easier special cases and
modifiability, at the cost of an arguably slower runtime.

## Pass 3. Preprocess

This is the bulk of the `Oak` language.

1. Macro definitions: Compile any that are functional
2. Fetch and recurse on all `include!`-ed files
3. Process all linkages, pragmas, flags, rule additions, new
    rules, rule removals, rule bundles, compile-time system
    commands, errors, warning
4. Resolve inline and functional macro calls
5. Turn math into overloadable operator calls
6. Apply ruleset once

## Pass 5: Grammar

`Oak` is not defined via EBNF: However, it is a useful tool for
visualizing parsing. Therefore, this section attempts to detail
the **post-preprocessing** (no macros, includes, or rules)
parsing process via EBNF / `bison` notation.

```yacc
global_scope:
  global_scope_entry
| global_scope_entry global_scope
;

global_scope_entry:
  let_struct
| let_enum
| let_fn
| '\n'
;

let_struct:
  'let' ID ':' 'struct' ';'
| 'let' ID ':' 'struct' body
;

let_enum:
  'let' ID ':' 'enum' ';'
| 'let' ID ':' 'enum' body
;

body:
  '{' variable_decls '}'
| '{' '}'
;

variable_decls:
  variable_decl
| variable_decl ',' variable_decls
;

let_fn:
  'let' ID '(' variable_decls ')' '->' type ';'
| 'let' ID '(' variable_decls ')' '->' type '{' stmts '}'
| 'let' ID '(' ')' '->' type ';'
| 'let' ID '(' ')' '->' type '{' stmts '}'
;

variable_decl:
  variable_names ':' type
;

variables_names:
  ID
| ID ',' variable_names
;

type:
  ID
| '[' ']' type
| '[' INTEGER_LITERAL ']' type
| '^' type
| '^' '(' variable_decls ')' '->' type
| '^' '(' ')' '->' type
;

stmts:
  stmt
| stmt stmts
;

stmt:
  '{' stmts '}'
| 'let' variables_names ':' type ';'
| 'if' '(' object ')' stmt
| 'if' '(' object ')' stmt 'else' stmt
| 'while' '(' object ')' stmt
| fn_call ';'
| match_stmt
;

object:
  fn_call
| obj_member
;

obj_member:
  ID '.' ID
| ID '.' obj_member
;

fn_call:
  ID '(' variable_names ')'
| ID '(' ')'
;

match_stmt:
  'match' '(' object ')' '{' match_cases '}'
;

match_cases:
  match_case
| match_else
| match_case match_cases
| match_else match_cases
;

match_case:
  'case' ID '(' variable_decl ')' stmt
;

match_else:
  'else' stmt
;
```

After the tokenized source code is parsed into an AST, the tree
is reconstructed into equivalent `C` code and written to file.
Note that there are numerous exceptions to this grammar: Many
macros must be delayed until parse- or reconstruct-time to
function.

## Pass 8: Linking via `g++`

Note that `g++` (a `C++` compiler) is used for linkage instead
of `gcc` (a `C` compiler): This allows `C++` object files to be
included at link-time. `g++` is also just generally "smarter",
so it is advantageous to use it.

## Pass 9: Execution

`acorn` retains information about the translation unit as it
runs: This allows us to use the `run_should_fail` pragma. This
pragma does *not* modify the actual runtime: It only says that,
if `acorn` is the one executing, a nonzero exit code should be
expected and a zero exit code should be treated as an error.

# Part 3: Future Features

Given a block:
1. For as long as the block is unclosed
  1. Parse a statement
    1. If preprocessor statement, delegate. Else if declaration,
      delegate. Else, fix math and process normal statement.
  2. Append the processes statement, if necessary

## Internal Rule Engines

There is no reason why rule engines must be external: In the
future, macros should be able to act as custom rule engines.
This should be in the form of dynamic `.so` loading so as to
reduce latency.

```rust
let rules::EngineInfo: struct {
  max_output: uint,
}

let PreProcessor: struct {
  // ...
}

/// Initialize the engine
let New(self: ^PreProcessor) -> void;

/// Report information about the class
let RuleEngineReport(self: ^PreProcessor) -> rules::EngineInfo;

/// If returns false, write the input. Else if output_size is 0
/// (default), don't write anything. If output_size is nonzero,
/// put the first `output_size` items of `output`.
let process(self: ^PreProcessor, input: []i8, output: [][]i8,
  output_size: ^uint) -> bool;

rule::register_engine!("preprocessor", PreProcessor);

```
