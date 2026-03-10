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
#include <interface/storage.h>

void verifyRequiredArgument(char* argument, char* optionName) {
    bool wasArgumentProvided = true;
    if (!argument) {
        // this is the case when you write no = at all
        wasArgumentProvided = false;
    } else if (argument[0] == '\0') {
        // if you write "source=", then the argument is given as an empty string
        wasArgumentProvided = false;
    }
    if (!wasArgumentProvided) {
        printf("The option %s is required.\n", optionName);
        exit(EXIT_FAILURE);
    }
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
    char* source = NULL;
    char* target = NULL;

    // these define the behavior for long options: https://sourceware.org/glibc/manual/latest/html_mono/libc.html#Parsing-Long-Options-with-getopt_005flong
    // note that source does require an argument, but we want to handle the case ourself if none was provided.
    // for example, if --help was provided, we still want to print the help as usual, regardless whether the other required arguments were given.
    struct option longOptions[] = {
        {"help", no_argument, &help, 1},
        {"verbose", no_argument, &verbose, 1},
        {"source", optional_argument, 0, 's'},
        {"target", optional_argument, 0, 't'},
        {0, 0, 0, 0}
    };
    
    char shortOption;
    int optionIndex = 0;
    while (true) {
        // returns a short option if the next option is a short one, else acts according to the option struct
        char shortOption = getopt_long_only(argc, argv, "hv", longOptions, &optionIndex);
        if (shortOption == -1) {
            // the options have ended
            break;
        }

        switch (shortOption) {
            case 'h':
                help = 1;
                break;
            case 's':
                source = optarg;
                break;
            case 't':
                target = optarg;
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

            verifyRequiredArgument(source, "--source=<raw word file>");
            verifyRequiredArgument(target, "--target=<target binary file name>");
            
            printIfVerbose(verbose, "Building binary from raw word list with verbose setting\n");

            sourceFile = fopen(source, "rb");
            if (sourceFile) {
                printIfVerbose(verbose, "Successfully opened %s ...\n", source);
            } else {
                printf("The file %s doesn't seem to exist.\n", source);
                exit(EXIT_FAILURE);
            }

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

            targetFile = fopen(target, "wb");
            if (target) {
                printIfVerbose(verbose, "Successfully opened %s ...\n", target);
            } else {
                printf("Can't create or open the file %s .\n", target);
                exit(EXIT_FAILURE);
            }

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

            verifyRequiredArgument(source, "--source=<raw word file>");
            verifyRequiredArgument(target, "--target=<target binary file name>");

            printIfVerbose(verbose, "Restoring raw word list from binary with verbose setting\n");

            sourceFile = fopen(source, "rb");
            if (sourceFile) {
                printIfVerbose(verbose, "Successfully opened %s ...\n", source);
            } else {
                printf("The file %s doesn't seem to exist.\n", source);
                exit(EXIT_FAILURE);
            }

            targetFile = fopen(target, "wb");
            if (target) {
                printIfVerbose(verbose, "Successfully opened %s ...\n", target);
            } else {
                printf("Can't create or open the file %s .\n", target);
                exit(EXIT_FAILURE);
            }

            restoreRawWordList(sourceFile, targetFile, lowercaseAlphabet, ALPHABET_LENGTH, verbose);
            
            break;
        case MODE_EXTRACT:
            printf("in extract mode\n");
            break;
    }

}