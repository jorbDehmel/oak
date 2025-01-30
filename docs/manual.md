
# The Oak Programming Language, v2
J Dehmel, MIT License

# Part 1: User Manual

## `Hello, World!`

# Part 2: Maintainer Manual

## Acorn Structure

- Parse command-line arguments
- If in test mode:
    - Collect locations to test
    - Compile all tests
    - Run tests if requested
- Else if in compile mode:
    - Load dialect file
    - Parse translation unit
        - Load entry point
        - Do syntax check
        - Recursively parse all included files
    - Translate if desired
    - Compile if desired
    - Link if desired
    - Execute if desired
