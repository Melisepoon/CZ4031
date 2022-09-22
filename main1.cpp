#include <iostream>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <math.h>

using std::vector;
using namespace std;


struct Records
{
    public:
        int tconst;
        double averageRating;
        int numVotes;

        //to get the size of a record
        int RecordSize(){
            return sizeof(tconst) + sizeof(averageRating) + sizeof(numVotes);
        }
};

struct Blocks
{
    public:
        vector <Records> records;
        //to obtain the size of a block
        int BlockSize(){
            int BLOCKSIZE = 0;
            int i =0;
            for(i=0;i<records.size();i++){
                BLOCKSIZE += records[i].RecordSize();
            }
            return BLOCKSIZE;
        }

        //to calculate the number of records in a block
        int NumRecords(){
            return records.size();
        } 

        //to add a record into the block
        void addRecord(Records r){
            records.push_back(r);
        }  
    
};

struct Storage
{
    public:
        vector <Blocks> blocks;


        //to obtain size of a storage
        long StorageSize(){
            double STORAGESIZE=0;
            int j = 0;
            for(j=0; j<blocks.size();j++){
                STORAGESIZE += blocks[j].BlockSize();
            }
            return STORAGESIZE;
        }


        //to calculate the number of blocks in the storage
        int NumBlocks(){
            return blocks.size();
        } 

        //to get total number of records
        int getNumRecords(){
            int NUMRECORDS;
            int k=0;
            for (k=0;k<blocks.size();k++){
                NUMRECORDS += blocks[k].NumRecords();
            }
            return NUMRECORDS;
        }

        //to add a block to storage
        void addBlock(Blocks b){
            blocks.push_back(b);
        }

};


int main(){
    int BLOCKSIZE;
    //set disk capacity to 200MB
    //need too also set also to 500MB for Exp 7

    cout<<"Enter block size: "<<endl;
    cin >> BLOCKSIZE;

    cout<<"Storage is of block size " << BLOCKSIZE <<"bytes" <<endl;

    //open file
    string line;
    ifstream myfile;
    myfile.open("data.tsv");
    if(!myfile.is_open()){
        cout<<"Unable to read file" <<endl;
        return 0;
    }

    Storage s;
    Blocks b;
    Records r;

while(getline(myfile,line)){
    vector <string> tokens;
    istringstream iss(line);
    string token;

    while(getline(iss,token,'\t')){
        tokens.push_back(token);
    };

    s.blocks.push_back(b);
    int block_count = s.NumBlocks() -1;
    int max_records = floor(BLOCKSIZE/r.RecordSize());

    if(s.blocks[block_count].NumRecords()<max_records){
        s.blocks[block_count].addRecord(r);
    }
    else{
        block_count++;
        Blocks b;
        s.addBlock(b);
        s.blocks[block_count].addRecord(r);
    };

}

    myfile.close();

    //experiement 1
    //creating memory pool to store data on the disk
    //

    cout<< "The number of blocks is: "<< s.NumBlocks()<<endl;
    cout<< "The number of records is: "<<s.getNumRecords()<<endl;



}