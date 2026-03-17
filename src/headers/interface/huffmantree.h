// huffman trees are a structure used to encode and compress textual data: https://en.wikipedia.org/wiki/Huffman_coding

struct huffmanTree {
    struct huffmanTreeNode* root;
    int totalWeight; // the total weight of all the nodes contained within, stored here again so that it does not need to be calculated over and over.
};

struct huffmanTreeNode {
    char content; // irrelevant if inner node
    int weight;
    struct huffmanTreeNode* leftChild;
    struct huffmanTreeNode* rightChild;
};

// frees a huffman tree, including its contents
void freeHuffmanTree(struct huffmanTree* _huffmanTree);

// builds a huffman tree from an alphabet and a distribution array, where the character alphabet[i] appeared distribution[i] times in the input text
struct huffmanTree* buildHuffmanTreeFromDistribution(const char* _alphabet, const int _alphabetSize, const int* _distribution);

// used when starting to reconstruct a huffman tree (in storage.c)
struct huffmanTree* getEmptyRootHuffmanTree();

// used when reading nodes from a file (in storage.c)
void addEncodedNodeToHuffmanTree(struct huffmanTree* _tree, char _content, char* _prefix, int _prefixLength);

// returns an array of strings of '0' and '1', where the index is alphabetical. NULL means no encoding was found.
const char** getEncodedAlphabet(struct huffmanTree* _tree, const char* _alphabet, int _alphabetLength);

// returns the length of the longest known huffman code
int getLongestHuffmanCodeLength(const char** _encodedAlphabet, int _alphabetLength);

// prints the huffman codes that a huffman tree symbolizes. every symbol in the huffman tree must also be in the given alphabet.
void printHuffmanCodes(struct huffmanTree* _tree, const char* _alphabet, int _alphabetLength);