#include <cstddef>
#include <cmath>
#include <cstring>
#include <stdlib.h>
#include "b_plus_tree.h"

BPlusTree::BPlusTree(std::size_t blockSize, MemoryPool *disk, MemoryPool *index)
{
    this->blockSize = blockSize;
    this->disk = disk;
    this->index = index;
    this->maxKeys = BPlusTree::calculateMaxKeys(blockSize);
    this->root = NULL;
    this->rootAddress = NULL;
    this->rootIndex = 0;
    this->height = 0;
    this->numOfNodes = 0;
};

int BPlusTree::calculateMaxKeys(std::size_t blockSize)
{
    int sum = sizeof(Address) + sizeof(bool) + sizeof(int);
    int maxKeys = 0;
    while(sum + sizeof(int) + sizeof(Address) <= blockSize)
    {
        maxKeys ++;
        sum += sizeof(int) + sizeof(Address);
    }
    return maxKeys;
};

// Code for inseting

void BPlusTree::insert(Address recordAddress, int value)
{ 
    // If no root exists, create a new B+ Tree root.
  if (root == nullptr)
  {
    // Create a new linked list (for duplicates) at the key.
    TreeNode *LLNode = new TreeNode(maxKeys);
    LLNode->setKey(0, value);
    LLNode->setNumOfKeys(1);
    LLNode->setPointer(0, recordAddress); // The disk address of the key just inserted

    // Allocate LLNode and root address
    Address LLNodeAddress = index->saveToDisk((void *)LLNode, sizeof(* root));

    // Create new node in main memory, set it to root, and add the key and values to it.
    root = new TreeNode(maxKeys);
    root->setKey(0, value);
    root->setIsLeaf(true); // It is both the root and a leaf.
    root->setNumOfKeys(1);
    root->setPointer(0, LLNodeAddress); // Add record's disk address to pointer.
    
    // Write the root node into disk and track of root node's disk address.
    Address wholeAddress = index->saveToDisk(root, sizeof(*root));
    rootAddress = wholeAddress.blockAddress;
    rootIndex = wholeAddress.index;
    height = 1;
  }
  // Else if root exists already, traverse the nodes to find the proper place to insert the key.
  else
  {

    TreeNode *current = root;
    TreeNode *parent;                          // Keep track of the parent as we go deeper into the tree in case we need to update it.
    void *parentDiskAddress = rootAddress; // Keep track of parent's disk address as well so we can update parent in disk.
    short int parentDiskIndex = rootIndex;
    void *currentDiskAddress = rootAddress; // Store current node's disk address in case we need to update it in disk.
    short int currentDiskIndex = rootIndex;

    // While not leaf, keep following the nodes to correct key.
    while (current->getIsLeaf() == false)
    {

      // Set the parent of the node (in case we need to assign new child later), and its disk address.
      parent = current;
      parentDiskAddress = currentDiskAddress;
      parentDiskIndex = currentDiskIndex;

      // Check through all keys of the node to find key and pointer to follow downwards.
      for (int i = 0; i < current->getNumOfKeys(); i++)
      {
        // If value is lesser than current key, go to the left pointer's node.
        if (value < current->getKey(i))
        {
          // Load node in from disk to main memory.
          TreeNode *mainMemoryNode = (TreeNode *)index->loadFromDisk(current->getPointer(i), sizeof(* root));

          // Update cursorDiskAddress to maintain address in disk if we need to update nodes.
          currentDiskAddress = current->getPointer(i).blockAddress;
          currentDiskIndex = current->getPointer(i).index;

          // Move to new node in main memory.
          current = mainMemoryNode;
          break;
        }
        // Else if value larger than all keys in the node, go to last pointer's node (rightmost).
        if (i == current->getNumOfKeys() - 1)
        {
          // Load node in from disk to main memory.
          TreeNode *mainMemoryNode = (TreeNode *)index->loadFromDisk(current->getPointer(i + 1), sizeof(* root));

          // Update diskAddress to maintain address in disk if we need to update nodes.
          currentDiskAddress = current->getPointer(i + 1).blockAddress;
          currentDiskIndex = current->getPointer(i + 1).index;

          // Move to new node in main memory.
          current = mainMemoryNode;
          break;
        }
      }
    }

    // When we reach here, it means we have hit a leaf node. Let's find a place to put our new record in.
    // If this leaf node still has space to insert a value, then find out where to put it.
    if (current->getNumOfKeys() < maxKeys)
    {
      int i = 0;
      // While we haven't reached the last key and the key we want to insert is larger than current key, keep moving forward.
      while (value > current->getKey(i) && i < current->getNumOfKeys())
      {
        i++;
      }
      // i is where our key goes in. Check if it's already there (duplicate).
      if (current->getKey(i) == value)
      {
        // If it's a duplicate, linked list already exists. Insert into linked list.
        // Insert and update the linked list head.
        
        current->setPointer(i, insertLL(current->getPointer(i), recordAddress, value));
      }
      else
      {
        // Update the last pointer to point to the previous last pointer's node. Aka maintain cursor -> Y linked list.
        Address next = current->getPointer(current->getNumOfKeys());

        // Now i represents the index we want to put our key in. We need to shift all keys in the node back to fit it in.
        // Swap from number of keys + 1 (empty key) backwards, moving our last key back and so on. We also need to swap pointers.
        for (int j = current->getNumOfKeys(); j > i; j--)
        {
          // Just do a simple swap from the back to preserve index order.
          current->setKey(j, current->getKey(j - 1));
          current->setPointer(j, current->getPointer(j - 1));
        }

        // Insert our new key and pointer into this node.
        current->setKey(i, value);

        // We need to make a new linked list to store our record.
        // Create a new linked list (for duplicates) at the key.
        TreeNode *LLNode = new TreeNode(maxKeys);
        LLNode->setKey(0, value);
        LLNode->setNumOfKeys(1);
        LLNode->setPointer(0, recordAddress); // The disk address of the key just inserted

        // Allocate LLNode into disk.
        Address LLNodeAddress = index->saveToDisk((void *)LLNode, sizeof(* root));

        // Update variables
        current->setPointer(i, LLNodeAddress);
        current->setNumOfKeys(current->getNumOfKeys()+1);
  
        // Update leaf node pointer link to next node
        current->setPointer(current->getNumOfKeys(), next);

        // Now insert operation is complete, we need to store this updated node to disk.
        // currentDiskAddress is the address of node in disk, current is the address of node in main memory.
        // In this case, we count read/writes as 1/O only (Assume block remains in main memory).
        Address currentrOriginalAddress{currentDiskAddress, currentDiskIndex};
        index->saveToDisk(current, sizeof(* root), currentrOriginalAddress);
      }
    }
    // Overflow: If there's no space to insert new key, we have to split this node into two and update the parent if required.
    else
    {
      // Create a new leaf node to put half the keys and pointers in.
      TreeNode *newLeaf = new TreeNode(maxKeys);

      // Copy all current keys and pointers (including new key to insert) to a temporary list.
      float tempKeyList[maxKeys + 1];

      // We only need to store pointers corresponding to records (ignore those that points to other nodes).
      // Those that point to other nodes can be manipulated by themselves without this array later.
      Address tempPointerList[maxKeys + 2];
      Address next = current->getPointer(current->getNumOfKeys());

      // Copy all keys and pointers to the temporary lists.
      int i = 0;
      for (i = 0; i < maxKeys; i++)
      {
        tempKeyList[i] = current->getKey(i);
        tempPointerList[i] = current->getPointer(i);
      }
      //tempPointerList[maxKeys] = current->getPointer(maxKeys);

      // Insert the new key into the temp key list, making sure that it remains sorted. Here, we find where to insert it.
      i = 0;
      while (value > tempKeyList[i] && i < maxKeys)
      {
        i++;
      }

      // KIVVVVVVVVVV OUT OF RANGE

      // i is where our key goes in. Check if it's already there (duplicate).
      // make sure it is not the last one 
      if (i < current->getNumOfKeys()) {
        if (current->getKey(i) == value)
        {
          // If it's a duplicate, linked list already exists. Insert into linked list.
          // Insert and update the linked list head.
          current->setPointer(i, insertLL(current->getPointer(i), recordAddress, value));
          return;
        } 
      }

      // Else no duplicate, insert new key.
      // The key should be inserted at index i in the temporary lists. Move all elements back.
      for (int j = maxKeys; j > i; j--)
      {
        // Bubble swap all elements (keys and pointers) backwards by one index.
        tempKeyList[j] = tempKeyList[j - 1];
        tempPointerList[j] = tempPointerList[j - 1];
      }

      // Insert the new key and pointer into the temporary lists.
      tempKeyList[i] = value;

      // The address to insert will be a new linked list node.
      // Create a new linked list (for duplicates) at the key.
      TreeNode *LLNode = new TreeNode(maxKeys);
      LLNode->setKey(0, value);
      LLNode->setNumOfKeys(1);
      LLNode->setPointer(0, recordAddress); // The disk address of the key just inserted

      // Allocate LLNode into disk.
      Address LLNodeAddress = index->saveToDisk((void *)LLNode, sizeof(* root));
      tempPointerList[i] = LLNodeAddress;
      
      newLeaf->setIsLeaf(true); // New node is a leaf node.

      // Split the two new nodes into two. ⌊(n+1)/2⌋ keys for left, n+1 - ⌊(n+1)/2⌋ (aka remaining) keys for right.
      current->setNumOfKeys(ceil((float(maxKeys) + 1) / 2));
      newLeaf->setNumOfKeys(floor((maxKeys + 1) / 2));

      // Set the last pointer of the new leaf node to point to the previous last pointer of the existing node (current).
      // Essentially newLeaf -> Y, where Y is some other leaf node pointer wherein current -> Y previously.
      // We use maxKeys since current was previously full, so last pointer's index is maxKeys.
      newLeaf->setPointer(newLeaf->getNumOfKeys(), next);

      // Now we need to deal with the rest of the keys and pointers.
      // Note that since we are at a leaf node, pointers point directly to records on disk.

      // Add in keys and pointers in both the existing node, and the new leaf node.
      // First, the existing node (current).
      for (i = 0; i < current->getNumOfKeys(); i++)
      {
        current->setKey(i, tempKeyList[i]);
        current->setPointer(i, tempPointerList[i]);
      }

      // Then, the new leaf node. Note we keep track of the i index, since we are using the remaining keys and pointers.
      for (int j = 0; j < newLeaf->getNumOfKeys(); i++, j++)
      {
        newLeaf->setKey(j, tempKeyList[i]);
        newLeaf->setPointer(j, tempPointerList[i]);
      }

      // Now that we have finished updating the two new leaf nodes, we need to write them to disk.
      Address newLeafAddress = index->saveToDisk(newLeaf, sizeof(* root));
      // Now to set the current's pointer to the disk address of the leaf and save it in place
      current->setPointer(current->getNumOfKeys(), newLeafAddress);

      // wipe out the wrong pointers and keys from current
      for (int i = current->getNumOfKeys(); i < maxKeys; i++) {
        current->setKey(i, int());
      }
      for (int i = current->getNumOfKeys()+1; i < maxKeys + 1; i++) {
        Address nullAddress{nullptr, 0};
        current->setPointer(i, nullAddress);
      }

      Address currentOriginalAddress{currentDiskAddress, currentDiskIndex};
      index->saveToDisk(current, sizeof(* root), currentOriginalAddress);

      // If we are at root (aka root == leaf), then we need to make a new parent root.
      if (current == root)
      {
        TreeNode *newRoot = new TreeNode(maxKeys);

        // We need to set the new root's key to be the left bound of the right child.
        newRoot->setKey(0, newLeaf->getKey(0));

        // Point the new root's children as the existing node and the new node.
        Address currentDisk{currentDiskAddress, currentDiskIndex};

        newRoot->setPointer(0, currentDisk);
        newRoot->setPointer(1, newLeafAddress);

        // Update new root's variables.
        newRoot->setNumOfKeys(1);

        // Write the new root node to disk and update the root disk address stored in B+ Tree.
        Address newRootAddress = index->saveToDisk(newRoot, sizeof(* root));

        // Update the root address
        rootAddress = newRootAddress.blockAddress;
        rootIndex = newRootAddress.index;
        root = newRoot;
      }
      // If we are not at the root, we need to insert a new parent in the middle levels of the tree.
      else
      { 
        Address parentAddress = {parentDiskAddress, parentDiskIndex};
        insertInternal(newLeaf->getKey(0), parentAddress, newLeafAddress);
      }
    }
  }

  // update numnodes 
  numOfNodes = index->getBlocksAllocated();
};

