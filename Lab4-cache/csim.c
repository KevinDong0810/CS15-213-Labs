#include "cachelab.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#define STRLEN 100
typedef unsigned int uint;

int strReader(char* ins, uint* typePtr, char s, char b, uint* tagPtr, uint* setIndexPtr, uint* bOffsetPtr, uint* lenPtr);

int main(int argc, char ** argv)
{
    char s, e, b; 
    uint type, len, tag, setIndex, bOffset;
    FILE *fp = NULL;
    char ins[STRLEN];

    s = atoi(argv[1]);
    e = atoi(argv[2]);
    b = atoi(argv[3]);
    fp = fopen(argv[4], "r");

    typedef struct block // this structure is used to simulate a block
    {
        uint valid;
        uint tag;
        char data[b]; // every element represents a bit
    } Block;
    
    Block * cache = (Block *)malloc( (int)(pow(2, s) * e) * sizeof(Block)); 



    while (fgets(ins, STRLEN, fp) != NULL && ins[0] != '\n'){
        strReader(ins, &type, s, b, &tag, &setIndex, &bOffset, &len);
        printf("input type %d\t", type);
        printf("input tag %d\t", tag);
        printf("input set %d\t", setIndex);
        printf("input block offset %d\n", bOffset);
    }
    puts("Done.");

    return 0;
}

int strReader(char* ins, uint* typePtr, char s, char b, uint* tagPtr, uint* setIndexPtr, uint* bOffsetPtr, uint* lenPtr){    
    char* typeP = strtok(ins, " ,");
    char* addP = strtok(NULL, " ,");
    char* lenP = strtok(NULL, " ,");
    switch (*typeP)
    {
    case 'I': *typePtr = 0; return 0; break;
    case 'L': *typePtr = 1; break;
    case 'S': *typePtr = 2; break;
    case 'M': *typePtr = 3; break;
    default:
        printf("No corresponding input: %s\n", typeP);
        break;
    }

    
    *lenPtr = atoi(lenP);
    uint address = atoi(addP); // input address
    char tagBits = 64 - s - b;
    *tagPtr = address >> (s + b);
    *setIndexPtr = (address << tagBits) >> (tagBits + b);
    *bOffsetPtr = (address << (s + tagBits)) >> (s + tagBits);

    return 1;
}

