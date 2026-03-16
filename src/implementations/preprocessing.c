#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <constants/alphanumerics.h>

#include <interface/commons.h>

bool isStringDigit(char* _string) {
    bool toReturn = true;
    int i = 0;
    while (true) {
        char c = _string[i];
        if (c == '\0') {
            break;
        }
        if (!isdigit(c)) {
            toReturn = false;
            break;
        }
        i++;
    }
    return toReturn;
}

void makeLowercase(char* _input) {
    int i = 0;
    char c = _input[0]; // starting value
    while (c != '\0') {
        c = _input[i];
        char* characterUppercasePointer = strchr(UPPERCASE_ALPHABET, c);
        if (characterUppercasePointer) {
            _input[i] = LOWERCASE_ALPHABET[characterUppercasePointer - UPPERCASE_ALPHABET];
        }
        i++;
    }
}

int* getCharacterDistributionFromCharArray(char* _alphabet, int _alphabetSize, char* _input, int _inputSize) {
    // we will do some voodoo to improve the speed here.
    // we will make our own copies of the alphabet, and each time we encounter a character, we swap it with its predecessor
    // this means that over time, common characters will automatically move forward in the alphabet, and can be found quicker
    // at the end we can use the original alphabet to restore ("unswap") our data

    char* swappableAlphabet = saferMalloc(_alphabetSize * sizeof(char), "char array");
    int* swappableOccurences = saferMalloc(_alphabetSize * sizeof(int), "int array");
    for (int i = 0; i < _alphabetSize; i++) {
        swappableAlphabet[i] = _alphabet[i];
        swappableOccurences[i] = 0;
    }

    for (int currentInputPosition = 0; currentInputPosition < _inputSize; currentInputPosition++) {
        char currentChar = _input[currentInputPosition];

        for (int currentCharAlphabetIndex = 0; currentCharAlphabetIndex < _alphabetSize; currentCharAlphabetIndex++) {
            // find the position of the current character in the rearrangeable alphabet
            if (swappableAlphabet[currentCharAlphabetIndex] == currentChar) {
                swappableOccurences[currentCharAlphabetIndex]++;
                if (currentCharAlphabetIndex) {
                    // if currentCharInAlphabet != 0 then we swap it with its preceding character
                    char tempChar = swappableAlphabet[currentCharAlphabetIndex - 1];
                    swappableAlphabet[currentCharAlphabetIndex - 1] = swappableAlphabet[currentCharAlphabetIndex];
                    swappableAlphabet[currentCharAlphabetIndex] = tempChar;
                    int tempOccurence = swappableOccurences[currentCharAlphabetIndex - 1];
                    swappableOccurences[currentCharAlphabetIndex - 1] = swappableOccurences[currentCharAlphabetIndex];
                    swappableOccurences[currentCharAlphabetIndex] = tempOccurence;
                }
            }
        }
    }

    // rearrange the alphabet by swapping characters as long as necessary, in turn telling us how to order the occurences correctly
    while (true) {
        int somethingChanged = false;

        // find the first index in which the given alphabet and the rearrangable alphabet differ
        for (int i = 0; i < _alphabetSize; i++) {
            if (_alphabet[i] != swappableAlphabet[i]) {
                // now look for the index j in swappableAlphabet where swappableAlphabet[j] == _alphabet[i]
                // we know this point cannot have occured yet
                int j = i + 1;
                while (j < _alphabetSize) {
                    if (_alphabet[i] == swappableAlphabet[j]) {
                        break;
                    }
                    j++;
                }

                // now swap the elements at index i and j in swappableAlphabet and swappableOccurences
                char tempChar = swappableAlphabet[i];
                int tempOccurance = swappableOccurences[i];
                swappableAlphabet[i] = swappableAlphabet[j];
                swappableAlphabet[j] = tempChar;
                swappableOccurences[i] = swappableOccurences[j];
                swappableOccurences[j] = tempOccurance;

                somethingChanged = true;
            }
        }

        if (!somethingChanged) {
            break;
        }
    }

    // clean up
    free(swappableAlphabet);

    return swappableOccurences;
}

int* getCharacterDistributionFromFile(char* _alphabet, int _alphabetSize, FILE* _input) {
    // this is functionally just like getCharacterDistributionFromCharArray, therefore no extra comments
    // we need to additionally keep track of the file pointer
    int startingPosition = ftell(_input);

    char* swappableAlphabet = saferMalloc(_alphabetSize * sizeof(char), "char array");
    int* swappableOccurences = saferMalloc(_alphabetSize * sizeof(int), "int array");
    for (int i = 0; i < _alphabetSize; i++) {
        swappableAlphabet[i] = _alphabet[i];
        swappableOccurences[i] = 0;
    }

    char currentChar;
    while (true) {
        currentChar = fgetc(_input);
        if (currentChar == EOF) {
            break;
        }

        for (int currentCharAlphabetIndex = 0; currentCharAlphabetIndex < _alphabetSize; currentCharAlphabetIndex++) {
            if (swappableAlphabet[currentCharAlphabetIndex] == currentChar) {
                swappableOccurences[currentCharAlphabetIndex]++;
                if (currentCharAlphabetIndex) {
                    char tempChar = swappableAlphabet[currentCharAlphabetIndex - 1];
                    swappableAlphabet[currentCharAlphabetIndex - 1] = swappableAlphabet[currentCharAlphabetIndex];
                    swappableAlphabet[currentCharAlphabetIndex] = tempChar;
                    int tempOccurence = swappableOccurences[currentCharAlphabetIndex - 1];
                    swappableOccurences[currentCharAlphabetIndex - 1] = swappableOccurences[currentCharAlphabetIndex];
                    swappableOccurences[currentCharAlphabetIndex] = tempOccurence;
                }
            }
        }
    }

    while (true) {
        int somethingChanged = false;

        for (int i = 0; i < _alphabetSize; i++) {
            if (_alphabet[i] != swappableAlphabet[i]) {
                int j = i + 1;
                while (j < _alphabetSize) {
                    if (_alphabet[i] == swappableAlphabet[j]) {
                        break;
                    }
                    j++;
                }

                char tempChar = swappableAlphabet[i];
                int tempOccurance = swappableOccurences[i];
                swappableAlphabet[i] = swappableAlphabet[j];
                swappableAlphabet[j] = tempChar;
                swappableOccurences[i] = swappableOccurences[j];
                swappableOccurences[j] = tempOccurance;

                somethingChanged = true;
            }
        }

        if (!somethingChanged) {
            break;
        }
    }

    free(swappableAlphabet);
    fseek(_input, startingPosition, SEEK_SET);

    return swappableOccurences;
}

void printCharacterDistribution(int* _distribution, char* _alphabet, int _alphabetLength) {
    for (int i = 0; i < _alphabetLength; i++) {
        char currentCharacter = _alphabet[i];
        if (currentCharacter == '\n') {
            printf("character \'\\n\' occurs %i times\n", _distribution[i]);
        } else {
            printf("character \'%c\' occurs %i times\n", _alphabet[i], _distribution[i]);
        }
    }
}