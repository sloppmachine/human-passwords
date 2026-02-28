#include <stdlib.h>
#include <stdio.h>

#include <constants/alphanumerics.h>

#include <interface/commons.h>
#include <interface/huffmantree.h>
#include <interface/preprocessing.h>

// a checkpoint where we wait for user input
void returnToProceed() {
    printf("\n> press return to continue.\n");
    fgetc(stdin);
}

void huffmanTreeWalkthrough(char* placeholder, int placeholderLength, char* lowercaseAlphabet, int alphabetLength) {
    printf("walking through the process of building a huffman tree.\n");
    printf("starting with: %s\n", placeholder);
    printf("making lowercase...\n");
    makeLowercase(placeholder);
    printf("after making lowercase: %s\n", placeholder);
    returnToProceed();

    // TODO: TEST EMPTY ALPHABET

    printf("getting character distribution...\n");
    int* distribution = getCharacterDistribution(
        lowercaseAlphabet, alphabetLength, placeholder, placeholderLength
    );
    printf("printing character distribution...\n");
    for (int i = 0; i < alphabetLength; i++) {
        printf("character \'%c\' occurs %i times\n", lowercaseAlphabet[i], distribution[i]);
    }
    returnToProceed();

    printf("constructing huffman tree...\n");
    printf("this is the lowercase alphabet %s\n", lowercaseAlphabet);
    printf("this is the alhabet length: %i\n", alphabetLength);

    struct huffmanTree* tree = buildHuffmanTreeFromDistribution(lowercaseAlphabet, alphabetLength, distribution);
    returnToProceed();

    printf("trying to print the huffman tree's contents.\n");
    printHuffmanCodes(tree);
    printf("you can compare this result with browser-based huffman tree generators.\n");

    printf("finished huffman tree walkthrough.\n");
    returnToProceed();
    free(tree);

}

int main() {
    printf("initializing testing assets...\n");
    
    char* placeholder = saferMalloc(sizeof(PLACEHOLDER), "placeholder text");
    int placeholderLength = sizeof(PLACEHOLDER) / sizeof(char);
    for (int i = 0; i < sizeof(PLACEHOLDER); i++) {
        placeholder[i] = PLACEHOLDER[i];
    }

    char* lowercaseAlphabet = saferMalloc(sizeof(LOWERCASE_ALPHABET), "lowercase alphabet");
    int alphabetLength = sizeof(LOWERCASE_ALPHABET) / sizeof(char);
    for (int i = 0; i < alphabetLength; i++) {
        lowercaseAlphabet[i] = LOWERCASE_ALPHABET[i];
    }

    printf("beginning tests.\n");
    returnToProceed();

    huffmanTreeWalkthrough(placeholder, placeholderLength, lowercaseAlphabet, alphabetLength);

}