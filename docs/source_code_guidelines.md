# Source Code Guidelines for Editing `Oak`

"A Foolish Consistency is the Hobgoblin of Little Minds"
- Guido van Rossum

## Author(s)

Jordan Dehmel, jdehmel@outlook.com

# Rules

Code written in the `Oak` language has a suite of syntactical
rules that it must obey. These rules are enforced by the `acorn`
compiler. However, the `C++` source code which makes up `acorn`
has many fewer guidelines. These are outlined below.

- No `using` keyword in `C++` code
- Descriptive variable names
- Upper camelcase variable names
- No `#define`s for global variables - use `const static`
- Descriptive, itemized commenting

Note: These guidelines apply only to the `C++` codebase making
up `acorn`. Any future bootstrappings of `acorn`, as well as any
other `Oak` code in the source codebase, must follow the native
`Oak` guidelines instead of these.
