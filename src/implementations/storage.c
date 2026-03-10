#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <constants/storagehandling.h>

#include <interface/bitbuffer.h>
#include <interface/commons.h>
#include <interface/huffmantree.h>
#include <interface/storage.h>

// writes a byte for the character, then a byte for the length of the code, then 0-bytes or 1-bytes for the code.
static void writeHuffmanTreeNodeToFile(FILE* _fileToWrite, struct huffmanTreeNode* _node, char* _prefix, int _prefixlength);

// recursively calls writeHuffmanTreeNodeToFile, and updates the amount of nodes saved
static void writeHuffmanTreeNodesToFile(FILE* _fileToWrite, struct huffmanTreeNode* _node, char* _prefix, int _prefixLength, int* _nodesSaved);

// writes a byte documenting the length of the representation, followed by the single node representations in no necessary order.
static void writeHuffmanTreeToFile(FILE* _fileToWrite, struct huffmanTree* _tree);

// encodes words and writes them into a file
static void writeEncodedWordsToFile(FILE* _wordsToEncode, FILE* _fileToWrite, struct huffmanTree* _tree, char* _alphabet, int _alphabetSize);

// builds a node into a tree from its code
static void extractAndAddHuffmanTreeNodeFromFile(FILE* _file, struct huffmanTree* _tree);

// the file pointer should be at the byte where the amount of nodes is stored.
static struct huffmanTree* reconstructHuffmanTreeFromFile(FILE* _file);

static void writeHuffmanTreeNodeToFile(FILE* _fileToWrite, struct huffmanTreeNode* _node, char* _prefix, int _prefixLength) {
    printf("writing node for %c to file\n", _node -> content);
    fputc(_node -> content, _fileToWrite);
    fputc(_prefixLength, _fileToWrite); // this is actually guaranteed to fit into a single byte.
    fwrite(_prefix, sizeof(char), _prefixLength, _fileToWrite);
}

static void writeHuffmanTreeNodesToFile(FILE* _fileToWrite, struct huffmanTreeNode* _node, char* _prefix, int _prefixLength, int* _nodesSaved) {
    bool isInner = false;

    // check for children. technically both childs must simultaneously exist or not exist but i like this flow of code more
    if (_node -> leftChild) {
        isInner = true;
        char* leftChildPrefix = saferMalloc(sizeof(char) * _prefixLength + 1, "huffman tree prefix");
        memcpy(leftChildPrefix, _prefix, _prefixLength);
        leftChildPrefix[_prefixLength] = '0';
        writeHuffmanTreeNodesToFile(_fileToWrite, _node -> leftChild, leftChildPrefix, _prefixLength + 1, _nodesSaved);
        free(leftChildPrefix);
    }
    if (_node -> rightChild) {
        isInner = true;
        char* rightChildPrefix = saferMalloc(sizeof(char) * _prefixLength + 1, "huffman tree prefix");
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
    printf("writing first byte\n");
    fputc('\0', _fileToWrite);

    int nodesSaved = 0;

    // then we encode the tree in no particular order (using a dummy string for the root prefix)
    writeHuffmanTreeNodesToFile(_fileToWrite, _tree -> root, "", 0, &nodesSaved);

    printf("saved a total of %i nodes, now updating first byte\n", nodesSaved);
    // now update the first byte
    int finalPosition = ftell(_fileToWrite);
    fseek(_fileToWrite, 0, SEEK_SET);
    fputc(nodesSaved, _fileToWrite);
    fseek(_fileToWrite, finalPosition, SEEK_SET);
}

static void writeEncodedWordsToFile(FILE* _wordsToEncode, FILE* _fileToWrite, struct huffmanTree* _tree, char* _alphabet, int _alphabetLength) {
    // generate the encodings from the tree and ready the bit buffer
    char** codes = getEncodedAlphabet(_tree, _alphabet, _alphabetLength);
    struct bitBuffer* bitBuffer = newBitBuffer();

    // the first byte will store how many bits of the final byte were used
    int finalBitCountBytePosition = ftell(_fileToWrite);
    fputc('\0', _fileToWrite);

    char characterToEncode;
    char* characterToEncodeAlphabetPointer;
    int characterToEncodeAlphabetIndex;
    int charactersEncodedSinceLastFlush = 0;
    // encode character by character, first into the bit buffer then into the file
    while (true) {
        characterToEncode = fgetc(_wordsToEncode);

        if (characterToEncode == EOF) {
            break;
        }

        characterToEncodeAlphabetPointer = strchr(_alphabet, characterToEncode);
        if (!characterToEncodeAlphabetPointer) {
            printf("The word file contains the character %c which is not part of the alphabet provided: \n", characterToEncode);
            printFromCharArray(_alphabet, _alphabetLength);
            exit(EXIT_FAILURE);
        }

        characterToEncodeAlphabetIndex = characterToEncodeAlphabetPointer - _alphabet;

        char* encoding = codes[characterToEncodeAlphabetIndex];
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
            while (getBitBufferSize(bitBuffer) >= 8) {
                //printf("entering while loop\n");
                unsigned char flushedByte = flushSingleByte(bitBuffer);
                //printf("flushed this: %i\n", flushedByte);
                fflush(stdout);
                fputc(flushedByte, _fileToWrite);
            }
            charactersEncodedSinceLastFlush = 0;
        }

    }

    // at this point the final complete byte has already been flushed, so we can just directly write the buffer size to the reserved byte
    int currentWritingPoint = ftell(_fileToWrite);
    fseek(_fileToWrite, finalBitCountBytePosition, SEEK_SET);
    fputc(getBitBufferSize(bitBuffer), _fileToWrite);
    printf("remaining bit buffer size %i\n", getBitBufferSize(bitBuffer));
    fseek(_fileToWrite, currentWritingPoint, SEEK_SET);
    fputc(flushSingleByte(bitBuffer), _fileToWrite);
    
    free(codes);
    free(bitBuffer);
}

