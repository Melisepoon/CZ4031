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
    std::size_t blockSize;      // Size for each block in the b+ tree 200B/500B

    int height;                 // Height of the b+ tree
    int numOfNodes;             // Total number of nodes in the b+tree
    int maxKeys;                // Maximum number of keys in each node
    
    void insertInternal(int value, TreeNode *current, TreeNode *child);
    TreeNode *findParent(TreeNode *current, TreeNode *child);
    int findParentValue(TreeNode *current);
    void revomeInternal(int value, TreeNode *current, TreeNode *child);

public:

    BPlusTree(std::size_t blockSize, MemoryPool *disk, MemoryPool *index);

    TreeNode *getRoot(){return root;};
    
    int getMaxKeys(){return maxKeys;};
    
    int getHeight(){return height;};

    int getNumOfNodes(){return numOfNodes;};
    
    int calculateMaxKeys(std::size_t blockSize);

    void insert(int value);

    void remove(int value);

    void search(int leftValue, int righValue);

};


#endif //BPLUSTREE_H