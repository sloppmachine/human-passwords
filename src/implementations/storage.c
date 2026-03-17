#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <constants/storagehandling.h>

#include <interface/bitbuffer.h>
#include <interface/commons.h>
#include <interface/huffmantree.h>
#include <interface/seeds.h>
#include <interface/stringbuilder.h>
#include <interface/storage.h>

// writes a byte for the character, then a byte for the length of the code, then 0-bytes or 1-bytes for the code.
static void writeHuffmanTreeNodeToFile(FILE* _fileToWrite, const struct huffmanTreeNode* _node, const char* _prefix, int _prefixlength);

// recursively calls writeHuffmanTreeNodeToFile, and updates the amount of nodes saved
static void writeHuffmanTreeNodesToFile(
    FILE* _fileToWrite,
    const struct huffmanTreeNode* _node,
    const char* _prefix,
    int _prefixLength,
    int* _nodesSaved
);

// writes a byte documenting the length of the representation, followed by the single node representations in no necessary order.
static void writeHuffmanTreeToFile(FILE* _fileToWrite, struct huffmanTree* _tree);

// encodes words and writes them into a file
static void writeEncodedWordsToFile(FILE* _wordsToEncode, FILE* _fileToWrite, struct huffmanTree* _tree, char* _alphabet, int _alphabetSize);

// builds a node into a tree from its code
static void extractAndAddHuffmanTreeNodeFromFile(FILE* _file, struct huffmanTree* _tree);

// the file pointer should be at the byte where the amount of nodes is stored.
static struct huffmanTree* reconstructHuffmanTreeFromFile(FILE* _file);

FILE* openSourceFile(bool _verbose, const char* _fileName) {
    FILE* toReturn = fopen(_fileName, "rb");
    if (toReturn) {
        printIfVerbose(_verbose, "Successfully opened %s ...\n", _fileName);
    } else {
        printf("Error: can't open the file %s . It might not exist, or you might be lacking permissions.\n", _fileName);
        exit(EXIT_FAILURE);
    }
    return toReturn;
}

FILE* openTargetFile(bool _verbose, const char* _fileName) {
    FILE* toReturn = fopen(_fileName, "wb");
    if (toReturn) {
        printIfVerbose(_verbose, "Successfully opened %s ...\n", _fileName);
    } else {
        printf("Error: can't create or open the file %s . You might be lacking permissions.\n", _fileName);
        exit(EXIT_FAILURE);
    }
    return toReturn;
}

static void writeHuffmanTreeNodeToFile(FILE* _fileToWrite, const struct huffmanTreeNode* _node, const char* _prefix, int _prefixLength) {
    fputc(_node -> content, _fileToWrite);
    fputc(_prefixLength, _fileToWrite); // this is actually guaranteed to fit into a single byte.
    fwrite(_prefix, sizeof(char), _prefixLength, _fileToWrite);
}

static void writeHuffmanTreeNodesToFile(
    FILE* _fileToWrite,
    const struct huffmanTreeNode* _node,
    const char* _prefix,
    int _prefixLength,
    int* _nodesSaved
) {
    bool isInner = false;

    // check for children. technically both childs must simultaneously exist or not exist but i like this flow of code more
    if (_node -> leftChild) {
        isInner = true;
        char* const leftChildPrefix = saferMalloc(sizeof(char) * _prefixLength + 1, "huffman tree prefix");
        memcpy(leftChildPrefix, _prefix, _prefixLength);
        leftChildPrefix[_prefixLength] = '0';
        writeHuffmanTreeNodesToFile(_fileToWrite, _node -> leftChild, leftChildPrefix, _prefixLength + 1, _nodesSaved);
        free(leftChildPrefix);
    }
    if (_node -> rightChild) {
        isInner = true;
        char* const rightChildPrefix = saferMalloc(sizeof(char) * _prefixLength + 1, "huffman tree prefix");
        memcpy(rightChildPrefix, _prefix, _prefixLength);
        rightChildPrefix[_prefixLength] = '1';
        writeHuffmanTreeNodesToFile(_fileToWrite, _node -> rightChild, rightChildPrefix, _prefixLength + 1, _nodesSaved);
        free(rightChildPrefix);
    }
    if(!isInner) {
        writeHuffmanTreeNodeToFile(_fileToWrite, _node, _prefix, _prefixLength);
        (*_nodesSaved)++;
    }
}

