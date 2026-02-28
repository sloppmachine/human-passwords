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
struct huffmanTree* buildHuffmanTreeFromDistribution(char* _alphabet, int _alphabetSize, int* _distribution);

// prints the huffmn codes that a huffman tree symbolizes.
void printHuffmanCodes(struct huffmanTree* _tree);