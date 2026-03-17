#include <stdlib.h>
#include <stdio.h>

void* saferMalloc(int _size, const char* _objectDescription) {
    void* const toReturn = malloc(_size);
    if (toReturn == NULL) {
        printf("Error: failed to allocate object of type \"%s\".\n", _objectDescription);
        exit(EXIT_FAILURE);
    }
    return toReturn;
}

void assert(int _expression, const char* _errorDescription) {
    if (!_expression) {
        printf("Error: %s\n", _errorDescription);
        exit(EXIT_FAILURE);
    }
}

char assertedFGetC(FILE* source) {
    int toReturn = fgetc(source);
    if (toReturn == -1) {
        printf("Error: could not read from file. It might be corruped or you might be lacking permissions.\n");
        exit(EXIT_FAILURE);
    }
    return (char) toReturn;
}

void printFromCharArray(char* source, int length) {
    for (int i = 0; i < length; i++) {
        printf("%c", source[i]);
    }
}