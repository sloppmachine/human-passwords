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
void verifyRequiredArgument(const char* _argument, const char* _optionName);

// exits if the amount argument is malformed, otherwise stores it in an amount struct
const struct amount* processAmountArgument(const char* _argument);

void verifyRequiredArgument(const char* _argument, const char* _optionName) {
    bool wasArgumentProvided = true;
    if (!_argument) {
        // this is the case when you write no = at all
        wasArgumentProvided = false;
    } else if (_argument[0] == '\0') {
        // if you write "source=", then the argument is given as an empty string
        wasArgumentProvided = false;
    }
    if (!wasArgumentProvided) {
        printf("Error: the option %s is required.\n", _optionName);
        exit(EXIT_FAILURE);
    }
}

const struct amount* processAmountArgument(const char* _argument) {
    // the argument needs to contain an 'x', and before and after that it needs to have a non-empty only-digit string

    // check for first occurence of 'x'
    const char* const xPointer = strchr(_argument, 'x');
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
    char* const beforeX = saferMalloc((xIndex + 1) * sizeof(char), "substring before the first 'x' in --argument");
    memcpy(beforeX, _argument, xIndex * sizeof(char));
    beforeX[xIndex] = '\0'; // create a new null terminator

    int afterXLength = strlen(_argument + xIndex + 1);
    char* const afterX = saferMalloc((afterXLength + 1) * sizeof(char), "substring after the first 'x' in --argument");
    memcpy(afterX, _argument + xIndex + 1, (afterXLength + 1) * sizeof(char)); // include the null terminator

    if (!isStringDigit(beforeX) || !isStringDigit(afterX)) {
        printf(MALFORMED_AMOUNT_ARGUMENT_TEXT);
        exit(EXIT_FAILURE);
    }

    struct amount* const toReturn = saferMalloc(sizeof(struct amount), "amount struct");
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

    // possible command line options, fitted to struct option
    int help = 0;
    int verbose = 0;
    const char* sourceFileName = NULL;
    const char* targetFileName = NULL;
    const char* amountInput = NULL; // this is the string that was inputted; not the extracted data.
    const char* separator = DEFAULT_SEPARATOR;

    // these define the behavior for long options: https://sourceware.org/glibc/manual/latest/html_mono/libc.html#Parsing-Long-Options-with-getopt_005flong
    // note that source does require an argument, but we want to handle the case ourself if none was provided.
    // for example, if --help was provided, we still want to print the help as usual, regardless whether the other required arguments were given.
    const struct option longOptions[] = {
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
    enum Mode mode;
    if (optind >= argc) {
        // in this case, all arguments have already been seen when reading the options and therefore no mode was provided
        mode = DEFAULT;
    } else if (optind + 2 <= argc) {
        // this case occurs if there are more than one argument that are not options
        printf(TOO_MANY_MODES_TEXT);
        exit(EXIT_FAILURE);
    } else {
        // in this case exactly 1 argument is not an option (it is interpreted as the mode)
        char* modeInput = argv[argc - 1];

        // strcmp returns 0 if and only if both strings are equal
        if (!strcmp(modeInput, "build")) {
            mode = BUILD;
        } else if (!strcmp(modeInput, "restore")) {
            mode = RESTORE;
        } else if (!strcmp(modeInput, "extract")) {
            mode = EXTRACT;
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
        case DEFAULT:
            if (help) {
                printf(HELP_TEXT_DEFAULT_MODE);
            } else {
                printf(DEFAULT_TEXT);
            }
            break;
        
        case BUILD:
            // if help was asked, ignore everything else
            if (help) {
                printf(HELP_TEXT_BUILD_MODE);
                break;
            }

            verifyRequiredArgument(sourceFileName, "--source=<raw word file>");
            verifyRequiredArgument(targetFileName, "--target=<target binary file name>");
            
            printIfVerbose(verbose, "Building binary from raw word list with verbose setting.\n");

            sourceFile = openSourceFile(verbose, sourceFileName);

            printIfVerbose(verbose, "Measuring the distribution of characters...\n");
            const int* distribution = getCharacterDistributionFromFile(
                lowercaseAlphabet, ALPHABET_LENGTH, sourceFile
            );
            
            printIfVerbose(verbose, "Building huffman tree...\n");
            struct huffmanTree* const tree = buildHuffmanTreeFromDistribution(lowercaseAlphabet, ALPHABET_LENGTH, distribution);

            targetFile = openTargetFile(verbose, targetFileName);

            printIfVerbose(verbose, "Writing binary...\n");
            buildWordPoolFile(sourceFile, targetFile, tree, lowercaseAlphabet, ALPHABET_LENGTH, verbose);

            printIfVerbose(verbose, "Cleaning up...\n");
            fclose(sourceFile);
            fclose(targetFile);
            break;
        
        case RESTORE:
            // if help was asked, ignore everything else
            if (help) {
                printf(HELP_TEXT_RESTORE_MODE);
                break;
            }

            verifyRequiredArgument(sourceFileName, "--source=<raw word file>");
            verifyRequiredArgument(targetFileName, "--target=<target binary file name>");

            printIfVerbose(verbose, "Restoring raw word list from binary with verbose setting.\n");

            sourceFile = openSourceFile(verbose, sourceFileName);
            targetFile = openTargetFile(verbose, targetFileName);

            restoreRawWordList(sourceFile, targetFile, lowercaseAlphabet, ALPHABET_LENGTH, verbose);

            printIfVerbose(verbose, "Cleaning up...\n");
            fclose(sourceFile);
            fclose(targetFile);
            break;
        case EXTRACT:
            if (help) {
                printf(HELP_TEXT_EXTRACT_MODE);
                break;
            }

            verifyRequiredArgument(sourceFileName, "--source=<raw word file>");
            verifyRequiredArgument(targetFileName, "--target=<target binary file name>");
            verifyRequiredArgument(amountInput, "--amount=<amount of passwords>x<words per password>");
            const struct amount* amount = processAmountArgument(amountInput);

            printIfVerbose(verbose, "Extracting passphrase from binary with verbose setting.\n");
            
            sourceFile = openSourceFile(verbose, sourceFileName);
            targetFile = openTargetFile(verbose, targetFileName);

            printIfVerbose(verbose, "Generating random seeds...\n");
            const unsigned int wordPoolSize = getWordPoolSize(sourceFile);
            const int** seedArray = generateSeedArray(amount, wordPoolSize);

            printIfVerbose(verbose, "Generating sorted seed list for retrieval...\n");
            const struct seedsToFind* const seedsToFind = getSeedsToFind(seedArray, amount);

            printIfVerbose(verbose, "Translating seeds using the binary...\n");
            const struct translatedSeedList* translatedSeedList = translateSeedListWithWordPool(
                sourceFile,
                seedsToFind,
                lowercaseAlphabet,
                ALPHABET_LENGTH,
                verbose
            );

            printIfVerbose(verbose, "Writing passphrases to file...\n");

            writeTranslatedSeedsToFile(targetFile, seedArray, amount, translatedSeedList, separator, verbose);

            printIfVerbose(verbose, "Cleaning up...\n");
            freeSeedArray(seedArray, amount -> amoutOfPasswords);
            freeSeedsToFind((struct seedsToFind*) seedsToFind); // make non-constant
            freeTranslatedSeedList((struct translatedSeedList*) translatedSeedList);
            free((struct amount*) amount);
            fclose(sourceFile);
            fclose(targetFile);
            break;
    }

}