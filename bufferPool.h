#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H

typedef unsigned int uint;
typedef unsigned char uchar;
const int SIZE = 10;

struct Record
{
    /*
     * Size of each record = 18 bytes
     */
    char tconst[SIZE];
    float avgRating;
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
    uint blkSize;
    uint usedBlkSize;
    uint totalRecordSize;
    uint currentBlkSizeUsed;

    uchar *bufferPoolPtr;
    uchar *blkPtr;

    int numBlkAlloc;
    int numBlkAvail;

public:
    bufferPool(uint bufferPoolSize, uint blkSize);

    //~bufferPool();

    Location insertRecord(uint sizeOfRecord);

    bool isBlockAvailable();

    void deleteRecord(Location location);

    uint getBlkSize();

    uint getTotalRecordSize();

    int getNumOfBlkAlloc();
};
#endif
