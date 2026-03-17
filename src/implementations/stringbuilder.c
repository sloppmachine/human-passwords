#include <stdio.h>
#include <stdlib.h>

#include <interface/commons.h>
#include <interface/stringbuilder.h>

struct stringBuilder* newStringBuilder() {
    struct stringBuilder* const toReturn = saferMalloc(sizeof(struct stringBuilder), "stringBuilder");
    toReturn -> first = NULL;
    return toReturn;
}

void appendCharToStringBuilder(struct stringBuilder* _stringBuilder, char character) {
    struct stringBuilderChar* const toInsert = saferMalloc(sizeof(struct stringBuilderChar), "stringBuilderChar");
    toInsert -> content = character;
    toInsert -> next = NULL;

    if (_stringBuilder -> first) {
        // try to find the last entry
        struct stringBuilderChar* last = _stringBuilder -> first;
        while (last -> next) {
            last = last -> next;
        }
        last -> next = toInsert;
    } else {
        _stringBuilder -> first = toInsert;
    }
}

char* finalizeStringBuilder(struct stringBuilder* _stringBuilder) {
    // guarantee the contents exist
    if (!_stringBuilder -> first) {
        printf("Warning: tried to finalize an empty string builder.\n");
        return NULL;
    }

    struct stringBuilderChar* current = _stringBuilder -> first;
    int length = 1;
    while (current -> next) {
        current = current -> next;
        length += 1;
    }

    // allocate a char array with the string builder's length plus space for the null terminator
    char* const toReturn = saferMalloc((length + 1) * sizeof(char), "finalized string from stringBuilder");
    current = _stringBuilder -> first;
    for (int i = 0; i < length; i++) {
        toReturn[i] = current -> content;
        current = current -> next;
    }
    toReturn[length] = '\0';

    return toReturn;
}