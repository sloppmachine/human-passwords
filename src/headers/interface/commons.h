// this structure is just for saving a list of words pulled from pool
struct wordList {
    char** pointers;
    int* lengths;
};

// tries to allocate an object of the given size. if allocation fails, it gives a message and aborts.
void* saferMalloc(int _size, char* _objectDescription);

// if the expression is false, this function terminates the program.
void assert(int _expression, char* _errorDescription);

// print the first x characters from a char array
void printFromCharArray(char* source, int length);