void BPlusTree::insertInternal(int value, Address currentDiskAddress, Address childDiskAddress)
{
    // Load in current (parent) and child from disk to get latest copy.
  TreeNode *current = (TreeNode *)index->loadFromDisk(currentDiskAddress, sizeof(* root));

  if (currentDiskAddress.blockAddress == rootAddress && currentDiskAddress.index == rootIndex)
  {
    root = current;
  }

  TreeNode *child = (TreeNode *)index->loadFromDisk(childDiskAddress, sizeof(* root));

  // If parent (current) still has space, we can simply add the child node as a pointer.
  // We don't have to load parent from the disk again since we still have a main memory pointer to it.
  if (current->getNumOfKeys() < maxKeys)
  {
    // Iterate through the parent to see where to put in the lower bound key for the new child.
    int i = 0;
    while (value > current->getKey(i) && i < current->getNumOfKeys())
    {
      i++;
    }

    // Now we have i, the index to insert the key in. Bubble swap all keys back to insert the new child's key.
    // We use numKeys as index since we are going to be inserting a new key.
    for (int j = current->getNumOfKeys(); j > i; j--)
    {
      current->setKey(j, current->getKey(j - 1));
    }

    // Shift all pointers one step right (right pointer of key points to lower bound of key).
    for (int j = current->getNumOfKeys() + 1; j > i + 1; j--)
    {
      current->setPointer(j, current->getPointer(j - 1));
    }

    // Add in new child's lower bound key and pointer to the parent.
    current->setKey(i, value);
    current->setNumOfKeys(current->getNumOfKeys()+1);

    // Right side pointer of key of parent will point to the new child node.

    current->setPointer((i + 1), childDiskAddress);

    // Write the updated parent (current) to the disk.

    index->saveToDisk(current, sizeof(* root), currentDiskAddress);
  }
  // If parent node doesn't have space, we need to recursively split parent node and insert more parent nodes.
  else
  { 
    // Make new internal node (split this parent node into two).
    // Note: We DO NOT add a new key, just a new pointer!
    TreeNode *newInternal = new TreeNode(maxKeys);

    // Same logic as above, keep a temp list of keys and pointers to insert into the split nodes.
    // Now, we have one extra pointer to keep track of (new child's pointer).
    float tempKeyList[maxKeys + 1];
    Address tempPointerList[maxKeys + 2];

    // Copy all keys into a temp key list.
    // Note all keys are filled so we just copy till maxKeys.
    for (int i = 0; i < maxKeys; i++)
    {
      tempKeyList[i] = current->getKey(i);
    }

    // Copy all pointers into a temp pointer list.
    // There is one more pointer than keys in the node so maxKeys + 1.
    for (int i = 0; i < maxKeys + 1; i++)
    {
      tempPointerList[i] = current->getPointer(i);
    }

    // Find index to insert key in temp key list.
    int i = 0;
    while (value > tempKeyList[i] && i < maxKeys)
    {
      i++;
    }

    // Swap all elements higher than index backwards to fit new key.
    int j;
    for (int j = maxKeys; j > i; j--)
    {
      tempKeyList[j] = tempKeyList[j - 1];
    }

    // Insert new key into array in the correct spot (sorted).
    tempKeyList[i] = value;

    // Move all pointers back to fit new child's pointer as well.
    for (int j = maxKeys + 1; j > i + 1; j--)
    {
      tempPointerList[j] = tempPointerList[j - 1];
    }

    // Insert a pointer to the child to the right of its key.

    tempPointerList[i + 1] = childDiskAddress;

    // Split the two new nodes into two. ⌊(n)/2⌋ keys for left.
    // For right, we drop the rightmost key since we only need to represent the pointer.
    current->setNumOfKeys((maxKeys + 1) / 2);
    newInternal->setNumOfKeys((maxKeys + 1) / 2);

    // Reassign keys into current from tempkeyslist to account for new child node
    for (int i = 0; i < current->getNumOfKeys(); i++)
    {
      current->setKey(i, tempKeyList[i]);
    }

    for (int i = 0; i <= current->getNumOfKeys(); i++)
    {
      current->setPointer(i, tempPointerList[i]);
    }
    
    // Insert new keys into the new internal parent node.
    for (i = 0, j = current->getNumOfKeys()+1; i < newInternal->getNumOfKeys(); i++, j++)
    {
      newInternal->setKey(i, tempKeyList[j]);
    }

    // Insert pointers into the new internal parent node.
    for (i = 0, j = current->getNumOfKeys() + 1; i < newInternal->getNumOfKeys() + 1; i++, j++)
    {
      newInternal->setPointer(i, tempPointerList[j]);
    }

    // Note that we don't have to modify keys in the old parent current.
    // Because we already reduced its numKeys as we are only adding to the right bound.

    // KIVVVVVVV

    // Get rid of unecessary current keys and pointers
    for (int i = current->getNumOfKeys(); i < maxKeys; i++) 
    {
      current->setKey(i, int());
    }

    for (int i = current->getNumOfKeys() + 1; i < maxKeys + 1; i++)
    {
      Address nullAddress{nullptr, 0};
      current->setPointer(i, nullAddress);
    }

    // assign the new child to the original parent
 //   current->setPointer(current->getNumOfKeys(), childAddress);

    // Save the old parent and new internal node to disk.

    index->saveToDisk(current, sizeof(* root), currentDiskAddress);

    // Address newInternalAddress{newInternal, 0};
    Address newInternalDiskAddress = index->saveToDisk(newInternal, sizeof(* root));

    // If current current is the root of the tree, we need to create a new root.
    if (current == root)
    {
      TreeNode *newRoot = new TreeNode(maxKeys);
      // Update newRoot to hold the children.
      // Take the rightmost key of the old parent to be the root.
      // Although we threw it away, we are still using it to denote the leftbound of the old child.
      //newRoot->setKey(0, current->getKey(current->getNumOfKeys()));
      Address firstLeafNodeAddress = getFirstLeaf(newInternalDiskAddress);
      TreeNode *firstLeafNode = (TreeNode *)index->loadFromDisk(firstLeafNodeAddress, sizeof(* root));
      newRoot->setKey(0, firstLeafNode->getKey(0));

      // Update newRoot's children to be the previous two nodes

      newRoot->setPointer(0, currentDiskAddress);
      newRoot->setPointer(1, newInternalDiskAddress);

      // Update variables for newRoot
      newRoot->setNumOfKeys(1);

      root = newRoot;

      // Save newRoot into disk.
      Address newRootAddress = index->saveToDisk(root, sizeof(*root));

      // Update rootAddress
      rootAddress = newRootAddress.blockAddress;
      rootIndex = newRootAddress.index;
    }
    // Otherwise, parent is internal, so we need to split and make a new parent internally again.
    // This is done recursively if needed.
    else
    { 
      Address wholeAddress = {rootAddress, rootIndex};
      Address parentDiskNode = findParent(wholeAddress, currentDiskAddress, current->getKey(0));

      // KIVVVVV
      // insertInternal(current->keys[current->numKeys], parentDiskAddress, (Node *)newInternalDiskAddress.blockAddress);
      insertInternal(tempKeyList[current->getNumOfKeys()], parentDiskNode, newInternalDiskAddress);
    }
  }
};

