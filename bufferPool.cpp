#include <iostream>
#include "bufferPool.h"

using namespace std;

bufferPool::bufferPool(uint bufferPoolSize, uint blockSize)
{
    this->bufferPoolSize = bufferPoolSize;
    uchar *bufferPoolPtr = nullptr;
    this->bufferPoolPtr = new uchar[bufferPoolSize];
    this->blockSize = blockSize;
    this->usedBlockSize = 0;
    this->currentBlockSizeUsed = 0;
    this->blockPtr = nullptr;
    this->numBlockAlloc = 0;
    this->numBlockAvail = bufferPoolSize / blockSize;
    this->totalRecordSize = 0;
}

Location bufferPool::insertRecord(uint sizeOfRecord)
{
    if (sizeOfRecord > blockSize)
    {
        cout << "ERROR! - fail to allocate space: size of record is greater than block size.\n";
        throw "ERROR! - fail to allocate space: size of record is greater than block size.\n";
    }
    else if (blockSize < (currentBlockSizeUsed + sizeOfRecord) or numBlockAlloc == 0)
    {
        if (numBlockAvail > 0)
        {
            blockPtr = bufferPoolPtr + (numBlockAlloc * blockSize);
            usedBlockSize += blockSize;
            numBlockAlloc += 1;
            numBlockAvail -= 1;
            currentBlockSizeUsed = 0; // New block assigned with 0 record inside
        }
        else
        {
            cout << "ERROR! - fail to allocate space: database is full.\n";
            throw "ERROR! - fail to allocate space: database is full.\n";
        }
    }
    totalRecordSize += sizeOfRecord;
    currentBlockSizeUsed += sizeOfRecord;

    Location recordAddress{blockPtr, currentBlockSizeUsed};

    return recordAddress;
}

void bufferPool::deleteRecord(Location location)
{
    int result;
    try
    {
        // to delete a record, we can change the values stored in that record to NULL
        // traverse from the beginning of the record location to the end based on the size of the record
        // fill the elements to the null character
        totalRecordSize -= 20;
        void *recordAddress = (uchar *)location.blockLocation + location.offset;
        fill((uchar *)location.blockLocation + location.offset, (uchar *)location.blockLocation + location.offset + 20, '\0');

        // Block is empty, remove size of block.
        uchar cmpBlk[blockSize];
        fill(cmpBlk, cmpBlk + 20, '\0');
        result = equal(cmpBlk, cmpBlk + 20, location.blockLocation);

        if (result == true)
        {
            currentBlockSizeUsed -= blockSize;
            numBlockAlloc--;
            numBlockAvail++;
        }
    }

    catch (exception &e)
    {
        cout << "ERROR! - Exception deleteRecord: " << e.what() << "\n";
    }
}

uint bufferPool::getBlockSize()
{
    return blockSize;
}

uint bufferPool::getTotalRecordSize()
{
    return totalRecordSize;
}

int bufferPool::getNumOfBlockAlloc()
{
    return numBlockAlloc;
}
