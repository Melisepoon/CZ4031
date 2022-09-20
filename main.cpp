#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string.h>
#include "memory_pool.h"
#include "b_plus_tree.h"


int main(){
    // chose the block size
    int BLOCKSIZE = 0;
    std::cout << "=============================Start=============================" << std::endl;
    std::cout << "Select Block Size: " << std::endl;

    int choice = 0;
    while (choice != 1 && choice != 2){
        std::cout << "Enter a choice:" << std::endl;
        std::cout << "1 for 200B" << std::endl;
        std::cout << "2 for 500B" << std::endl;
        std::cin >> choice;
        if (int(choice) == 1)
            BLOCKSIZE = 200;
        else if (int(choice) == 2)
            BLOCKSIZE = 500;
        else{
            std::cin.clear();
            std::cout << "Invalid input! Please chose 1 or 2." << std::endl;
        }
    }
    std::cout << "Block size selected: " << BLOCKSIZE << std::endl;
    std::cout << "=============================Creating Memory Pool=============================" << std::endl;

    MemoryPool disk(200000000, BLOCKSIZE); //200MB
    MemoryPool index(200000000, BLOCKSIZE); //200MB

    BPlusTree tree = BPlusTree(BLOCKSIZE, &disk, &index);
    std::cout << "Max records per block: " << disk.getMaxRecords() << std::endl;
    std::cout << "Max index keys per block: " << tree.getMaxKeys() << std::endl;

    std::cout << "Reading in data from data.tsv file" << std::endl;
    std::ifstream file("./data/data.tsv");

    int recordCount = 0;

    if (file.is_open())
    {
        std::string line;
        getline(file, line);

        while (getline(file, line))
        {
            Record tempRecord;
            std::istringstream lineStream(line);

            //Assigning tempRecord.tconst, tempRecord.averageRating & tempRecord.numVotes values
            lineStream >> tempRecord.tconst >> tempRecord.averageRating >> tempRecord.numVotes;

            //Insert this record into the database
            Address tempAddress = disk.saveToDisk(&tempRecord, sizeof(tempRecord));

            //Build the bplustree as we insert records
        //    tree.insert(tempRecord->numVotes);

            //Uncomment to see each data record
            //std::cout << tempRecord.tconst << "---"<< tempRecord.averageRating <<"---"<< tempRecord.numVotes << std::endl;
            //std::cout << "Inserted record " << recordCount + 1 << " at block address: " << tempAddress.blockAddress << " and index " << tempAddress.index << std::endl;
            recordCount += 1;
        }
    }

    std::cout << "Memory pool created with:" << std::endl;
    std::cout << "Block size: " << BLOCKSIZE << "B" << std::endl;
    std::cout << "Data Blocks Count: " << disk.getBlocksAllocated() << std::endl; 
    std::cout << "Total Records Count: " << recordCount << std::endl;


}