Address BPlusTree::insertLL(Address LLHead, Address address, int value)
{
  // Load the linked list head node into main memory.
  TreeNode *head = (TreeNode *)index->loadFromDisk(LLHead, sizeof(* root));

  // Check if the head node has space to put record.
  if (head->getNumOfKeys() < maxKeys)
  {

    // Move all keys back to insert at the head.
    for (int i = head->getNumOfKeys(); i > 0; i--)
    {
      head->setKey(i, head->getKey(i - 1));
    }

    // Move all pointers back to insert at the head.
    for (int i = head->getNumOfKeys() + 1; i > 0; i--)

    {
      head->setPointer(i, head->getPointer(i - 1));
    }

    // Insert new record into the head of linked list.
    head->setKey(0, value);
    head->setPointer(0, address); // the disk address of the key just inserted
    head->setNumOfKeys(head->getNumOfKeys() + 1);
    
    // Write head back to disk.
    LLHead = index->saveToDisk((void *)head, sizeof(* root), LLHead);

    // displayNode(head);
    //   if(head->getPointer(head->getNumOfKeys()).blockAddress){
    //     Address innerPrintNode = head->getPointer(head->getNumOfKeys());
    //     TreeNode *innerPrintNod = (TreeNode *)index->loadFromDisk(innerPrintNode,sizeof(* root));
    //     std::cout << "continue: " ;
    //     displayNode(innerPrintNod);
    //   }

    // Return head address
    return LLHead;
  }
  // No space in head node, need a new linked list node.
  else
  {
    // Make a new node and add variables
    TreeNode *LLNode = new TreeNode(maxKeys);
    LLNode->setKey(0, value);
    LLNode->setNumOfKeys(1);

    // Insert key into head of linked list node.
    LLNode->setPointer(0, address);

    // Now this node is head of linked list, point to the previous head's disk address as next.
    LLNode->setPointer(1, LLHead);

    // Write new linked list node to disk.
    Address LLNodeAddress = index->saveToDisk((void *)LLNode, sizeof(* root));

    // for (int i = 0; i < LLNode->getNumOfKeys(); i ++)
    // {
    //   displayNode(LLNode);
    //     if(LLNode->getPointer(LLNode->getNumOfKeys()).blockAddress){
    //       Address innerPrintNode = LLNode->getPointer(LLNode->getNumOfKeys());
    //       TreeNode *innerPrintNod = (TreeNode *)index->loadFromDisk(innerPrintNode,sizeof(* root));
    //       std::cout << "continue: " ;
    //       displayNode(innerPrintNod);
    //     }
    // }

    // Return disk address of new linked list head
    return LLNodeAddress;
  }
}

