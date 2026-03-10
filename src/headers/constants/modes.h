// modes are the basic functionalities for which the final product can be called
typedef int Mode;

// represents the case where no mode has been parsed
#define MODE_DEFAULT 0

// represents the case where you build a binary file from a raw readable file of words
#define MODE_BUILD 1

// represents the case where you restore the original raw readable file of words from a binary file
#define MODE_RESTORE 2

// represents the case where you generate passwords from the binary file
#define MODE_EXTRACT 3