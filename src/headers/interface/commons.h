// this is a loosely connected group of general functions used in multiple sceniarios during development

// this can replace a normal printf, a semicolon naturally follows after: https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
#define printIfVerbose(_verbose, _text, ...) if (_verbose) printf(_text, ##__VA_ARGS__)

// tries to allocate an object of the given size. if allocation fails, it gives a message and aborts.
void* saferMalloc(int _size, char* _objectDescription);

// if the expression is false, this function terminates the program.
void assert(int _expression, char* _errorDescription);

// performs and returns fgetc, unless if fgetc returns -1 (EOF or read error); then, throws an error and exits.
// use this only when an EOF at the current position would imply a malformed input.
char assertedFGetC(FILE* source);

// print the first x characters from a char array
void printFromCharArray(char* source, int length);