static void writeHuffmanTreeToFile(FILE* _fileToWrite, struct huffmanTree* _tree) {
    // the first byte declares the amount of nodes of the following section. we will fill it in the end. a byte will be enough to store the number
    int reservedByteLocation = ftell(_fileToWrite);
    fputc('\0', _fileToWrite);

    int nodesSaved = 0;

    // then we encode the tree in no particular order (using a dummy string for the root prefix)
    writeHuffmanTreeNodesToFile(_fileToWrite, _tree -> root, "", 0, &nodesSaved);

    // now update the byte stating amount of nodes
    const int finalPosition = ftell(_fileToWrite);
    fseek(_fileToWrite, reservedByteLocation, SEEK_SET);
    fputc(nodesSaved, _fileToWrite);
    fseek(_fileToWrite, finalPosition, SEEK_SET);
}

static void writeEncodedWordsToFile(FILE* _wordsToEncode, FILE* _fileToWrite, struct huffmanTree* _tree, char* _alphabet, int _alphabetLength) {
    // generate the encodings from the tree and ready the bit buffer
    const char** codes = getEncodedAlphabet(_tree, _alphabet, _alphabetLength);
    struct bitBuffer* const bitBuffer = newBitBuffer();

    // reserve a byte to store how many bits of the final byte are relevant
    int finalBitCountBytePosition = ftell(_fileToWrite);
    fputc('\0', _fileToWrite);

    unsigned int wordPoolSize = 0; // this will increase for each '\n' encoded

    char characterToEncode;
    char* characterToEncodeAlphabetPointer;
    int characterToEncodeAlphabetIndex;
    int charactersEncodedSinceLastFlush = 0;
    // encode character by character, first into the bit buffer then into the file
    while (true) {
        characterToEncode = fgetc(_wordsToEncode);

        if (characterToEncode == '\n') {
            wordPoolSize++;
        }

        if (characterToEncode == EOF) {
            // perform a final flush until less than a byte remains
            while (bitBuffer -> size >= 8) {
                unsigned char flushedByte = flushSingleByte(bitBuffer);
                fputc(flushedByte, _fileToWrite);
            }
            break;
        }

        characterToEncodeAlphabetPointer = strchr(_alphabet, characterToEncode);
        if (!characterToEncodeAlphabetPointer) {
            printf("Error: the word file contains the character %c which is not part of the alphabet provided: ", characterToEncode);
            printFromCharArray(_alphabet, _alphabetLength);
            printf("\n");
            exit(EXIT_FAILURE);
        }

        characterToEncodeAlphabetIndex = characterToEncodeAlphabetPointer - _alphabet;

        const char* encoding = codes[characterToEncodeAlphabetIndex];
        int i = 0;
        char currentEncodingChar;
        while (true) {
            currentEncodingChar = encoding[i];
            if (currentEncodingChar == '\0') {
                break;
            }
            if (currentEncodingChar == '0') {
                addBit(bitBuffer, false);
            } else {
                addBit(bitBuffer, true);
            }
            i++;
        }

        // after a specific amount of characters have been encoded, flush the buffer as long as possible without breaking whole bytes
        charactersEncodedSinceLastFlush++;
        if (charactersEncodedSinceLastFlush >= CHARACTERS_TO_ENCODE_PER_FLUSH) {
            while (bitBuffer -> size >= 8) {
                unsigned char flushedByte = flushSingleByte(bitBuffer);
                fputc(flushedByte, _fileToWrite);
            }
            charactersEncodedSinceLastFlush = 0;
        }

    }

    // at this point the final complete byte has already been flushed, so we can write the final buffer size and word pool size to the reserved bytes
    int currentWritingPoint = ftell(_fileToWrite);

    fseek(_fileToWrite, finalBitCountBytePosition, SEEK_SET);
    fputc(bitBuffer -> size, _fileToWrite);

    // in 4 bytes, write the size of the word pool.
    fseek(_fileToWrite, 0, SEEK_SET);
    unsigned int remainingSizeToWrite = wordPoolSize; // this value can not be negative
    // we write from back to front
    for (int i = 0; i < 4; i++) {
        fseek(_fileToWrite, 3 - i, SEEK_SET);
        fputc(remainingSizeToWrite % 256, _fileToWrite);
        remainingSizeToWrite /= 256;
    }
    
    fseek(_fileToWrite, currentWritingPoint, SEEK_SET);
    fputc(flushSingleByte(bitBuffer), _fileToWrite);
    
    free(codes);
    free(bitBuffer);
}

