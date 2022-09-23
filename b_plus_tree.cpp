#include <cstddef>
#include <cmath>
#include "b_plus_tree.h"

BPlusTree::BPlusTree(std::size_t blockSize, MemoryPool *disk, MemoryPool *index)
{
    this->blockSize = blockSize;
    this->disk = disk;
    this->index = index;
    this->maxKeys = BPlusTree::calculateMaxKeys(blockSize);
    this->root = NULL;
    this->rootAddress = NULL;
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
    // Create a new tree node
    TreeNode *node = new TreeNode(maxKeys);
    node->setKey(0, value);
    node->setNumOfKeys(1);
    node->setIsLeaf(true);
    node->setPointer(0, recordAddress); // The disk address of the key just inserted

    // Allocate node and root address
    Address nodeAddress = index->saveToDisk((void *)node, blockSize);

    // Keep track of root node and root node's disk address.
    this->root = node;
    rootAddress = &nodeAddress;
  }
  // Else if root exists already, traverse the nodes to find the proper place to insert the key.
  else
  {
    TreeNode *current = root;
    TreeNode *parent;                          // Keep track of the parent as we go deeper into the tree in case we need to update it.
    void *parentDiskAddress = rootAddress; // Keep track of parent's disk address as well so we can update parent in disk.
    void *currentDiskAddress = rootAddress; // Store current node's disk address in case we need to update it in disk.

    // While not leaf, keep following the nodes to correct key.
    while (current->getIsLeaf() == false)
    {

      // Set the parent of the node (in case we need to assign new child later), and its disk address.
      parent = current;
      parentDiskAddress = currentDiskAddress;

      // Check through all keys of the node to find key and pointer to follow downwards.
      for (int i = 0; i < current->getNumOfKeys(); i++)
      {
        // If value is lesser than current key, go to the left pointer's node.
        if (value < current->getKey(i))
        {
          // Load node in from disk to main memory.
          TreeNode *mainMemoryNode = (TreeNode *)index->loadFromDisk(current->getPointer(i), blockSize);

          // Update cursorDiskAddress to maintain address in disk if we need to update nodes.
          currentDiskAddress = current->getPointer(i).blockAddress;

          // Move to new node in main memory.
          current = mainMemoryNode;
          break;
        }
        // Else if value larger than all keys in the node, go to last pointer's node (rightmost).
        if (i == current->getNumOfKeys() - 1)
        {
          // Load node in from disk to main memory.
          TreeNode *mainMemoryNode = (TreeNode *)index->loadFromDisk(current->getPointer(i + 1), blockSize);

          // Update diskAddress to maintain address in disk if we need to update nodes.
          currentDiskAddress = current->getPointer(i + 1).blockAddress;

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
        current->getPointer(i) = insertLL(current->getPointer(i), recordAddress, value);
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
        Address LLNodeAddress = index->saveToDisk((void *)LLNode, blockSize);

        // Update variables
        current->setPointer(i, LLNodeAddress);
        current->setNumOfKeys(current->getNumOfKeys()+1);
  
        // Update leaf node pointer link to next node
        current->setPointer(current->getNumOfKeys(), next);

        // Now insert operation is complete, we need to store this updated node to disk.
        // currentDiskAddress is the address of node in disk, current is the address of node in main memory.
        // In this case, we count read/writes as 1/O only (Assume block remains in main memory).
        Address currentrOriginalAddress{currentDiskAddress, 0};
        index->saveToDisk(current, blockSize, currentrOriginalAddress);
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
      Address tempPointerList[maxKeys + 1];
      Address next = current->getPointer(current->getNumOfKeys());

      // Copy all keys and pointers to the temporary lists.
      int i = 0;
      for (i = 0; i < maxKeys; i++)
      {
        tempKeyList[i] = current->getKey(i);
        tempPointerList[i] = current->getPointer(i);
      }

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
          current->getPointer(i) = insertLL(current->getPointer(i), recordAddress, value);
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
      Address LLNodeAddress = index->saveToDisk((void *)LLNode, blockSize);
      tempPointerList[i] = LLNodeAddress;
      
      newLeaf->setIsLeaf(true); // New node is a leaf node.

      // Split the two new nodes into two. ⌊(n+1)/2⌋ keys for left, n+1 - ⌊(n+1)/2⌋ (aka remaining) keys for right.
      current->setNumOfKeys(ceil((maxKeys + 1) / 2));
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
      Address newLeafAddress = index->saveToDisk(newLeaf, blockSize);

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

      Address currentOriginalAddress{currentDiskAddress, 0};
      index->saveToDisk(current, blockSize, currentOriginalAddress);

      // If we are at root (aka root == leaf), then we need to make a new parent root.
      if (current == root)
      {
        TreeNode *newRoot = new TreeNode(maxKeys);

        // We need to set the new root's key to be the left bound of the right child.
        newRoot->setKey(0, newLeaf->getKey(0));

        // Point the new root's children as the existing node and the new node.
        Address currentDisk{currentDiskAddress, 0};

        newRoot->setPointer(0, currentDisk);
        newRoot->setPointer(1, newLeafAddress);

        // Update new root's variables.
        newRoot->setNumOfKeys(1);

        // Write the new root node to disk and update the root disk address stored in B+ Tree.
        Address newRootAddress = index->saveToDisk(newRoot, blockSize);

        // Update the root address
        rootAddress = newRootAddress.blockAddress;
        root = newRoot;
      }
      // If we are not at the root, we need to insert a new parent in the middle levels of the tree.
      else
      {
        insertInternal(newLeaf->getKey(0), (TreeNode *)parentDiskAddress, (TreeNode *)newLeafAddress.blockAddress);
      }
    }
  }

  // update numnodes 
  numOfNodes = index->getBlocksAllocated();
};

void BPlusTree::insertInternal(int value, TreeNode *currentDiskAddress, TreeNode *childDiskAddress)
{
    // Load in current (parent) and child from disk to get latest copy.
  Address currentAddress{currentDiskAddress, 0};
  TreeNode *current = (TreeNode *)index->loadFromDisk(currentAddress, blockSize);

  if (currentDiskAddress == rootAddress)
  {
    root = current;
  }

  Address childAddress{childDiskAddress, 0};
  TreeNode *child = (TreeNode *)index->loadFromDisk(childAddress, blockSize);

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
    Address childAddress{childDiskAddress, 0};
    current->setPointer((i + 1), childAddress);

    // Write the updated parent (current) to the disk.
    Address currentAddress{currentDiskAddress, 0};
    index->saveToDisk(current, blockSize, currentAddress);
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
    Address childAddress = {childDiskAddress, 0};
    tempPointerList[i + 1] = childAddress;

    // Split the two new nodes into two. ⌊(n)/2⌋ keys for left.
    // For right, we drop the rightmost key since we only need to represent the pointer.
    current->setNumOfKeys(ceil((maxKeys + 1) / 2));
    newInternal->setNumOfKeys(floor((maxKeys + 1) / 2));

    // Reassign keys into current from tempkeyslist to account for new child node
    for (int i = 0; i < current->getNumOfKeys(); i++)
    {
      current->setKey(i, tempKeyList[i]);
    }
    
    // Insert new keys into the new internal parent node.
    for (i = 0, j = current->getNumOfKeys() + 1; i < newInternal->getNumOfKeys(); i++, j++)
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
    current->setPointer(current->getNumOfKeys(), childAddress);

    // Save the old parent and new internal node to disk.
    Address currentAddress{currentDiskAddress, 0};
    index->saveToDisk(current, blockSize, currentAddress);

    // Address newInternalAddress{newInternal, 0};
    Address newInternalDiskAddress = index->saveToDisk(newInternal, blockSize);

    // If current current is the root of the tree, we need to create a new root.
    if (current == root)
    {
      TreeNode *newRoot = new TreeNode(blockSize);
      // Update newRoot to hold the children.
      // Take the rightmost key of the old parent to be the root.
      // Although we threw it away, we are still using it to denote the leftbound of the old child.
      newRoot->setKey(0, current->getKey(current->getNumOfKeys()));

      // Update newRoot's children to be the previous two nodes
      Address currentAddress = {currentDiskAddress, 0};
      newRoot->setPointer(0, currentAddress);
      newRoot->setPointer(1, newInternalDiskAddress);

      // Update variables for newRoot
      newRoot->setNumOfKeys(1);

      root = newRoot;

      // Save newRoot into disk.
      Address newRootAddress = index->saveToDisk(root, blockSize);

      // Update rootAddress
      rootAddress = newRootAddress.blockAddress;
    }
    // Otherwise, parent is internal, so we need to split and make a new parent internally again.
    // This is done recursively if needed.
    else
    {
      TreeNode *parentDiskAddress = findParent((TreeNode *)rootAddress, currentDiskAddress, current->getKey(0));

      // KIVVVVV
      // insertInternal(current->keys[current->numKeys], parentDiskAddress, (Node *)newInternalDiskAddress.blockAddress);
      insertInternal(tempKeyList[current->getNumOfKeys()], parentDiskAddress, (TreeNode *)newInternalDiskAddress.blockAddress);
    }
  }
};

