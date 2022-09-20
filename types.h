#ifndef TYPES_H
#define TYPES_H

// Defines the address for a record
struct Address
{
    void *blockAddress;
    short int index;
};

// Defines the strcuture for a single record
struct Record
{
    /* data */
    char tconst[10];        // ID of the movie title
    float averageRating;    // Weighted average of all the individual user ratings
    int numVotes;           // Number of votes it has receives
};

#endif // TYPES_H