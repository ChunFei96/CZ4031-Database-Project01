#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H

typedef unsigned int uint;
typedef unsigned char uchar;
const int SIZE = 11;

struct Record
{
    /*
     * Size of each record = 20 bytes
     */
    char tconst[SIZE];
    float averageRating;
    uint numVotes;
};

struct Location
{
    uchar *blockLocation;
    uint offset;
};

struct LLNode
{
    Location location;
    int size;
    struct LLNode *next;
};

class bufferPool
{
private:
    uint bufferPoolSize;
    uchar *bufferPoolPtr;
    uint blockSize;
    uint usedBlockSize;
    uint currentBlockSizeUsed;
    uchar *blockPtr;
    int numBlockAlloc;
    int numBlockAvail;
    uint totalRecordSize;

public:
    bufferPool(uint bufferPoolSize, uint blockSize);

    //~bufferPool();

    Location insertRecord(uint sizeOfRecord);

    void deleteRecord(Location location);

    uint getBlockSize();

    uint getTotalRecordSize();

    int getNumOfBlockAlloc();
};
#endif