void buildWordPoolFile(FILE* _source, FILE* _target, struct huffmanTree* _tree, char* _alphabet, int _alphabetLength, bool verbose) {
    // the first 4 bytes will store how many words are in the data pool
    for (int i = 0; i < 4; i++) {
        fputc('\0', _target);
    }

    printIfVerbose(verbose, "Writing huffman tree to file...\n");
    writeHuffmanTreeToFile(_target, _tree);

    printIfVerbose(verbose, "Writing encoded words to file...\n");
    writeEncodedWordsToFile(_source, _target, _tree, _alphabet, _alphabetLength);
}

static void extractAndAddHuffmanTreeNodeFromFile(FILE* _file, struct huffmanTree* _tree) {
    char character = assertedFGetC(_file);
    char prefixLength = assertedFGetC(_file);
    char* prefix = saferMalloc(sizeof(char) * prefixLength, "huffman tree prefix");
    fread(prefix, sizeof(char), prefixLength, _file);
    addEncodedNodeToHuffmanTree(_tree, character, prefix, prefixLength);
    free(prefix);
}

static struct huffmanTree* reconstructHuffmanTreeFromFile(FILE* _file) {
    int nodesToRead = assertedFGetC(_file);
    struct huffmanTree* toReturn = getEmptyRootHuffmanTree();

    int nodesExtracted = 0;
    while (true) {
        if (nodesExtracted == nodesToRead) {
            break;
        } else {
            extractAndAddHuffmanTreeNodeFromFile(_file, toReturn);
            nodesExtracted++;
        }
    }

    return toReturn;
}

void restoreRawWordList(FILE* source, FILE* target, char* _alphabet, int _alphabetLength, bool verbose) {
    // skip the first 4 bytes, which represent the size of the word pool
    fseek(source, 4, SEEK_SET);

    printIfVerbose(verbose, "Reconstructing  huffman tree from file...\n");
    struct huffmanTree* tree = reconstructHuffmanTreeFromFile(source);
    const char** huffmanCodes = getEncodedAlphabet(tree, _alphabet, _alphabetLength);

    int longestCode = getLongestHuffmanCodeLength(huffmanCodes, _alphabetLength);

    printIfVerbose(verbose, "Decoding the binary's word pool...\n");

    // the next byte represents the amount of bits in the last byte that are of relevance
    char finalBitCount = assertedFGetC(source);

    // read bytes from the encoded word stream. the last byte has a special meaning, so we need to read "a byte ahead" to see the EOF in time
    int currentChar = 0;
    int nextChar = fgetc(source);

    // special case: the first character read is EOF
    if (nextChar == EOF) {
        printIfVerbose(verbose, "The binary's word pool is empty.");
    } else {
        currentChar = nextChar;
        nextChar = fgetc(source);
    }

    struct bitBuffer* bitBuffer = newBitBuffer();
    int bytesReadSinceLastFlush = 0;
    while (true) {
        // check whether we have reached the final byte
        if (nextChar == EOF) {
            // of the final byte, we only need the finalBitCount most significant byte
            for (int i = 0; i < finalBitCount; i++) {
                addBit(bitBuffer, currentChar >= 128);
                currentChar = (currentChar << 1) % 256; // reduce it to 8 bits
            }

            // do the final flush of the bitbuffer
            while (true) {

                if (!bitBuffer -> size) {
                    break;
                }
                int flushedCharacterIndex = flushEncodedCharacter(bitBuffer, huffmanCodes, _alphabetLength);
                if (flushedCharacterIndex == -1) {
                    printf("Error: the word pool in the binary is malformed.\n");
                    exit(EXIT_FAILURE);
                }
                fputc(_alphabet[flushedCharacterIndex], target);
            }
            break;
        }
        
        // add the current character into the buffer
        addByte(bitBuffer, currentChar);
        bytesReadSinceLastFlush++;

        // flush the bit buffer if necessary
        if (bytesReadSinceLastFlush >= BYTES_TO_READ_PER_FLUSH) {
            // decode characters from the buffer as long as a complete encoded character is guaranteed to be there (else the encoding must somehow be corrupted)
            // this is the case if the bit buffer is at least as long as the longest encoding
            while (bitBuffer -> size >= longestCode) {
                int indexOfEncodedChar = flushEncodedCharacter(bitBuffer, huffmanCodes, _alphabetLength);
                if (indexOfEncodedChar == -1) {
                    printf("Error: the word pool in the binary is malformed.\n");
                    exit(EXIT_FAILURE);
                }
                fputc(_alphabet[indexOfEncodedChar], target);
            }
            bytesReadSinceLastFlush = 0;
        }

        // prepare the next iteration
        currentChar = nextChar;
        nextChar = fgetc(source);
    }

    free(huffmanCodes);
    free(bitBuffer);
    free(tree);
}

