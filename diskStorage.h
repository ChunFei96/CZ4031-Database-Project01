#ifndef DISK_STORAGE_H
#define DISK_STORAGE_H

typedef unsigned int uint;
typedef unsigned char uchar;
const int SIZE = 12;

struct Record
{
    /*
     * Size of each record = 20 bytes
     */
    char tconst[SIZE];   // 12
    float averageRating; // 4
    uint numVotes;       // 4
};

struct Location
{
    uchar *blockLocation; // pointer
    uint offset;          //
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
    uint currentBlockSizeUsed;
    uchar *blockPtr;
    int numBlockUsed;
    int numBlockAvailable;
    uint totalRecordSize;

public:
    diskStorage(uint diskStorageSize, uint blockSize);

    //~diskStorage();

    Location insertRecord(uint sizeOfRecord);

    void deleteRecord(Location location);

    uint getBlockSize();

    uint getTotalRecordSize();

    int getNumOfBlockUsed();
};
#endif
