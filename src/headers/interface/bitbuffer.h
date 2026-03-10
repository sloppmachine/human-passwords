// a structure to buffer single bits that can be flushed out as complete bytes, or as huffman codes
struct bitBuffer {
    struct bitBufferElement* first;
};

struct bitBufferElement {
    bool content;
    struct bitBufferElement* next;
};

struct bitBuffer* newBitBuffer();

void freeBitBuffer(struct bitBuffer* _bitBuffer);

// this is the current amount of bits buffered
int getBitBufferSize(struct bitBuffer* _bitBuffer);

// this runs in linear time. this structure isn't meant to hold more than some bytes at once
void addBit(struct bitBuffer* _bitBuffer, bool _bit);

// see addBit.
void addByte(struct bitBuffer* _bitBuffer, unsigned char _byte);

// if there are less than 8 bits in the buffer, the byte is filled with 0 from the unsignificant side
unsigned char flushSingleByte(struct bitBuffer* _bitBuffer);

// flushes out the index of the encoded character in the beginning of the bit buffer, otherwise -1
int flushEncodedCharacter(struct bitBuffer* _bitBuffer, char** _encodings, int _alphabetLength);