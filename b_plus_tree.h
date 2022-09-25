#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include <iostream>
#include <vector>
#include <cstddef>
#include "types.h"
#include "memory_pool.h"
#include "tree_node.h"

class BPlusTree
{
private:
    MemoryPool *disk;           // Pointer to the memory pool for the data
    MemoryPool *index;          // Pointer to the memory pool for the index created
    TreeNode *root;             // Pointer to the root of the b+ tree
    void *rootAddress;
    short int rootIndex;
    std::size_t blockSize;      // Size for each block in the b+ tree 200B/500B

    int height;                 // Height of the b+ tree
    int numOfNodes;             // Total number of nodes in the b+tree
    int maxKeys;                // Maximum number of keys in each node
    
    void insertInternal(int value, Address currentDiskAddress, Address childDiskAddress);
    void removeInternal(int value, Address currentDiskAddress, Address childDiskAddress);
    Address findParent(Address currentDiskAddress, Address childDiskAddress, int value);

public:

    BPlusTree(std::size_t blockSize, MemoryPool *disk, MemoryPool *index);

    TreeNode *getRoot(){return root;};

    void*getRootAddress(){return rootAddress;};

    short int getRootIndex(){return rootIndex;}
    
    int getMaxKeys(){return maxKeys;};

    int getHeight(){return height;};

    int getNumOfNodes(){return numOfNodes;};
    
    int calculateMaxKeys(std::size_t blockSize);

    void displayTree(TreeNode *current, int height);

    void displayNode(TreeNode *current);

    void displayBlock(void *blockAddress);

    void displayLL(Address address);

    void insert(Address recordAddress, int value);

    Address insertLL(Address LLHead, Address address, int value);

    void remove(int value);

    void search(int leftValue, int righValue);

    Address getFirstLeaf(Address current);

    void calculateHeight(Address current, int height);

};


#endif //BPLUSTREE_H