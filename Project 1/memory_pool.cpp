//
//  Created by Huang NengQi
//  On 20/09/2022
//

#include <iostream>
#include <string>
#include <vector>
#include <cstddef>
#include <cstring>
#include "memory_pool.h"

MemoryPool::MemoryPool(std::size_t poolSize, std::size_t blockSize)
{
    this->poolPtr = operator new(poolSize);
    std::memset(poolPtr, '\0', poolSize);
    this->poolSize = poolSize;
    this->blockSize = blockSize;
    this->totalBlockSizeUsed = 0;
    this->totalRecordSizeUsed = 0;
    this->blocksAllocated = 0;
    this->blocksAccessed = 0;

    this->blockPtr = nullptr;
    this->blockSizeUsed = 0;
    this->maxRecords = MemoryPool::calculateMaxRecords(blockSize);
};

/**
 * Function to allocate a block in the memory pool
 * @return True if can allocate. False if cannot.
 */
bool MemoryPool::allocateBlock()
{
    if (totalBlockSizeUsed + blockSize <= poolSize)
    {
        totalBlockSizeUsed += blockSize;
        blockPtr = (char *)poolPtr + blocksAllocated * blockSize;
        blockSizeUsed = 0;
        blocksAllocated += 1;
        return true;
    }else
    {
        return false;
    }
};

/**
 * Function to allocate an space in a block
 * @return The Address of allocated
 */
Address MemoryPool::allocate(std::size_t size)
{
    if (size > blockSize)
    {
        std::cout << "Error: Request size is larger than block size" << std::endl;
        throw std::invalid_argument("requested size larger than block size");
    }
    if (blocksAllocated == 0 || (blockSizeUsed + size > blockSize))
    {
        if (!allocateBlock())
        {
            std::cout << "Error: Failed to allocate a block" << std::endl;
            throw std::invalid_argument("Failed to allocate a block");
        }
    }

    short int index = blockSizeUsed;

    blockSizeUsed += size;
    totalRecordSizeUsed += size;

    Address currentAddress = {blockPtr, index};
    return currentAddress;
}

/** Function to deallocated a Record and also a block if after
 * deallocated a Record the block is empty
 * @param address The address of record to be deallocated
 * @param size The size of the record
 * @return Ture if can deallocated. False if cannot.
 */
bool MemoryPool::deallocate(Address address, std::size_t size)
{
    try
    {
        void *addressToDelete = (char *)address.blockAddress + address.index;
        std::memset(addressToDelete, '\0', size);

        totalRecordSizeUsed -= size;

        // Check if the block is empty after delete the record
        //unsigned char emptyBlock[blockSize];
        //if (std::memcmp(address.blockAddress, emptyBlock, blockSize) == 0)
        if (address.index == 0)
        {
            totalBlockSizeUsed -= blockSize;
            blocksAllocated --;
        }

        return true;

    }
    catch(...)
    {
        std::cout << "Error: Could not remove record/block at the given address" << std::endl;
        throw std::invalid_argument("Coudl not remove record/block");
        return false;
    }
}

/** Function to load a record from disk to main memory
 * @return The pointer to the address of the record in the main memory
 */
void *MemoryPool::loadFromDisk(Address address, std::size_t size)
{
    void *mainMemoryAddress = operator new(size);
    std::memcpy(mainMemoryAddress, (char *)address.blockAddress + address.index, size);

    blocksAccessed ++;
    
    return mainMemoryAddress;
}

/** Function to save a record to the disk
 * @return The Address of the record in the disk
 */
Address MemoryPool::saveToDisk(void *itemAddress, std::size_t size)
{
    Address diskAddress = MemoryPool::allocate(size);
    std::memcpy((char *)diskAddress.blockAddress + diskAddress.index, itemAddress, size);

    blocksAccessed ++;

    return diskAddress;
}

// Function to update a record to the disk
Address MemoryPool::saveToDisk(void *itemAddress, std::size_t size, Address address)
{
    std::memcpy((char *)address.blockAddress + address.index, itemAddress, size);
    blocksAccessed++;

    return address;
}

/** Function to calculate the maximum number of records in a block
 * @return Number the maximum number of records in a block
 */
int MemoryPool::calculateMaxRecords(std::size_t blockSize)
{
    int sum = 0;
    int maxRecords = 0;
    while(sum + sizeof(Record) <= blockSize)
    {
        maxRecords ++;
        sum += sizeof(Record);
    }
    return maxRecords;
}

// Destructor
MemoryPool::~MemoryPool(){};
