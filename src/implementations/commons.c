#include <stdlib.h>
#include <stdio.h>

void* saferMalloc(int _size, char* _objectDescription) {
    void* toReturn = malloc(_size);
    if (toReturn == NULL) {
        printf("Fatal error: failed to allocate object of type \"%s\".\n", _objectDescription);
        exit(EXIT_FAILURE);
    }
    return toReturn;
}

void assert(int _expression, char* _errorDescription) {
    if (!_expression) {
        printf("Fatal error: %s\n", _errorDescription);
        exit(EXIT_FAILURE);
    }
}

void printFromCharArray(char* source, int length) {
    for (int i = 0; i < length; i++) {
        printf("%c", source[i]);
    }
}