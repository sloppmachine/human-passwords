// creates the bin file that contains the word pool from which words are pulled
void createWordPoolFile(FILE* _source, FILE* _target, struct huffmanTree* _tree);

// the seeds act like the indices of the words pulled.
struct wordList* extractFromWordPool(char* _fileName, char* _alphabet, int _alphabetSize, int* _seeds, int _seedsLength);