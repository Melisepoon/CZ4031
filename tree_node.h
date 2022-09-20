#ifndef TREENODE_H
#define TREENODE_H

#include <cstddef>
#include "types.h"


class TreeNode
{
private:
    Address *diskAddress;
    int *keys;                  // Pointer to an array of keys in the node
    TreeNode **pointers;        // Pointer points to the other treenodes
    int numOfKeys;
    bool isLeaf;

public:
    TreeNode(int maxKeys)
    {
        keys = new int[maxKeys];
        pointers = new TreeNode *[maxKeys + 1];
        for (int i = 0; i < maxKeys; i++)
        {
            this->pointers[i] = NULL;
        }
    }

    int getKey(int index){return keys[index];};
    TreeNode *getPointer(int index){return pointers[index];};
    int getNumOfKeys(){return numOfKeys;};
    bool getIsLeaf(){return isLeaf;};
    Address *getDiskAddress(){return diskAddress;};

    void setKey(int index, int value){keys[index] = value;};
    void setPointer(int index, TreeNode *pointer){pointers[index] = pointer;};
    void setNumOfKeys(int numOfKeys){this->numOfKeys = numOfKeys;};
    void setLeaf(bool isLeaf){this->isLeaf = isLeaf;};

};

#endif //TREENODE_H