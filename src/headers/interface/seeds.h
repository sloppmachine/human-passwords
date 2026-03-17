// seeds are numbers that determine the random contents of generates passphrases

// this struct holds the data given in the argument for --amount
struct amount {
    int amoutOfPasswords;
    int wordsPerPassword;
};

// this struct is used to quickly verify whether the current seed is one of interest. it is not useful for anything else since
// it uses a sorted and duplicate-free array and therefore contains less information than the original seed array
struct seedsToFind {
    int amount;
    int* sortedArray;
};

// a sorted list of seed values translated to words from a word pool
// sorting is only guaranteed because the seeds are translated in order
struct translatedSeedList {
    struct translatedSeedListEntry* first;
};

// this struct is not used by anything besides translatedSeedList
struct translatedSeedListEntry {
    int seed;
    const char* word;
    struct translatedSeedListEntry* next;
};

void freeSeedsToFind(struct seedsToFind* _seedsToFind);

struct translatedSeedList* newTranslatedSeedList();

void freeTranslatedSeedList(struct translatedSeedList* _list);

void appendToTranslatedSeedList(struct translatedSeedList* _list, int _seed, char* _word);

// perform sequential search. returns a null pointer if the translation is not known
const char* getSeedTranslation(int _seedToTranslate, const struct translatedSeedList* _list);

// generate array of arrays with ints, used for extraction from the word pool, guaranteed to be in the wordPoolSize and without modulo bias
const int** generateSeedArray(const struct amount* _amount, unsigned int _wordPoolSize);

// frees the 2d array allocated by generateSeedArray
void freeSeedArray(const int** _seedArray, int _amountOfPasswords);

// generates a seedsToFind struct from a seed array
struct seedsToFind* getSeedsToFind(const int** _seeds, const struct amount* _amount);
