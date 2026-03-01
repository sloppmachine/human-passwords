#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
static void writeEncodedWordsToFile(FILE* _wordsToEncode, FILE* _fileToWrite, struct huffmanTree* _tree);

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

static void writeEncodedWordsToFile(FILE* _wordsToEncode, FILE* _fileToWrite, struct huffmanTree* _tree) {
    printf("not yet implemented\n");
    exit(EXIT_FAILURE);
}

void createWordPoolFile(char* _fileNameToWrite, char* _wordFileName, struct huffmanTree* _tree) {
    printf("opening file\n");
    FILE* fileToWrite = fopen(_fileNameToWrite, "wb");
    assert(fileToWrite != NULL, "could not open the file."); // the !=NULL is for the compiler not giving a warning
    printf("attempting to write huffman tree\n");
    writeHuffmanTreeToFile(fileToWrite, _tree);
    fclose(fileToWrite);
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

struct wordList* extractFromWordPool(char* _fileName, int* _seeds, int _seedsLength) {
    printf("opening file\n");
    FILE* fileToRead = fopen(_fileName, "rb");
    assert(fileToRead != NULL, "could not open the file."); // the !=NULL is for the compiler not giving a warning

    printf("attempting to read the huffman tree from the file\n");
    struct huffmanTree* tree = reconstructHuffmanTreeFromFile(fileToRead);
    printHuffmanCodes(tree);

    freeHuffmanTree(tree);
    fclose(fileToRead);
}