// Code for removing

void BPlusTree::remove(int value)
{

};

void BPlusTree::removeInternal(int value, Address current, Address child)
{
    
};

// Code for find Parent node

Address BPlusTree::findParent(Address currentDiskAddress, Address childDiskAddress, int value)
{
  Address parent = currentDiskAddress;
  Address nullAddress = {nullptr, 0};
  TreeNode *current = (TreeNode *)index->loadFromDisk(currentDiskAddress, sizeof(* root));
  if(current->getIsLeaf())
  {
    return nullAddress;
  }
  for(int i = 0; i < current->getNumOfKeys()+1; i++)
  {
    // std::cout << i << " Current node key: " <<  current->getKey(i) << " finding: " << value << std::endl;
    if(current->getPointer(i).blockAddress == childDiskAddress.blockAddress && current->getPointer(i).index == childDiskAddress.index)
    {
      
      parent = currentDiskAddress;
      return parent;
    }
    // else
    // {
    //     parent = findParent(current->getPointer(i),childDiskAddress, value);
    //     if(parent.blockAddress!=NULL)return parent;
    // }
  }
  for (int i = 0; i < current->getNumOfKeys(); i++)
  {
    // If key is lesser than current key, go to the left pointer's node.
    if (value < current->getKey(i))
    {
      // Move to new node in main memory.
      findParent(current->getPointer(i), childDiskAddress, value);
    }

    // Else if key larger than all keys in the node, go to last pointer's node (rightmost).
    if (i == current->getNumOfKeys() - 1)
    {
      // Load node in from disk to main memory.
      findParent(current->getPointer(current->getNumOfKeys()), childDiskAddress, value);
    }
  }
  return parent;
}
// {
//     // Load in current into main memory, starting from root.

