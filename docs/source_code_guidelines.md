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
- Lower camelcase variable names
- Upper camelcase class / struct names
- No `#define`s for global variables - use `const static`
- As few `#define` function-style macros as possible
- As few `#include`s as possible
- Try to keep a linear `#include` flow- Not a tangled web
- Descriptive, itemized commenting
- Keep a 64-char width in all documentation and commenting.
- If this guide does not specify something, stick to the
  precedent set by the existing codebase.

Note: These guidelines apply only to the `C++` codebase making
up `acorn`. Any future bootstrappings of `acorn`, as well as any
other `Oak` code in the source codebase, must follow the native
`Oak` guidelines instead of these.

# Is My Modification a Patch, Minor Version, or Major Version?

Reminder: `Oak` versions look like this: `major.minor.patch`.

## Does the existing `std` test suite still work?

Yes, and I made only small source code changes: Patch

Yes, and I made large source code changes: Minor version

No, but only minor changes are required: Minor version

No, major changes are required and the major version is `0`:
Minor release

No, major changes are required and the major version is not `0`:
Major release
