#include <iostream>
#include "bufferPool.h"

using namespace std;

bufferPool::bufferPool(uint bufferPoolSize, uint blkSize)
{
    this->bufferPoolSize = bufferPoolSize;
    this->blkSize = blkSize;
    this->usedBlkSize = 0;
    this->currentBlkSizeUsed = 0;
    this->totalRecordSize = 0;
    uchar *bufferPoolPtr = nullptr;
    this->bufferPoolPtr = new uchar[bufferPoolSize];
    this->blkPtr = nullptr;
    this->numBlkAlloc = 0;
    this->numBlkAvail = bufferPoolSize / blkSize;
}

Location bufferPool::insertRecord(uint sizeOfRecord)
{
    if (sizeOfRecord > blkSize)
    {
        cout << "Unable to allocate space for record as record size is greater than block size.\n";
        throw "Unable to allocate space for record as record size is greater than block size.\n";
    }
    else if (blkSize < (currentBlkSizeUsed + sizeOfRecord) or numBlkAlloc == 0)
    {
        if (!isBlockAvailable())
        {
            cout << "Unable to allocate space as buffer is full.\n";
            throw "Unable to allocate space as buffer is full.\n";
        }
    }
    totalRecordSize += sizeOfRecord;
    currentBlkSizeUsed += sizeOfRecord;

    Location recordAddress{blkPtr, currentBlkSizeUsed};

    return recordAddress;
}

bool bufferPool::isBlockAvailable()
{
    if (numBlkAvail > 0)
    {
        blkPtr = bufferPoolPtr + (numBlkAlloc * blkSize);
        usedBlkSize += blkSize;
        numBlkAlloc += 1;
        numBlkAvail -= 1;
        currentBlkSizeUsed = 0; // New block assigned with 0 record inside
        return true;
    }
    else
        return false;
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
        if (blkSize == 100)
        {
            uchar cmpBlk[100];
            fill(cmpBlk, cmpBlk + 20, '\0');
            result = equal(cmpBlk, cmpBlk + 20, location.blockLocation);
        }
        else
        {
            uchar cmpBlk[500];
            fill(cmpBlk, cmpBlk + 20, '\0');
            result = equal(cmpBlk, cmpBlk + 20, location.blockLocation);
        }

        if (result == true)
        {
            currentBlkSizeUsed -= blkSize;
            numBlkAlloc--;
            numBlkAvail++;
        }
    }

    catch (exception &e)
    {
        cout << "Exception" << e.what() << "\n";
        cout << "Delete record or block failed"
             << "\n";
    }
}

uint bufferPool::getBlkSize()
{
    return blkSize;
}

uint bufferPool::getTotalRecordSize()
{
    return totalRecordSize;
}

int bufferPool::getNumOfBlkAlloc()
{
    return numBlkAlloc;
}

int bufferPool::getNumOfBlkAvail()
{
    return numBlkAvail;
}