//   Address nullAddress = {nullptr, 0};
//   TreeNode *current = (TreeNode *)index->loadFromDisk(currentDiskAddress, sizeof(* root));

//   // If the root current passed in is a leaf node, there is no children, therefore no parent.
//   if (current->getIsLeaf())
//   { 
//     return nullAddress;
//   }

//   // Maintain parentDiskAddress
//   Address parentDiskAddress = currentDiskAddress;

//   // While not leaf, keep following the nodes to correct key.
//   while (true)
//   {
//     // Check through all pointers of the node to find match.
//     for (int i = 0; i < current->getNumOfKeys() + 1; i++)
//     {
//       if (current->getPointer(i).blockAddress == childDiskAddress.blockAddress && current->getPointer(i).index == childDiskAddress.index)
//       {
//         return parentDiskAddress;
//       }
//     }

//     for (int i = 0; i < current->getNumOfKeys(); i++)
//     {
//       // If key is lesser than current key, go to the left pointer's node.
//       if (value < current->getKey(i))
//       {
//         // Load node in from disk to main memory.
//         TreeNode *mainMemoryNode = (TreeNode *)index->loadFromDisk(current->getPointer(i), sizeof(* root));

//         // Update parent address.
//         parentDiskAddress = current->getPointer(i);

//         // Move to new node in main memory.
//         current = (TreeNode *)mainMemoryNode;
//         break;
//       }

