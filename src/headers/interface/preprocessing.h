// these loosely connected methods are used mainly for preprocessing strings.

// returns true if all characters of the string are digits or it is empty
bool isStringDigit(char* _string);

void makeLowercase(char* _input);

// returns an int array that represents the occurance of all characters
int* getCharacterDistributionFromCharArray(char* _alphabet, int _alphabetSize, char* _input, int _inputSize);

// this is just the equivalent of getCharacterDistributionFromCharArray for a file.
int* getCharacterDistributionFromFile(char* _alphabet, int _alphabetSize, FILE* _input);

void printCharacterDistribution(int* _distribution, char* _alphabet, int _alphabetLength);