unsigned int getWordPoolSize(FILE* _source) {
    unsigned int toReturn;
    int currentFileLocation = ftell(_source);
    // read the first 4 bytes as one integer
    fseek(_source, 0, SEEK_SET);
    for (int i = 0; i < 4; i++) {
        toReturn *= 256;
        toReturn += assertedFGetC(_source);
    }
    fseek(_source, currentFileLocation, SEEK_SET);
    return toReturn;
}

struct translatedSeedList* translateSeedListWithWordPool(
    FILE* source,
    const struct seedsToFind* _seedsToFind,
    char* _alphabet,
    int _alphabetLength,
    bool verbose
) {
    // the resulting list is guaranteed to be sorted by seed, since the seeds are translated in order
    struct translatedSeedList* toReturn = newTranslatedSeedList();
    int currentSeed = 0;
    int* relevantSeeds = _seedsToFind -> sortedArray;
    int relevantSeedsAmount = _seedsToFind -> amount;
    int nextRelevantSeedIndex = 0;
    struct stringBuilder* currentTranslatedSeedStringBuilder; // initialized once seed is relevant
    bool isCurrentSeedRelevant;
    if (relevantSeeds[nextRelevantSeedIndex] == currentSeed) {
        isCurrentSeedRelevant = true;
        if (nextRelevantSeedIndex < relevantSeedsAmount - 1) {
            nextRelevantSeedIndex++;
        }
        currentTranslatedSeedStringBuilder = newStringBuilder();
    } else {
        isCurrentSeedRelevant = false;
        currentTranslatedSeedStringBuilder = NULL;
    }

    // skip the first 4 bytes, which represent the size of the word pool
    fseek(source, 4, SEEK_SET);
    
    // the approach is the same as when restoring a word pool
    printIfVerbose(verbose, "Reconstructing  huffman tree from file...\n");
    struct huffmanTree* tree = reconstructHuffmanTreeFromFile(source);
    const char** huffmanCodes = getEncodedAlphabet(tree, _alphabet, _alphabetLength);

    // we need to be able to recognize the newline character
    int longestCode = getLongestHuffmanCodeLength(huffmanCodes, _alphabetLength);
    char* newLineCharacterAlphabetPointer = strchr(_alphabet, '\n');
    if (!newLineCharacterAlphabetPointer) {
        printf("The word file does not contain a newline character.");
        exit(EXIT_FAILURE);
    }
    int newLineCharacterAlphabetIndex = newLineCharacterAlphabetPointer - _alphabet;

    printIfVerbose(verbose, "Decoding the binary's word pool...\n");

    // the next byte represents the amount of bits in the last byte that are of relevance
    char finalBitCount = assertedFGetC(source);

    // read bytes from the encoded word stream. the last byte has a special meaning, so we need to read "a byte ahead" to see the EOF in time
    int currentChar = 0;
    int nextChar = fgetc(source);

    // special case: the first character read is EOF
    if (nextChar == EOF) {
        printIfVerbose(verbose, "The binary's word pool is empty.");
    } else {
        currentChar = nextChar;
        nextChar = fgetc(source);
    }

    struct bitBuffer* bitBuffer = newBitBuffer();
    int bytesReadSinceLastFlush = 0;
    while (true) {

        // check whether we have reached the final byte
        if (nextChar == EOF) {
            // of the final byte, we only need the finalBitCount most significant byte
            for (int i = 0; i < finalBitCount; i++) {
                addBit(bitBuffer, currentChar >= 128);
                currentChar = (currentChar << 1) % 256; // reduce it to 8 bits
            }

            // do the final flush of the bitbuffer
            while (true) {

                if (!bitBuffer -> size) {
                    break;
                }
                int flushedCharacterIndex = flushEncodedCharacter(bitBuffer, huffmanCodes, _alphabetLength);
                if (flushedCharacterIndex == -1) {
                    printf("Error: the word pool in the binary is malformed.\n");
                    exit(EXIT_FAILURE);
                }
                if (flushedCharacterIndex == newLineCharacterAlphabetIndex) {
                    // finalize the previous seed translation
                    if (currentTranslatedSeedStringBuilder) {
                        char* translatedSeed = finalizeStringBuilder(currentTranslatedSeedStringBuilder);
                        appendToTranslatedSeedList(toReturn, currentSeed, translatedSeed);
                    }

                    // increase seed and prepare next string builder if current seed is relevant
                    currentSeed++;

                    if (nextRelevantSeedIndex >= relevantSeedsAmount) {
                        // we are finished and can clean up
                        free(huffmanCodes);
                        free(bitBuffer);
                        free(tree);
                        return toReturn;
                    }

                    if (relevantSeeds[nextRelevantSeedIndex] == currentSeed) {
                        isCurrentSeedRelevant = true;
                        nextRelevantSeedIndex++;
                        currentTranslatedSeedStringBuilder = newStringBuilder();
                    } else {
                        isCurrentSeedRelevant = false;
                        currentTranslatedSeedStringBuilder = NULL;
                    }
                } else if (isCurrentSeedRelevant) {
                    appendCharToStringBuilder(currentTranslatedSeedStringBuilder, _alphabet[flushedCharacterIndex]);
                }
            }
            break;
        }
        
        // add the current character into the buffer
        addByte(bitBuffer, currentChar);
        bytesReadSinceLastFlush++;

        // flush the bit buffer if necessary
        if (bytesReadSinceLastFlush >= BYTES_TO_READ_PER_FLUSH) {
            // decode characters from the buffer as long as a complete encoded character is guaranteed to be there (else the encoding must somehow be corrupted)
            // this is the case if the bit buffer is at least as long as the longest encoding
            while (bitBuffer -> size >= longestCode) {
                int indexOfEncodedChar = flushEncodedCharacter(bitBuffer, huffmanCodes, _alphabetLength);
                if (indexOfEncodedChar == -1) {
                    printf("Error: the word pool in the binary is malformed.\n");
                    exit(EXIT_FAILURE);
                }
                if (indexOfEncodedChar == newLineCharacterAlphabetIndex) {
                    // finalize the previous seed translation
                    if (currentTranslatedSeedStringBuilder) {
                        char* translatedSeed = finalizeStringBuilder(currentTranslatedSeedStringBuilder);
                        appendToTranslatedSeedList(toReturn, currentSeed, translatedSeed);
                    }

                    // increase seed and prepare next string builder if current seed is relevant
                    currentSeed++;

                    if (nextRelevantSeedIndex >= relevantSeedsAmount) {
                        // we are finished and can clean up
                        free(huffmanCodes);
                        free(bitBuffer);
                        free(tree);
                        return toReturn;
                    }
                    
                    if (relevantSeeds[nextRelevantSeedIndex] == currentSeed) {
                        isCurrentSeedRelevant = true;
                        nextRelevantSeedIndex++;
                        currentTranslatedSeedStringBuilder = newStringBuilder();
                    } else {
                        isCurrentSeedRelevant = false;
                        currentTranslatedSeedStringBuilder = NULL;
                    }
                } else if (isCurrentSeedRelevant) {
                    appendCharToStringBuilder(currentTranslatedSeedStringBuilder, _alphabet[indexOfEncodedChar]);
                }
            }
            bytesReadSinceLastFlush = 0;
        }

        // prepare the next iteration
        currentChar = nextChar;
        nextChar = fgetc(source);
    }

    free(huffmanCodes);
    free(bitBuffer);
    free(tree);

    return toReturn;
}

void writeTranslatedSeedsToFile(
    FILE* target,
    const int** _seeds,
    const struct amount* _amount,
    const struct translatedSeedList* _translatedSeedList,
    const char* seperator,
    bool verbose
) {
    for (int currentLine = 0; currentLine < _amount -> amoutOfPasswords; currentLine++) {
        for (int currentWord = 0; currentWord < _amount -> wordsPerPassword; currentWord++) {
            if (currentWord) {
                fprintf(target, "%s", seperator);
            }
            const int seedToTranslate = _seeds[currentLine][currentWord];
            const char* translation = getSeedTranslation(seedToTranslate, _translatedSeedList);
            if (!translation) {
                printf("Error: Could not translate seed using the binary. It is probably incomplete or corrupted.\n");
                exit(EXIT_FAILURE);
            }
            fprintf(target, "%s", getSeedTranslation(_seeds[currentLine][currentWord], _translatedSeedList));
        }
        fputc('\n', target);
    }
}