//       // Else if key larger than all keys in the node, go to last pointer's node (rightmost).
//       if (i == current->getNumOfKeys() - 1)
//       {
//         // Load node in from disk to main memory.
//         TreeNode *mainMemoryNode = (TreeNode *)index->loadFromDisk(current->getPointer(i + 1), sizeof(* root));

//         // Update parent address.
//         parentDiskAddress = current->getPointer(i + 1);

//         // Move to new node in main memory.
//         current = (TreeNode *)mainMemoryNode;
//         break;
//       }
//     }
//   }

//   // If we reach here, means cannot find already.
//   return nullAddress;
// };

// Code for query

void BPlusTree::search(int leftValue, int rightValue)
{
    // Tree is empty.
  if (rootAddress == nullptr)
  {
    throw std::logic_error("Tree is empty!");
  }
  // Else iterate through root node and follow the keys to find the correct key.
  else
  {
    // Load in root from disk.
    Address rootDiskAddress{rootAddress, rootIndex};
    root = (TreeNode *)index->loadFromDisk(rootDiskAddress, sizeof(* root));

    // for displaying to output file
    std::cout << "Index node accessed. Content is -----";
    displayNode(root);

    TreeNode *current = root;

    bool found = false;

    // While we haven't hit a leaf node, and haven't found a range.
    while (current->getIsLeaf() == false)
    {
      // Iterate through each key in the current node. We need to load nodes from the disk whenever we want to traverse to another node.
      for (int i = 0; i < current->getNumOfKeys(); i++)
      {
        // If lowerBoundKey is lesser than current key, go to the left pointer's node to continue searching.
        if (leftValue < current->getKey(i))
        {
          // Load node from disk to main memory.
          current = (TreeNode *)index->loadFromDisk(current->getPointer(i), sizeof(* root));

          // for displaying to output file
          std::cout << "Index node accessed. Content is -----";
          displayNode(current);

          break;
        }
        // If we reached the end of all keys in this node (larger than all), then go to the right pointer's node to continue searching.
        if (i == current->getNumOfKeys() - 1)
        {
          // Load node from disk to main memory.
          // Set current to the child node, now loaded in main memory.
          current = (TreeNode *)index->loadFromDisk(current->getPointer(i + 1), sizeof(* root));

          // for displaying to output file
          std::cout << "Index node accessed. Content is -----";
          displayNode(current);
          break;
        }
      }
    }

    // When we reach here, we have hit a leaf node corresponding to the lowerBoundKey.
    // Again, search each of the leaf node's keys to find a match.
    // vector<Record> results;
    // unordered_map<void *, void *> loadedBlocks; // Maintain a reference to all loaded blocks in main memory.

    // Keep searching whole range until we find a key that is out of range.
    bool stop = false;

    while (stop == false)
    {
      int i;
      for (i = 0; i < current->getNumOfKeys(); i++)
      {
        // Found a key within range, now we need to iterate through the entire range until the upperBoundKey.
        if (current->getKey(i) > rightValue)
        {
          stop = true;
          break;
        }
        if (current->getKey(i) >= leftValue && current->getKey(i) <= rightValue)
        {
          // for displaying to output file
          std::cout << "Leaf Node content is -----";
          displayNode(current);

          // Add new line for each leaf node's linked list printout.
          std::cout << std::endl;
          std::cout << "LLNode: tconst for number of votes: " << current->getKey(i) << " > ";          

          // Access the linked list node and print records.
          displayLL(current->getPointer(i));
        }
      }
      while (current->getKey(current->getNumOfKeys()-1)< rightValue)
      {
        Address nextNodeAddress = current->getPointer(current->getNumOfKeys());
        TreeNode *nextNode = (TreeNode *)index->loadFromDisk(nextNodeAddress, sizeof(*root));
        current = nextNode;
        for (i = 0; i < current->getNumOfKeys(); i++)
        {
          // Found a key within range, now we need to iterate through the entire range until the upperBoundKey.
          if (current->getKey(i) > rightValue)
          {
            stop = true;
            break;
          }
          if (current->getKey(i) >= leftValue && current->getKey(i) <= rightValue)
          {
            // for displaying to output file
            std::cout << "Leaf Node content is -----";
            displayNode(current);

            // Add new line for each leaf node's linked list printout.
            std::cout << std::endl;
            std::cout << "LLNode: tconst for number of votes: " << current->getKey(i) << " > ";          

            // Access the linked list node and print records.
            displayLL(current->getPointer(i));
          }
        }
      }

      // // On the last pointer, check if last key is max, if it is, stop. Also stop if it is already equal to the max
      // if (current->getPointer(current->getNumOfKeys()).blockAddress != nullptr && current->getKey(i) != rightValue)
      // {
      //   // Set current to be next leaf node (load from disk).
      //   current = (TreeNode *)index->loadFromDisk(current->getPointer(current->getNumOfKeys()), sizeof(* root));

      //   // for displaying to output file
      //   std::cout << "Index node accessed. Content is -----";
      //   displayNode(current);

      // }
      // else
      // {
      //   stop = true;
      // }
    }
  }
};

