#include <ctype.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <constants/alphanumerics.h>
#include <constants/cmdresponses.h>
#include <constants/modes.h>

#include <interface/commons.h>
#include <interface/huffmantree.h>
#include <interface/preprocessing.h>
#include <interface/seeds.h>
#include <interface/storage.h>

// exits if the required argument wasn't given
void verifyRequiredArgument(char* _argument, char* _optionName);

// exits if the amount argument is malformed, otherwise stores it in an amount struct
struct amount* processAmountArgument(char* _argument);

void verifyRequiredArgument(char* _argument, char* _optionName) {
    bool wasArgumentProvided = true;
    if (!_argument) {
        // this is the case when you write no = at all
        wasArgumentProvided = false;
    } else if (_argument[0] == '\0') {
        // if you write "source=", then the argument is given as an empty string
        wasArgumentProvided = false;
    }
    if (!wasArgumentProvided) {
        printf("The option %s is required.\n", _optionName);
        exit(EXIT_FAILURE);
    }
}

struct amount* processAmountArgument(char* _argument) {
    // the argument needs to contain an 'x', and before and after that it needs to have a non-empty only-digit string

    // check for first occurence of 'x'
    char* xPointer = strchr(_argument, 'x');
    if (!xPointer) {
        printf(MALFORMED_AMOUNT_ARGUMENT_TEXT);
        exit(EXIT_FAILURE);
    }

    int xIndex = xPointer - _argument;

    // check the substring before and after the 'x' is not empty
    if (xIndex == 0 || _argument[xIndex + 1] == '\0') {
        printf(MALFORMED_AMOUNT_ARGUMENT_TEXT);
        exit(EXIT_FAILURE);
    }

    // find and copy the substrings before and after the 'x'
    char* beforeX = saferMalloc((xIndex + 1) * sizeof(char), "substring before the first 'x' in --argument");
    memcpy(beforeX, _argument, xIndex * sizeof(char));
    beforeX[xIndex] = '\0'; // create a new null terminator

    int afterXLength = strlen(_argument + xIndex + 1);
    char* afterX = saferMalloc((afterXLength + 1) * sizeof(char), "substring after the first 'x' in --argument");
    memcpy(afterX, _argument + xIndex + 1, (afterXLength + 1) * sizeof(char)); // include the null terminator

    if (!isStringDigit(beforeX) || !isStringDigit(afterX)) {
        printf(MALFORMED_AMOUNT_ARGUMENT_TEXT);
        exit(EXIT_FAILURE);
    }

    struct amount* toReturn = saferMalloc(sizeof(struct amount), "amount struct");
    toReturn -> amoutOfPasswords = atoi(beforeX);
    toReturn -> wordsPerPassword = atoi(afterX);

    return toReturn;
}

