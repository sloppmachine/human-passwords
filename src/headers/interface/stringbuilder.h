// a string builder is used to construct a short string from a variable amount of characters

// builds a string from a (possibly unknown) amount of chars. linear runtime, not meant for big strings
struct stringBuilder {
    struct stringBuilderChar* first;
};

// not used by anything besides stringBuilder
struct stringBuilderChar {
    char content;
    struct stringBuilderChar* next;
};

struct stringBuilder* newStringBuilder();

void appendCharToStringBuilder(struct stringBuilder* _stringBuilder, char character);

// returns the string of all characters in the string builder, then frees the string builder. remember the return value must be freed at some point
char* finalizeStringBuilder(struct stringBuilder* _stringBuilder);