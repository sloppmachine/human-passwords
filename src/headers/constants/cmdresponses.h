// this file holds some text that will be printed in the command line as a response to executing the program

#define DEFAULT_TEXT "(WIP) This is the default text. use -h or --help\n"

#define HELP_TEXT_DEFAULT_MODE "(WIP) This is where the help section will be\n\
Motivation:\n\
    (here a short explanation what the tool does)\n\
General syntax:\n\
    human-passwords <up to one mode> <options>\n\
Modes:\n\
    build               create binary from raw readable word list\n\
                        required options: --source, --target\n\
    restore             restore raw readable word list from binary\n\
                        required options: --source, --target\n\
    extract             extract a password from binary\n\
                        required options: --source, --target, --amount\n\
General options:\n\
    --help      -h      CONTINUEHERE\n\
    --verbose   -v      CONTINUEHERE\n\
\n"
#define HELP_TEXT_BUILD_MODE "(WIP) This is where the help section will be\n\
Motivation:\n\
    (here a short explanation what the build mode does)\n\
General syntax:\n\
    human-passwords build <options>\n\
Required options:\n\
    --source=   -s=     CONTINUEHERE\n\
    --target=   -t=     CONTINUEHERE\n\
\n"
#define HELP_TEXT_RESTORE_MODE "(WIP) This is where the help section will be\n\
Motivation:\n\
    (here a short explanation what the restore mode does)\n\
General syntax:\n\
    human-passwords restore <options>\n\
Required options:\n\
    --source=   -s=     CONTINUEHERE\n\
    --target=   -t=     CONTINUEHERE\n\
\n"

#define UNKNOWN_MODE_TEXT "(WIP) Error: Unknown mode. \n"
#define TOO_MANY_MODES_TEXT "(WIP) Error: You have specified to many modes. \n"