#ifndef DISK_STORAGE_H
#define DISK_STORAGE_H

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

class diskStorage
{
private:
    uint diskStorageSize;
    uchar *diskStoragePtr;
    uint blockSize;
    uint usedBlockSize;
    uint currentBlockSizeUsed;
    uchar *blockPtr;
    int numBlockAlloc;
    int numBlockAvail;
    uint totalRecordSize;

public:
    diskStorage(uint diskStorageSize, uint blockSize);

    //~diskStorage();

    Location insertRecord(uint sizeOfRecord);

    void deleteRecord(Location location);

    uint getBlockSize();

    uint getTotalRecordSize();

    int getNumOfBlockAlloc();
};
#endif