void buildWordPoolFile(FILE* _source, FILE* _target, struct huffmanTree* _tree, char* _alphabet, int _alphabetLength) {
    printf("attempting to write huffman tree\n");
    writeHuffmanTreeToFile(_target, _tree);

    printf("attempting to write encoded words\n");
    writeEncodedWordsToFile(_source, _target, _tree, _alphabet, _alphabetLength);
}

static void extractAndAddHuffmanTreeNodeFromFile(FILE* _file, struct huffmanTree* _tree) {
    char character = fgetc(_file);
    char prefixLength = fgetc(_file);
    char* prefix = saferMalloc(sizeof(char) * prefixLength, "huffman tree prefix");
    fread(prefix, sizeof(char), prefixLength, _file);
    addEncodedNodeToHuffmanTree(_tree, character, prefix, prefixLength);
    free(prefix);
}

static struct huffmanTree* reconstructHuffmanTreeFromFile(FILE* _file) {
    int nodesToRead = fgetc(_file);
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
    printIfVerbose(verbose, "Reconstructing  huffman tree from file...\n");
    struct huffmanTree* tree = reconstructHuffmanTreeFromFile(source);
    if (verbose) {
        printHuffmanCodes(tree, _alphabet, _alphabetLength);
    }
    char** huffmanCodes = getEncodedAlphabet(tree, _alphabet, _alphabetLength);

    // find the length of the longest code
    int longestCode = 0;
    for (int currentCodeIndex = 0; currentCodeIndex < _alphabetLength; currentCodeIndex++) {
        char* currentCode = huffmanCodes[currentCodeIndex];
        int currentIndexInCode = 0;
        while (true) {
            if (currentCode[currentIndexInCode] == '\0') {
                if (currentIndexInCode > longestCode) {
                    longestCode = currentIndexInCode;
                }
                break;
            } else {
                currentIndexInCode++;
            }
        }
    }

    printIfVerbose(verbose, "Decoding the binary's word pool...\n");

    // the next byte represents the amount of bits in the last byte that are of relevance
    char finalBitCount = fgetc(source);

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

                if (getBitBufferSize(bitBuffer) == 0) {
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
            while (getBitBufferSize(bitBuffer) >= longestCode) {
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

}


struct wordList* extractFromWordPool(FILE* source, char* _alphabet, int _alphabetLength, int* _seeds, int _seedsLength, bool verbose) {
    printIfVerbose(verbose, "Reconstructing  huffman tree from file...\n");
    struct huffmanTree* tree = reconstructHuffmanTreeFromFile(source);
    printHuffmanCodes(tree, _alphabet, _alphabetLength);

    printf("not yet implemented\n");
    exit(EXIT_FAILURE);

    freeHuffmanTree(tree);
}