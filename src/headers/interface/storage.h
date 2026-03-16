// storage.h and storage.c contain the functions that directly write to or read from the local file system.

// exits if the source file couldn't be opened, implements a verbose option
FILE* openSourceFile(bool _verbose, char* _fileName);

// exits if the target file couldn't be opened/created, implements a verbose option
FILE* openTargetFile(bool _verbose, char* _fileName);

// creates the bin file that contains the word pool from which words are pulled
void buildWordPoolFile(FILE* _source, FILE* _target, struct huffmanTree* _tree, char* _alphabet, int _alphabetLength, bool verbose);

// recreates the original file from the bin file
void restoreRawWordList(FILE* source, FILE* target, char* _alphabet, int _alphabetLength, bool verbose);

// gets the amount of words stored in a binary
unsigned int getWordPoolSize(FILE* _source);

// reads the file and returns a list of the translated seeds. the seeds act as the indexes of words in the pool
struct translatedSeedList* translateSeedListWithWordPool(
    FILE* source,
    struct seedsToFind* _seedsToFind,
    char* _alphabet,
    int _alphabetLength,
    bool verbose
);

// writes lines of passwords
void writeTranslatedSeedsToFile(
    FILE* target,
    int** _seeds,
    struct amount* _amount,
    struct translatedSeedList* _translatedSeedList,
    char* seperator,
    bool verbose
);