# Conventions
This document details the conventions I followed while coding. They are details on implementation and therefore probably not relevant to know when a compiled version.

## Directory structure
The `src/` directory is split into `src/implementations` (the actual code) as well as `src/headers` (consisting of interfaces and constants). Any file `src/implementations/component.c` besides the `main.c` file corresponds to a file `src/headers/interface/component.h`, which contains declarations and comments on what the component does.

## File Structure
### Files ending in `.c`
The parts of these files are ordered like:
- standard library inclusions;
- custom header inclusions;
- static declarations;
- file-specific structs;
- function definitions.

### Files ending in `.h`
The parts of these files are ordered like:
- a comment explaining the purpose;
- macro declarations;
- struct declarations.

## Naming Conventions
The naming follows the following scheme:
- lowerCamelCase for variables, functions
- _lowerCamelCase with leading underscore for function parameters
- SCREAMING_SNAKE_CASE for constants (macros)

## Formatting
The general formatting of the code also has these specific conventions of mine:
- Every method is declared somewhere before it is defined.
- When you expand a function call or declaration across multiple lines, all arguments of the expanded call get an own line.
- Comments that have a space after the comment symbol are descriptive comments. Comments that have no space after the comment symbol are commented out code.
- For pointer types, the * is directly next to the type pointed to; meaning you write `char* a` instead of `char *a`.

## Further Details
- The choice of integer types is based on their minimum width according to the C standard - in other words, this code was written assuming every type only has its minimum range.
- `sizeof(char)` always evaluates to 1 according to the C standard - still, the expression `sizeof(char)` is included verbatim in the respective allocations to make the code readable and intuitive.
- Fields marked as `const` may still be freed; for that, they are casted to a non-`const` type. Casting a `const` type to a non-`const` type is however not done anywhere else.
- Strings are usually not null-terminated if their length is passed with them.