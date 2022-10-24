//
//  Created by Huang NengQi
//  On 20/09/2022
//

#ifndef TYPES_H
#define TYPES_H

// Defines the address, size: 16B
struct Address
{
    void *blockAddress;
    short int index;
};

// Defines the strcuture for a single record, size 20B
struct Record
{
    /* data */
    char tconst[10];        // ID of the movie title
    float averageRating;    // Weighted average of all the individual user ratings
    int numVotes;           // Number of votes it has receives
};

#endif // TYPES_H
