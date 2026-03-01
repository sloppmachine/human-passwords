// creates the bin file that contains the word pool from which words are pulled
void createWordPoolFile(char* _fileNameToWrite, char* _wordFileName, struct huffmanTree* _tree);

// the seeds act like the indexes of the words pulled.
struct wordList* extractFromWordPool(char* _fileName, int* _seeds, int _seedsLength);