// Code for displaying the tree

void BPlusTree::displayTree(TreeNode *currentDiskAddress, int height)
{
  // Load in current from disk.
  Address currentMainMemoryAddress{currentDiskAddress, 0};
  TreeNode *current = (TreeNode *)index->loadFromDisk(currentMainMemoryAddress, sizeof(* root));

  // If tree exists, display all nodes.
  if (current != nullptr)
  {
    for (int i = 0; i < height; i++)
    {
      std::cout << "   ";
    }
    std::cout << " height " << height << ": ";

    displayNode(current);

    if (current->getIsLeaf() != true)
    {
      for (int i = 0; i < current->getNumOfKeys() + 1; i++)
      {
        // Load node in from disk to main memory.
        TreeNode *mainMemoryNode = (TreeNode *)index->loadFromDisk(current->getPointer(i), sizeof(* root));

        displayTree((TreeNode *)mainMemoryNode, height + 1);
      }
    }
  }
};

void BPlusTree::displayNode(TreeNode *current)
{
  // Print out all contents in the current node as such |pointer|key|pointer|
  int i = 0;
  std::cout << "|";
  for (int i = 0; i < current->getNumOfKeys(); i++)
  {
    // std::cout << current->getPointer(i).blockAddress << "" << current->getPointer(i).index << " | ";
    std::cout << current->getKey(i) << " | ";
  }

  // Print last filled pointer
  // if (current->getPointer(current->getNumOfKeys()).blockAddress == nullptr) {
  //   std::cout << " Null |";
  // }
  // else {
  //   std::cout << current->getPointer(current->getNumOfKeys()).blockAddress << "" << current->getPointer(i).index << "|";
  // }

  for (int i = current->getNumOfKeys(); i < maxKeys; i++)
  {
    std::cout << " x |";      // Remaining empty keys
    // std::cout << "  Null  |"; // Remaining empty pointers
  }

  std::cout << std::endl;
};

