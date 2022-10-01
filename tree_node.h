//
//  Created by Huang NengQi
//  On 20/09/2022
//

#ifndef TREENODE_H
#define TREENODE_H

#include <cstddef>
#include "types.h"


class TreeNode
{
private:
    Address *pointers;
    int *keys;                  // Pointer to an array of keys in the node
    int numOfKeys;
    bool isLeaf;

public:
    TreeNode(int maxKeys)
    {
        keys = new int[maxKeys];
        pointers = new Address[maxKeys + 1];
        numOfKeys = 0;
        isLeaf = false;
        
        Address nullAddress = {nullptr,0};
        for (int i = 0; i < maxKeys + 1; i++)
        {
            setPointer(i,nullAddress);
        }
    };

    int getKey(int index){return keys[index];};
    Address getPointer(int index){return pointers[index];};
    int getNumOfKeys(){return numOfKeys;};
    bool getIsLeaf(){return isLeaf;};

    void setKey(int index, int value){keys[index] = value;};
    void setNumOfKeys(int numOfKeys){this->numOfKeys = numOfKeys;};
    void setPointer(int index, Address address){pointers[index] = address;};
    void setIsLeaf(bool isLeaf){this->isLeaf = isLeaf;};
};

#endif //TREENODE_H