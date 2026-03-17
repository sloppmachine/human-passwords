// this file holds some text that will be printed in the command line as a response to executing the program

#define DEFAULT_TEXT "(WIP) This is the default text. use -h or --help\n"

#define HELP_TEXT_DEFAULT_MODE "This is a tool by slopmachine: https://github.com/sloppmachine/human-passwords \n\
Motivation:\n\
    The tool generates passphrases from a given pool of words.\n\
General syntax:\n\
    human-passwords <up to one mode> <options>\n\
Modes:\n\
    build               Create a compressed binary from a plain text word pool, where each word is separated by newline characters.\n\
                        required options: --source, --target\n\
    restore             Restore a plain text word pool from a compressed binary.\n\
                        required options: --source, --target\n\
    extract             Extract an amount of passwords from a compressed binary.\n\
                        required options: --source, --target, --amount\n\
General options:\n\
    --help          -h      Prints general help or help concerning any specific mode,\n\
    --verbose       -v      Prints what the program is currently doing.\n\
\n"
#define HELP_TEXT_BUILD_MODE "The build mode is used to turn a plain text word pool (words being separated by newline characters) into a compressed binary file, which can then be used to extract passphrases using the extract mode.\n\
General syntax:\n\
    human-passwords build <options>\n\
Required options:\n\
    --source=       -s=     The plain text file to read from.\n\
    --target=       -t=     The location to write the output.\n\
\n"
#define HELP_TEXT_RESTORE_MODE "The restore mode is used to restore the original plain text word pool from a compressed binary that was made using the build mode.\n\
General syntax:\n\
    human-passwords restore <options>\n\
Required options:\n\
    --source=       -s=     The plain text file to read from.\n\
    --target=       -t=     The location to write the output.\n\
\n"
#define HELP_TEXT_EXTRACT_MODE "The extract mode is used to extract an amount of passphrases from the word pool represented by a compressed binary produced by the build mode.\n\
General syntax:\n\
    human-passwords extract <options>\n\
Required options:\n\
    --source=       -s=     The plain text file to read from.\n\
    --target=       -t=     The location to write the output.\n\
    --amount=       -a=     The format of passphrases to be extracted. It follows the shape <amount of passphrases>x<words per passphrase>\n\
Optional options:\n\
    --seperator=    -p=     The series of characters placed betweens words of a passphrase.\n\
\n"

// errors
#define UNKNOWN_MODE_TEXT "Error: Unknown mode. \n"
#define TOO_MANY_MODES_TEXT "Error: You have specified to many modes. \n"
#define MALFORMED_AMOUNT_ARGUMENT_TEXT "Error: malformed argument to --amount. The argument must follow the form: <amount of passphrases>x<words per passphrase>, for example like this: --argument=100x10.\n"
#define COULD_NOT_READ_FILE_TEXT "Error: could not read from file. It might be corruped or you might be lacking permissions.\n"