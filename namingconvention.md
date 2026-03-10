lowerCamelCase for variables, functions
_lowerCamelCase with leading underscore for parameters
SCREAMING_SNAKE_CASE for constants (macros)

top of .c file: library includes, then custom header includes, then static declarations, then file-specific structs, then functions.

when you expand a function call across multiple lines, add a new line and indentation for every single nested function call in the current scope

liens of code commented out in debug have no space after the comment symbol

reorder functions

for pointer types, the * is directly next to the type pointed to, meaning "char* a" instead of "char *a".