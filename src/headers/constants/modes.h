// modes are the basic functionalities for which the final product can be called#

enum Mode {
    DEFAULT, // represents the case where no mode has been parsed
    BUILD, // represents the case where you build a binary file from a raw readable file of words
    RESTORE, // represents the case where you restore the original raw readable file of words from a binary file
    EXTRACT, // represents the case where you generate passwords from the binary file
    STRENGTH // give information on size and strength of a word pool
};