Address BPlusTree::insertLL(Address LLHead, Address address, int value)
{
  // Load the linked list head node into main memory.
  TreeNode *head = (TreeNode *)index->loadFromDisk(LLHead, blockSize);

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
    LLHead = index->saveToDisk((void *)head, blockSize, LLHead);

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
    Address LLNodeAddress = index->saveToDisk((void *)LLNode, blockSize);

    // Return disk address of new linked list head
    return LLNodeAddress;
  }
}

// Code for removing

void BPlusTree::remove(int value)
{

};

void BPlusTree::removeInternal(int value, TreeNode *current, TreeNode *child)
{
    
};

// Code for find Parent node

TreeNode *BPlusTree::findParent(TreeNode *currentDiskAddress, TreeNode *childDiskAddress, int value)
{
    // Load in current into main memory, starting from root.
  Address currentAddress{currentDiskAddress, 0};
  TreeNode *current = (TreeNode *)index->loadFromDisk(currentAddress, blockSize);

  // If the root current passed in is a leaf node, there is no children, therefore no parent.
  if (current->getIsLeaf())
  {
    return nullptr;
  }

  // Maintain parentDiskAddress
  TreeNode *parentDiskAddress = currentDiskAddress;

  // While not leaf, keep following the nodes to correct key.
  while (current->getIsLeaf() == false)
  {
    // Check through all pointers of the node to find match.
    for (int i = 0; i < current->getNumOfKeys() + 1; i++)
    {
      if (current->getPointer(i).blockAddress == childDiskAddress)
      {
        return parentDiskAddress;
      }
    }

    for (int i = 0; i < current->getNumOfKeys(); i++)
    {
      // If key is lesser than current key, go to the left pointer's node.
      if (value < current->getKey(i))
      {
        // Load node in from disk to main memory.
        TreeNode *mainMemoryNode = (TreeNode *)index->loadFromDisk(current->getPointer(i), blockSize);

        // Update parent address.
        parentDiskAddress = (TreeNode *)current->getPointer(i).blockAddress;

        // Move to new node in main memory.
        current = (TreeNode *)mainMemoryNode;
        break;
      }

      // Else if key larger than all keys in the node, go to last pointer's node (rightmost).
      if (i == current->getNumOfKeys() - 1)
      {
        // Load node in from disk to main memory.
        TreeNode *mainMemoryNode = (TreeNode *)index->loadFromDisk(current->getPointer(i + 1), blockSize);

        // Update parent address.
        parentDiskAddress = (TreeNode *)current->getPointer(i + 1).blockAddress;

        // Move to new node in main memory.
        current = (TreeNode *)mainMemoryNode;
        break;
      }
    }
  }

  // If we reach here, means cannot find already.
  return nullptr;
};

// Code for query

void BPlusTree::search(int leftValue, int rightValue)
{
    
};

// Code for displaying the tree

void BPlusTree::displayTree(TreeNode *currentDiskAddress, int height)
{
  // Load in current from disk.
  Address currentMainMemoryAddress{currentDiskAddress, 0};
  TreeNode *current = (TreeNode *)index->loadFromDisk(currentMainMemoryAddress, blockSize);

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
        TreeNode *mainMemoryNode = (TreeNode *)index->loadFromDisk(current->getPointer(i), blockSize);

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
    std::cout << current->getPointer(i).blockAddress << " | ";
    std::cout << current->getKey(i) << " | ";
  }

  // Print last filled pointer
  if (current->getPointer(current->getNumOfKeys()).blockAddress == nullptr) {
    std::cout << " Null |";
  }
  else {
    std::cout << current->getPointer(current->getNumOfKeys()).blockAddress << "|";
  }

  for (int i = current->getNumOfKeys(); i < maxKeys; i++)
  {
    std::cout << " x |";      // Remaining empty keys
    std::cout << "  Null  |"; // Remaining empty pointers
  }

  std::cout << std::endl;
};

void BPlusTree::getFirstLeaf()
{
    
};