Address BPlusTree::getFirstLeaf(Address current)
{
  Address nullAddress = {nullptr, 0};
  if(current.blockAddress == nullptr){
    return nullAddress;
  }else{
      TreeNode *leafNode = (TreeNode *)index->loadFromDisk(current, sizeof(* root));
      while(!leafNode->getIsLeaf()){
        leafNode = (TreeNode *)index->loadFromDisk(leafNode->getPointer(0), sizeof(* root));
      }
      Address leafNodeAddress = {leafNode, 0};
      return (leafNodeAddress);
  }
};

void BPlusTree::displayBlock(void *blockAddress)
{
  // Load block into memory
  void *block = operator new(blockSize);
  std::memcpy(block, blockAddress, blockSize);

  unsigned char testBlock[blockSize];
  std::memset(testBlock, '\0', blockSize);

  // Block is empty.
  if (std::memcmp(testBlock, block, blockSize) == 0)
  {
    std::cout << "Empty block!" << '\n';
    return;
  }

  unsigned char *blockChar = (unsigned char *)block;

  int i = 0;
  while (i < blockSize)
  {
    // Load each record
    void *recordAddress = operator new(sizeof(Record));
    std::memcpy(recordAddress, blockChar, sizeof(Record));

    Record *record = (Record *)recordAddress;

    std::cout << "[" << record->tconst << "|" << record->averageRating << "|" << record->numVotes << "]  ";
    blockChar += sizeof(Record);
    i += sizeof(Record);
  }
  
}

void BPlusTree::displayLL(Address address)
{
  // Load linked list head into main memory.
  TreeNode *head = (TreeNode *)index->loadFromDisk(address, sizeof(* root));

  std::cout << "\nData block accessed. Content is -----";
  displayNode(head);
  std::cout << std::endl;
  // Print all records in the linked list.
  for (int i = 0; i < head->getNumOfKeys(); i++)
  {
    // Load the block from disk.
    // void *blockMainMemoryAddress = operator new(nodeSize);
    // std::memcpy(blockMainMemoryAddress, head->pointers[i].blockAddress, nodeSize);

    Record result = *(Record *)(disk->loadFromDisk(head->getPointer(i), sizeof(Record)));
    std::cout << result.tconst << " | " << result.averageRating << " | " << result.numVotes << " | ";


  }

  // // Print empty slots
  // for (int i = head->getNumOfKeys(); i < maxKeys; i++)
  // {
  //   std::cout << "x | ";
  // }
  
  // End of linked list
  if (head->getPointer(head->getNumOfKeys()).blockAddress == nullptr)
  { 
    std::cout << std::endl;
    std::cout << "End of linked list" << std::endl;
    return;
  }

  // Move to next node in linked list.
  if (head->getPointer(head->getNumOfKeys()).blockAddress != nullptr)
  {
    displayLL(head->getPointer(head->getNumOfKeys()));
  }
}