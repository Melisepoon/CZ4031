# CZ4031 - Database System Principles
_This project was completed as part of the requirements for Module CZ4031: Database System Principles, in partial fulfillment of the Computer Science degree program at Nanyang Technological University (NTU)._ <br>
****

### Description of Project:
#### Project 1
This project is to design and implement the following two components of a database management system, storage and indexing.
(1) For the storage component, the following settings are assumed.
- a fraction of main memory is allocated to be used as disk storage for simplicity (and thus there is no need to worry about the buffer management); In the following, by disk storage, it refers to this allocated memory storage.
- the disk capacity could be 100 - 500 MB (depending on your machine’s main memory configuration);
- the disk storage is organized and accessed with a block as a unit; Note that since this disk storage is the allocated memory storage and can in fact be accessed with a byte/word as a unit, some mechanism needs to be designed here to simulate accesses with a block as a unit.
- the block size is 200 B;
(2) For the indexing component, the following settings are assumed.
- a B+ tree is used;
- for simplicity, the B+ tree is stored in main memory, with each node is stored in a memory region bounded by the block size (this simulates the case that each B+ tree node could be stored in a block on actual disk when
necessary);

##### Implementation and Experiments.
(1) Experiment 1: store the data (which is about IMDb movives and described in Part 4) on the disk (as specified in Part 1) and report the following statistics:
- the number of blocks;
- the size of database (in terms of MB);
(2) Experiment 2: build a B+ tree on the attribute "numVotes" by inserting the records sequentially and report the following statistics:
- the parameter n of the B+ tree;
- the number of nodes of the B+ tree;
- the height of the B+ tree, i.e., the number of levels of the B+ tree;
- the content of the root node and its 1st child node;
(3) Experiment 3: retrieve those movies with the “numVotes” equal to 500 and report the following statistics:
- the number and the content of index nodes the process accesses; (for the content, it would be sufficient to report for the first 5 index nodes or data blocks only if there are more than 5, and this applies throughout Experiment 3 and Experiment 4).
- the number and the content of data blocks the process accesses;
- the average of “averageRating’s” of the records that are returned;
(4) Experiment 4: retrieve those movies with the attribute “numVotes” from 30,000 to 40,000, both inclusively and report the following statistics:
- the number and the content of index nodes the process accesses;
- the number and the content of data blocks the process accesses;
- the average of “averageRating’s” of the records that are returned;
(5) Experiment 5: delete those movies with the attribute “numVotes” equal to 1,000, update the B+ tree accordingly, and report the following statistics:
- the number of times that a node is deleted (or two nodes are merged) during the process of the updating the B+ tree;
- the number nodes of the updated B+ tree;
- the height of the updated B+ tree;
- the content of the root node and its 1st child node of the updated B+ tree;
(6) Re-set the block size to be 500 B and re-do Experiment 1, 2, 3, 4, and 5

##### Data. 
The data contains the IMDb rating and votes information for movies
- tconst (string) - alphanumeric unique identifier of the title
- averageRating – weighted average of all the individual user ratings
- numVotes - number of votes the title has received
The first line in each file contains headers that describe what is in each column.
The data could be downloaded via this link:
https://www.dropbox.com/s/c04kfatnd9lrtx9/data.tsv?dl=0

#### Project 2 : Connecting Query with query plans
