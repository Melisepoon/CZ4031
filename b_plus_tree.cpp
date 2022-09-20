#include <cstddef>
#include "b_plus_tree.h"

BPlusTree::BPlusTree(std::size_t blockSize, MemoryPool *disk, MemoryPool *index)
{
    this->blockSize = blockSize;
    this->disk = disk;
    this->index = index;
    this->maxKeys = BPlusTree::calculateMaxKeys(blockSize);
    this->root = NULL;
    this->height = 0;
    this->numOfNodes = 0;
    
};

int BPlusTree::calculateMaxKeys(std::size_t blockSize)
{
    int sum = sizeof(TreeNode*);
    int maxKeys = 0;
    while(sum + sizeof(int) + sizeof(TreeNode*) <= blockSize)
    {
        maxKeys ++;
        sum += sizeof(int) + sizeof(TreeNode*);
    }
    return maxKeys;
}

void insertInternal(int value, TreeNode *current, TreeNode *child)
{

};

TreeNode *findParent(TreeNode *current, TreeNode *child)
{

};

int findParentValue(TreeNode *current)
{

};

void revomeInternal(int value, TreeNode *current, TreeNode *child)
{
    
};

void BPlusTree::insert(int value)
{

};

void BPlusTree::remove(int value)
{

};

void BPlusTree::search(int leftValue, int rightValue)
{

};

