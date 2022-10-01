#include <iostream>
#include "diskStorage.h"

using namespace std;

diskStorage::diskStorage(uint diskStorageSize, uint blockSize)
{
    this->diskStorageSize = diskStorageSize;
    uchar *diskStoragePtr = nullptr;
    this->diskStoragePtr = new uchar[diskStorageSize];
    this->blockSize = blockSize;
    this->currentBlockSizeUsed = 0;
    this->blockPtr = nullptr;
    this->numBlockAlloc = 0;
    this->numBlockAvail = diskStorageSize / blockSize;
    this->totalRecordSize = 0;
}

Location diskStorage::insertRecord(uint sizeOfRecord)
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
            blockPtr = diskStoragePtr + (numBlockAlloc * blockSize);
            numBlockAlloc += 1;
            numBlockAvail -= 1;
            currentBlockSizeUsed = 0;
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

void diskStorage::deleteRecord(Location location)
{
    int result;
    try
    {
        totalRecordSize -= 20;
        void *recordAddress = (uchar *)location.blockLocation + location.offset;
        fill((uchar *)location.blockLocation + location.offset, (uchar *)location.blockLocation + location.offset + 20, '\0');

        uchar emptyBlk[blockSize];
        fill(emptyBlk, emptyBlk + 20, '\0');
        result = equal(emptyBlk, emptyBlk + 20, location.blockLocation);

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

uint diskStorage::getBlockSize()
{
    return blockSize;
}

uint diskStorage::getTotalRecordSize()
{
    return totalRecordSize;
}

int diskStorage::getNumOfBlockAlloc()
{
    return numBlockAlloc;
}
