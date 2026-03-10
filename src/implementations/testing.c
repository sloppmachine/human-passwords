#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <constants/alphanumerics.h>

#include <interface/bitbuffer.h>
#include <interface/commons.h>
#include <interface/huffmantree.h>
#include <interface/preprocessing.h>
#include <interface/storage.h>

// a checkpoint where we wait for user input
void returnToProceed() {
    printf("\n> press return to continue.\n");
    fgetc(stdin);
}

void huffmanTreeWalkthrough(char* _placeholder, int _placeholderLength, char* _lowercaseAlphabet, int _alphabetLength) {
    printf("walking through the process of building a huffman tree.\n");
    printf("starting with: %s\n", _placeholder);
    printf("making lowercase...\n");
    makeLowercase(_placeholder);
    printf("after making lowercase: %s\n", _placeholder);
    returnToProceed();

    // TODO: TEST EMPTY ALPHABET

    printf("getting character distribution...\n");
    int* distribution = getCharacterDistributionFromCharArray(
        _lowercaseAlphabet, _alphabetLength, _placeholder, _placeholderLength
    );
    printf("printing character distribution...\n");
    for (int i = 0; i < _alphabetLength; i++) {
        printf("character \'%c\' occurs %i times\n", _lowercaseAlphabet[i], distribution[i]);
    }
    returnToProceed();

    printf("constructing huffman tree...\n");
    printf("this is the lowercase alphabet %s\n", _lowercaseAlphabet);
    printf("this is the alhabet length: %i\n", _alphabetLength);

    struct huffmanTree* tree = buildHuffmanTreeFromDistribution(_lowercaseAlphabet, _alphabetLength, distribution);
    returnToProceed();

    printf("trying to print the huffman tree's contents.\n");
    printHuffmanCodes(tree, _lowercaseAlphabet, _alphabetLength);
    printf("you can compare this result with browser-based huffman tree generators.\n");
    returnToProceed();

    free(tree);

}

void fullWalkthrough(char* _sourceName, char* _lowercaseAlphabet, int _alphabetLength, char* _targetName) {
    printf("trying to extract word distribution from the raw word file %s ...\n", _sourceName);

    FILE* rawWordFile = fopen(_sourceName, "rb");
    if (rawWordFile) {
        printf("Successfully opened the word source file...\n");
    } else {
        printf("The file %s doesn't seem to exist.\n", _sourceName);
        exit(EXIT_FAILURE);
    }

    int* distribution = getCharacterDistributionFromFile(
        _lowercaseAlphabet, _alphabetLength, rawWordFile
    );
    printf("printing character distribution...\n");
    for (int i = 0; i < _alphabetLength; i++) {
        char currentCharacter = _lowercaseAlphabet[i];
        if (currentCharacter == '\n') {
            printf("character \'\\n\' occurs %i times\n", distribution[i]);
        } else {
            printf("character \'%c\' occurs %i times\n", _lowercaseAlphabet[i], distribution[i]);
        }
    }
    returnToProceed();

    printf("building huffman tree...\n");
    struct huffmanTree* tree = buildHuffmanTreeFromDistribution(_lowercaseAlphabet, _alphabetLength, distribution);
    returnToProceed();

    printf("trying to print the huffman tree's contents.\n");
    printHuffmanCodes(tree, _lowercaseAlphabet, _alphabetLength);
    printf("you can compare this result with browser-based huffman tree generators.\n");
    returnToProceed();

    printf("trying to create the word pool binary...\n");
    FILE* wordPoolFile = fopen(_targetName, "wb");
    if (rawWordFile) {
        printf("Successfully opened the target file...\n");
    } else {
        printf("The file %s doesn't seem to exist.\n", _targetName);
        exit(EXIT_FAILURE);
    }
    buildWordPoolFile(rawWordFile, wordPoolFile, tree, _lowercaseAlphabet, _alphabetLength);
    returnToProceed();

    fclose(rawWordFile);

    printf("trying to read from the same file...\n");
    FILE* fileToRead = fopen(_targetName, "rb");
    assert(fileToRead != NULL, "could not open the file."); // the !=NULL is for the compiler not giving a warning

    extractFromWordPool(fileToRead, _lowercaseAlphabet, _alphabetLength, NULL, 0, true);

    fclose(fileToRead);

    printf("finished huffman tree walkthrough.\n");
    returnToProceed();

    free(tree);
    free(distribution);
}

int main() {
    printf("initializing testing assets...\n");
    
    char* placeholder = saferMalloc(sizeof(PLACEHOLDER), "placeholder text");
    int placeholderLength = sizeof(PLACEHOLDER) / sizeof(char);
    for (int i = 0; i < sizeof(PLACEHOLDER); i++) {
        placeholder[i] = PLACEHOLDER[i];
    }

    char* lowercaseAlphabet = saferMalloc(sizeof(LOWERCASE_ALPHABET), "lowercase alphabet");
    for (int i = 0; i < ALPHABET_LENGTH; i++) {
        lowercaseAlphabet[i] = LOWERCASE_ALPHABET[i];
    }

    printf("beginning tests.\n");
    returnToProceed();

    // huffmanTreeWalkthrough(placeholder, placeholderLength, lowercaseAlphabet, ALPHABET_LENGTH);
    fullWalkthrough("data/dump.txt", lowercaseAlphabet, ALPHABET_LENGTH, "testing/wordpool.bin");

}