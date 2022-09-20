#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H

#include <vector>
#include "types.h"

class MemoryPool
{
private:
    // Variables for memory pool
    void *poolPtr;                          // Pointer to the memory pool

    std::size_t poolSize;                   // Size of the Memory pool 200MB
    std::size_t blockSize;                  // Size for each block 200B/500B
    std::size_t totalBlockSizeUsed;         // Total Size of Blocks used in the memory pool
    std::size_t totalRecordSizeUsed;        // Total Size of Records used in a block

    int blocksAllocated;                    // Number of blocks allocated
    int blocksAccessed;                     // Number of blocks accessed

    // Variables for a block
    void *blockPtr;                         // Pointer to a block

    std::size_t blockSizeUsed;              // Size used in a block
    int maxRecords;                         // Maximum number of Records per block

public:
    /**
     * Constructor to initialize class MemoryPool with the given arguments
     * @param poolSize Total size of the memory pool allocated
     * @param blockSize The size of each block in the memory pool
     */
    MemoryPool(std::size_t poolSize, std::size_t blockSize);
    
    /**
     * Function to get the total size of the memory pool
     * @return Total size of this memory pool
     */
    std::size_t getPoolSize()
    {
        return poolSize;
    }

    /**
     * Function to get the block size of the memory pool
     * @return The block size of this memory pool
     */
    std::size_t getBlcokSize()
    {
        return blockSize;
    }

    /**
     * Function to get the total size of block used in the memory pool
     * @return The total size of block used in this memory pool
     */
    std::size_t getTotalBlockSizeUsed()
    {
        return totalBlockSizeUsed;
    }

    /**
     * Function to get the total size of record used in the memory pool
     * @return The total size of record used in this memory pool
     */
    std::size_t getTotalRecordSizeUsed()
    {
        return totalRecordSizeUsed;
    }

    /**
     * Function to get the size used in a block
     * @return The size used in this block
     */
    std::size_t getBlockSizeUsed()
    {
        return blockSizeUsed;
    }

    /**
     * Function to get the number of allocated blocks in the memory pool
     * @return The number of allocated blocks in this memory pool
     */
    int getBlocksAllocated()
    {
        return blocksAllocated;
    }

    /**
     * Function to set the number of allocated blocks in the memory pool
     * @param num The number of allocated blocks in this memory pool
     */
    void setBlocksAllocated(int num)
    {
        this->blocksAllocated = num;
    }

    /**
     * Function to set the number of blocks accessed in the memory pool
     * @param num The number of blocks accessed in this memory pool
     */
    void setBlocksAccessed(int num)
    {
        this->blocksAccessed = num;
    }

    /**
     * Function to get the maximum number of records per block
     * @return The maximum number of records per block
     */
    int getMaxRecords()
    {
        return maxRecords;
    }

    int calculateMaxRecords(std::size_t blockSize);

    bool allocateBlock();

    Address allocateRecord(std::size_t size);

    bool deallocateRecord(Address address, std::size_t size);

    void *loadFromDisk(Address address, std::size_t size);

    Address saveToDisk(void *itemAddress, std::size_t size);

    void saveToDisk(void *itemAddress, std::size_t size, Address address);

    ~MemoryPool();

};

#endif // MEMORYPOOL_H