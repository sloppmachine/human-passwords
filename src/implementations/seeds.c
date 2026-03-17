#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>

#include <interface/commons.h>
#include <interface/seeds.h>

static struct translatedSeedListEntry* newTranslatedSeedListEntry(int _seed, const char* _word);

// also frees the word saved in it!
static void freeTranslatedSeedListEntry(struct translatedSeedListEntry* _entry);

void freeSeedsToFind(struct seedsToFind* _seedsToFind) {
    free(_seedsToFind -> sortedArray);
    free(_seedsToFind);
}

struct translatedSeedList* newTranslatedSeedList() {
    struct translatedSeedList* const toReturn = saferMalloc(sizeof(struct translatedSeedList), "translatedSeedList");
    toReturn -> first = NULL;
    return toReturn;
}

void freeTranslatedSeedList(struct translatedSeedList* _list) {
    struct translatedSeedListEntry* current = _list -> first;
    while (current) {
        struct translatedSeedListEntry* next = current -> next;
        freeTranslatedSeedListEntry(current);
        current = next;
    }
    free(_list);
}

static struct translatedSeedListEntry* newTranslatedSeedListEntry(int _seed, const char* _word) {
    struct translatedSeedListEntry* const toReturn = saferMalloc(sizeof(struct translatedSeedListEntry), "translatedSeedListEntry");
    toReturn -> next = NULL;
    toReturn -> seed = _seed;
    toReturn -> word = _word;
    return toReturn;
}

static void freeTranslatedSeedListEntry(struct translatedSeedListEntry* _entry) {
    free((char*) _entry -> word); // cast to a non-const pointer
    free(_entry);
}

void appendToTranslatedSeedList(struct translatedSeedList* _list, int _seed, char* _word) {
    struct translatedSeedListEntry* const toInsert = newTranslatedSeedListEntry(_seed, _word);
    if (_list -> first) {
        // try to find the last entry
        struct translatedSeedListEntry* last = _list -> first;
        while (last -> next) {
            last = last -> next;
        }
        last -> next = toInsert;
    } else {
        _list -> first = toInsert;
    }
}

const char* getSeedTranslation(int _seedToTranslate, const struct translatedSeedList* _list) {
    const char* toReturn = NULL;
    struct translatedSeedListEntry* current = _list -> first;
    while (current) {
        if (current -> seed == _seedToTranslate) {
            toReturn = current -> word;
            break;
        }
        current = current -> next;
    }
    return toReturn;
}

const int** generateSeedArray(const struct amount* _amount, unsigned int wordPoolSize) {
    const int amountOfPasswords = _amount -> amoutOfPasswords;
    const int wordsPerPassword = _amount -> wordsPerPassword;
    int** const toReturn = saferMalloc(sizeof(unsigned long int *) * amountOfPasswords, "2d seed array, 1st dimension");

    // accepting numbers beyong this point leads to modulo bias. if a random number is bigger than this, it is discarded.
    // it is the largest multiple of wordPoolSize that is still in range of an unsigned long int
    const unsigned long int retryZone = (ULONG_MAX / wordPoolSize) * wordPoolSize;

    for (int currentIndex1 = 0; currentIndex1 < amountOfPasswords; currentIndex1++) {
        toReturn[currentIndex1] = saferMalloc(sizeof(int) * wordsPerPassword, "2d seed array, 2nd dimension");
        for (int currentIndex2 = 0; currentIndex2 < wordsPerPassword; currentIndex2++) {
            unsigned long int randomNumber;
            while (true) {
                getrandom(&randomNumber, sizeof(unsigned long int), 0);
                if (randomNumber < retryZone) {
                    break;
                }
            }
            toReturn[currentIndex1][currentIndex2] = (int) (randomNumber % wordPoolSize);
        }
    }

    return (const int**) toReturn;
}

void freeSeedArray(const int** _seedArray, int _amountOfPasswords) {
    for (int i = 0; i < _amountOfPasswords; i++) {
        free((int*) _seedArray[i]);
    }
    free(_seedArray);
}

struct seedsToFind* getSeedsToFind(const int** _seeds, const struct amount* _amount) {
    const int level1Size = _amount -> amoutOfPasswords;
    const int level2Size = _amount -> wordsPerPassword;

    // initialize the struct assuming there are no duplicates (duplicates are handled later)
    struct seedsToFind* const toReturn = saferMalloc(sizeof(struct seedsToFind), "seedsToFind");
    toReturn -> amount = level1Size * level2Size;
    toReturn -> sortedArray = saferMalloc(level1Size * level2Size * sizeof(int), "seedsToFind sortedArray");
    
    // use selection sort (i think this is not a critical point concerning performance)
    // keep track of how many elements have been written to sortedArray; if these are less than level1Size * level2Size then there have been duplicates
    int elementsWritten = 0;
    int biggestElementWritten = -1;
    while (true) {
        // search for the smallest element not yet written
        int smallestElementNotWritten = -1;
        for (int level1Index = 0; level1Index < level1Size; level1Index++) {
            for (int level2Index = 0; level2Index < level2Size; level2Index++) {
                int current = _seeds[level1Index][level2Index];
                // never consider elements smaller than the biggest one already written
                if (biggestElementWritten < current) {
                    // check if there is a new candidate for smallestElementNotWritten
                    if (smallestElementNotWritten > current || smallestElementNotWritten == -1) {
                        smallestElementNotWritten = current;
                    }
                }
            }
        }

        // finish once no unwritten numbers are left
        if (smallestElementNotWritten == -1) {
            break;
        }

        toReturn -> sortedArray[elementsWritten] = smallestElementNotWritten;
        biggestElementWritten = smallestElementNotWritten;
        elementsWritten++;
    }

    // check for duplicates
    if (elementsWritten != toReturn -> amount) {
        const int* oldArray = toReturn -> sortedArray;
        const int oldArraySize = toReturn -> amount;
        // create a new array where duplicates are not copied over
        toReturn -> amount = elementsWritten;
        int* const newArray = saferMalloc(elementsWritten * sizeof(int), "seedsToReturn sortedArray");

        int newArrayIndex = 0;
        int lastElementCopied = -1;
        for (int oldArrayIndex = 0; oldArrayIndex < oldArraySize; oldArrayIndex++) {
            int currentOldArrayElement = oldArray[oldArrayIndex];
            if (lastElementCopied == -1) {
                newArray[newArrayIndex] = currentOldArrayElement;
                lastElementCopied = currentOldArrayElement;
                newArrayIndex++;
            } else if (lastElementCopied < currentOldArrayElement) {
                newArray[newArrayIndex] = currentOldArrayElement;
                lastElementCopied = currentOldArrayElement;
                newArrayIndex++;
            }
        }

        toReturn -> sortedArray = newArray;
        free((int*) oldArray);
    }

    return toReturn;
}