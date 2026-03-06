#include <stdbool.h>
#include <stdlib.h>

#include <interface/bitbuffer.h>
#include <interface/commons.h>

static struct bitBufferElement* newBitBufferElement(bool _content);

// this method guarantees either a 0 or a 1. no other values
static char flushSingleBit(struct bitBuffer* _bitBuffer);

struct bitBuffer* newBitBuffer() {
    struct bitBuffer* toReturn = saferMalloc(sizeof(struct bitBuffer), "bitBuffer");
    return toReturn;
}

void freeBitBuffer(struct bitBuffer* _bitBuffer) {
    struct bitBufferElement* current = _bitBuffer -> first;
    while (current) {
        struct bitBufferElement* next = current -> next;
        free(current);
        current = next;
    }
    free(_bitBuffer);
}

static struct bitBufferElement* newBitBufferElement(bool _content) {
    struct bitBufferElement* toReturn = saferMalloc(sizeof(struct bitBufferElement), "bitBufferElement");
    toReturn -> content = _content;
    return toReturn;
}

int getBitBufferSize(struct bitBuffer* _bitBuffer) {
    int toReturn = 0;
    struct bitBufferElement* current = _bitBuffer -> first;
    while (current) {
        toReturn++;
        current -> next;
    }
    return toReturn;
}

void addBit(struct bitBuffer* _bitBuffer, bool _bit) {
    struct bitBufferElement* toInsert = newBitBufferElement(_bit);
    if (_bitBuffer -> first) {
        // try to find the last entry
        struct bitBufferElement* last = _bitBuffer -> first;
        while (last -> next) {
            last = last -> next;
        }
        last -> next = toInsert;
    } else {
        _bitBuffer -> first = toInsert;
    }
}

static char flushSingleBit(struct bitBuffer* _bitBuffer) {
    if (_bitBuffer -> first) {
        struct bitBufferElement* firstElement = _bitBuffer -> first;
        char toReturn;
        if (firstElement -> content) {
            toReturn = 1;
        } else {
            toReturn = 0;
        }
        _bitBuffer -> first = firstElement -> next;
        free(firstElement);
        return toReturn;
    } else {
        return 0;
    }
}


char flushSingleByte(struct bitBuffer* _bitBuffer) {
    char toReturn = 0;
    // read a bit, then shift left.
    for (int i = 0; i < 8; i++) {
        toReturn += flushSingleBit(_bitBuffer);
        toReturn << 1;
    }
    return toReturn;
}