// a structure to buffer single bits that can be flushed out as complete bytes
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

// if there are less than 8 bits in the buffer, the byte is filled with 0 from the unsignificant side
char flushSingleByte(struct bitBuffer* _bitBuffer);