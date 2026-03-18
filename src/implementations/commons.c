#include <stdlib.h>
#include <stdio.h>

#include <constants/cmdresponses.h>

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

unsigned char assertedFGetC(FILE* source) {
    int toReturn = fgetc(source);
    if (toReturn == -1) {
        printf(COULD_NOT_READ_FILE_TEXT);
        exit(EXIT_FAILURE);
    }
    return (unsigned char) toReturn;
}

void printFromCharArray(char* source, int length) {
    for (int i = 0; i < length; i++) {
        printf("%c", source[i]);
    }
}