#include <stdbool.h>
#include <stdlib.h>

#include <interface/bitbuffer.h>
#include <interface/commons.h>

static struct bitBufferElement* newBitBufferElement(bool _content);

// this method guarantees either a 0 or a 1. no other values
static char flushSingleBit(struct bitBuffer* _bitBuffer);

struct bitBuffer* newBitBuffer() {
    struct bitBuffer* toReturn = saferMalloc(sizeof(struct bitBuffer), "bitBuffer");
    toReturn -> size = 0;
    toReturn -> first = NULL;
    toReturn -> last = NULL;
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
    toReturn -> next = NULL;
    toReturn -> last = NULL;
    return toReturn;
}

void addBit(struct bitBuffer* _bitBuffer, bool _bit) {
    struct bitBufferElement* toInsert = newBitBufferElement(_bit);
    if (_bitBuffer -> size) {
        toInsert -> last = _bitBuffer -> last;
        _bitBuffer -> last -> next = toInsert;
        _bitBuffer -> last = toInsert;
    } else {
        _bitBuffer -> first = toInsert;
        _bitBuffer -> last = toInsert;
    }
    _bitBuffer -> size++;
}

void addByte(struct bitBuffer* _bitBuffer, unsigned char _byte) {
    for (int i = 0; i < 8; i++) {
        addBit(_bitBuffer, _byte & 0x80); // this is the case iff the first bit is 1
        _byte = _byte << 1;
    }
}

static char flushSingleBit(struct bitBuffer* _bitBuffer) {
    if (_bitBuffer -> size) {
        struct bitBufferElement* firstElement = _bitBuffer -> first;
        char toReturn;
        if (firstElement -> content) {
            toReturn = 1;
        } else {
            toReturn = 0;
        }
        _bitBuffer -> first = firstElement -> next;
        free(firstElement);
        // if we just removed the last element, we also need to update the field of the struct
        if (!_bitBuffer -> first) {
            _bitBuffer -> last = NULL;
        }
        _bitBuffer -> size--;
        return toReturn;
    } else {
        return 0;
    }
}

unsigned char flushSingleByte(struct bitBuffer* _bitBuffer) {
    unsigned char toReturn = 0;
    // read a bit, then shift left. the most significant bits should be those that appeared first in the bit buffer
    for (int i = 0; i < 8; i++) {
        toReturn = toReturn << 1;
        toReturn += flushSingleBit(_bitBuffer);
    }
    return toReturn;
}

int flushEncodedCharacter(struct bitBuffer* _bitBuffer, char** _encodings, int _alphabetLength) {
    // the beginning of the bit buffer may only hold up to one encoding. there is no ambiguity
    for (int currentEncodingIndex = 0; currentEncodingIndex < _alphabetLength; currentEncodingIndex++) {
        char* encoding = _encodings[currentEncodingIndex];
        // check if the encoding matches the beginning of the bitbuffer (compare the two).
        // the matching is successful if we reach the null terminator of the encoding before the bit buffer ends or holds different values than the encoding
        bool isEncodingMatched = true;

        int currentIndexInEncoding = 0;
        struct bitBufferElement* currentBitBufferElement = _bitBuffer -> first;
        while (true) {
            char currentEncodingChar = encoding[currentIndexInEncoding];
            if (currentEncodingChar == '\0') {
                // the encoding has ended and been matched successfully
                break;
            }
            if (!currentBitBufferElement) {
                // the bit buffer ended before the encoding could be matched
                isEncodingMatched = false;
                break;
            } else {
                bool currentEncodingBit = !(currentEncodingChar == '0'); // convert '0' and '1' to "booleans" (zero and nonzero ints)
                bool currentBitBufferBit = currentBitBufferElement -> content;
                // if the encoding does not match with the bit in the bitbuffer, the matching fails
                if (!currentEncodingBit && currentBitBufferBit || currentEncodingBit && !currentBitBufferBit) {
                    isEncodingMatched = false;
                    break;
                }
            }
            currentIndexInEncoding++;
            currentBitBufferElement = currentBitBufferElement -> next;
        }
        if (isEncodingMatched) {
            //printf("seems we matched an encoding, character index %i, encoding %s \n", currentEncodingIndex, encoding);
            // actually flush all bits (currentIndexInEncoding is the amount of bits to flush)
            for (int i = 0; i < currentIndexInEncoding; i++) {
                flushSingleBit(_bitBuffer);
            }
            return currentEncodingIndex;
        }
    }
    return -1;
}