int main(int argc, char** argv) {
    // the alphabet
    char* lowercaseAlphabet = saferMalloc(sizeof(LOWERCASE_ALPHABET), "lowercase alphabet");
    for (int i = 0; i < ALPHABET_LENGTH; i++) {
        lowercaseAlphabet[i] = LOWERCASE_ALPHABET[i];
    }

    // possible command line options
    int help = 0;
    int verbose = 0;
    char* sourceFileName = NULL;
    char* targetFileName = NULL;
    char* amountInput = NULL; // this is the string that was inputted; not the extracted data.
    char* separator = DEFAULT_SEPARATOR;

    // these define the behavior for long options: https://sourceware.org/glibc/manual/latest/html_mono/libc.html#Parsing-Long-Options-with-getopt_005flong
    // note that source does require an argument, but we want to handle the case ourself if none was provided.
    // for example, if --help was provided, we still want to print the help as usual, regardless whether the other required arguments were given.
    struct option longOptions[] = {
        {"help", no_argument, &help, 1},
        {"verbose", no_argument, &verbose, 1},
        {"amount", optional_argument, 0, 'a'},
        {"seperator", optional_argument, 0, 'p'},
        {"source", optional_argument, 0, 's'},
        {"target", optional_argument, 0, 't'},
        {0, 0, 0, 0}
    };
    
    char shortOption;
    int optionIndex = 0;
    while (true) {
        // returns a short option if the next option is a short one, else acts according to the option struct
        char shortOption = getopt_long_only(argc, argv, "hva::p::s::t::", longOptions, &optionIndex);
        if (shortOption == -1) {
            // the options have ended
            break;
        }

        switch (shortOption) {
            case 'h':
                help = 1;
                break;
            case 'a':
                amountInput = optarg;
                break;
            case 'p':
                separator = optarg;
                break;
            case 's':
                sourceFileName = optarg;
                break;
            case 't':
                targetFileName = optarg;
                break;
            case 'v':
                verbose = 1;
                break;
        }
    }

    // find out which mode was given
    Mode mode;
    if (optind >= argc) {
        // in this case, all arguments have already been seen when reading the options and therefore no mode was provided
        mode = MODE_DEFAULT;
    } else if (optind + 2 <= argc) {
        // this case occurs if there are more than one argument that are not options
        printf(TOO_MANY_MODES_TEXT);
        exit(EXIT_FAILURE);
    } else {
        // in this case exactly 1 argument is not an option (it is interpreted as the mode)
        char* modeInput = argv[argc - 1];

        // strcmp returns 0 if and only if both strings are equal
        if (!strcmp(modeInput, "build")) {
            mode = MODE_BUILD;
        } else if (!strcmp(modeInput, "restore")) {
            mode = MODE_RESTORE;
        } else if (!strcmp(modeInput, "extract")) {
            mode = MODE_EXTRACT;
        } else {
            printf(UNKNOWN_MODE_TEXT);
            exit(EXIT_FAILURE);
        }
    }

    // declare file pointers beforehand; multiple declarations in different switch cases are illegal
    FILE* sourceFile;
    FILE* targetFile;
    // decide action to take
    switch (mode) {
        case MODE_DEFAULT:
            if (help) {
                printf(HELP_TEXT_DEFAULT_MODE);
            } else {
                printf(DEFAULT_TEXT);
            }
            break;
        
        case MODE_BUILD:
            // if help was asked, ignore everything else
            if (help) {
                printf(HELP_TEXT_BUILD_MODE);
                break;
            }

            verifyRequiredArgument(sourceFileName, "--source=<raw word file>");
            verifyRequiredArgument(targetFileName, "--target=<target binary file name>");
            
            printIfVerbose(verbose, "Building binary from raw word list with verbose setting\n");

            sourceFile = openSourceFile(verbose, sourceFileName);

            printIfVerbose(verbose, "Measuring the distribution of characters...\n");
            int* distribution = getCharacterDistributionFromFile(
                lowercaseAlphabet, ALPHABET_LENGTH, sourceFile
            );
            if (verbose) {
                printCharacterDistribution(distribution, lowercaseAlphabet, ALPHABET_LENGTH);
            }
            
            printIfVerbose(verbose, "Building huffman tree...\n");
            struct huffmanTree* tree = buildHuffmanTreeFromDistribution(lowercaseAlphabet, ALPHABET_LENGTH, distribution);
            if (verbose) {
                printHuffmanCodes(tree, lowercaseAlphabet, ALPHABET_LENGTH);
            }

            targetFile = openTargetFile(verbose, targetFileName);

            printIfVerbose(verbose, "Writing binary...\n");
            buildWordPoolFile(sourceFile, targetFile, tree, lowercaseAlphabet, ALPHABET_LENGTH);

            printIfVerbose(verbose, "Cleaning up...\n");
            fclose(sourceFile);
            fclose(targetFile);
            break;
        
        case MODE_RESTORE:
            // if help was asked, ignore everything else
            if (help) {
                printf(HELP_TEXT_RESTORE_MODE);
                break;
            }

            verifyRequiredArgument(sourceFileName, "--source=<raw word file>");
            verifyRequiredArgument(targetFileName, "--target=<target binary file name>");

            printIfVerbose(verbose, "Restoring raw word list from binary with verbose setting\n");

            sourceFile = openSourceFile(verbose, sourceFileName);
            targetFile = openTargetFile(verbose, targetFileName);

            restoreRawWordList(sourceFile, targetFile, lowercaseAlphabet, ALPHABET_LENGTH, verbose);
            
            break;
        case MODE_EXTRACT:
            if (help) {
                printf(HELP_TEXT_EXTRACT_MODE);
                break;
            }

            verifyRequiredArgument(sourceFileName, "--source=<raw word file>");
            verifyRequiredArgument(targetFileName, "--target=<target binary file name>");
            verifyRequiredArgument(amountInput, "--amount=<amount of passwords>x<words per password>");
            
            printf("got source %s target %s amount input %s seperator %s \n", sourceFileName, targetFileName, amountInput, separator);

            struct amount* amount = processAmountArgument(amountInput);
            printf("got following to amount: %i %i\n", amount -> amoutOfPasswords, amount -> wordsPerPassword);
            
            sourceFile = openSourceFile(verbose, sourceFileName);
            targetFile = openTargetFile(verbose, targetFileName);

            printIfVerbose(verbose, "getting size of word pool...\n");
            unsigned int wordPoolSize = getWordPoolSize(sourceFile);

            printIfVerbose(verbose, "generating random seeds...\n");
            int** seedArray = generateSeedArray(amount, wordPoolSize);

            for (int layer1 = 0; layer1 < amount -> amoutOfPasswords; layer1++) {
                for (int layer2 = 0; layer2 < amount -> wordsPerPassword; layer2++) {
                    printf("%i %i %u\n", layer1, layer2, seedArray[layer1][layer2]);
                }
            }

            printIfVerbose(verbose, "generating sorted seed list for retrieval...\n");
            struct seedsToFind* seedsToFind = getSeedsToFind(seedArray, amount);
            
            printf("list length %i contents ", seedsToFind -> amount);
            for (int i = 0; i < seedsToFind -> amount; i++) {
                printf("%i; ", seedsToFind -> sortedArray[i]);
            }
            printf("\n");

            printIfVerbose(verbose, "translating seeds to words...\n");
            struct translatedSeedList* translatedSeedList = translateSeedListWithWordPool(
                sourceFile,
                seedsToFind,
                lowercaseAlphabet,
                ALPHABET_LENGTH,
                verbose
            );

            printIfVerbose(verbose, "writing passwords to file...\n");

            writeTranslatedSeedsToFile(targetFile, seedArray, amount, translatedSeedList, separator, verbose);

            printIfVerbose(verbose, "cleaning up...\n");

            freeSeedArray(seedArray, amount -> amoutOfPasswords);
            free(amount);
            break;
    }

}