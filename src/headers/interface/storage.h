// creates the bin file that contains the word pool from which words are pulled
void buildWordPoolFile(FILE* _source, FILE* _target, struct huffmanTree* _tree, char* _alphabet, int _alphabetLength);

// recreates the original file from the bin file
void restoreRawWordList(FILE* source, FILE* target, char* _alphabet, int _alphabetLength, bool verbose);

// the seeds act like the indices of the words pulled.
struct wordList* extractFromWordPool(FILE* source, char* _alphabet, int _alphabetSize, int* _seeds, int _seedsLength, bool verbose);