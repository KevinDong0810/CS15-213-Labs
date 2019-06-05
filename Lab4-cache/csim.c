#include "cachelab.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
extern char *optarg;

#define STRLEN 100
typedef unsigned int uint;

typedef struct node{
   char key;
   struct node* prev;
   struct node* next;
}Node;

typedef struct queue{
    char maxCapacity;
    Node* head;
    Node* tail;
}Queue;

typedef struct block
{
    char valid;
    uint tag;
}Block; 

typedef struct set{
    Block* blocks;
    Queue* record;
}Set;

int strReader(char* ins, uint* typePtr, char s, char b, uint* tagPtr, uint* setIndexPtr, uint* bOffsetPtr, uint* lenPtr);
Node* creatNode(char value);
Queue* createQueue(char e);
int removeNode(Node* nod);
int addNode(Node* nod, Queue* que);
int initSet(Set* setPtr, char e);
Node* getNode(char key, Queue* queuePtr);
int readWrite(Set* setPtr, uint tag, uint* hitPtr, uint* missPtr, uint* evicPtr);
int modify(Set* setPtr, uint tag, uint* hitPtr, uint* missPtr, uint* evicPtr);


int main(int argc, char ** argv){
    char s, e, b; 
    uint type, len, tag, setIndex, bOffset, i;
    uint setNum;
    uint hit = 0, miss = 0, eviction = 0;
    char ins[STRLEN];
    FILE *fp = NULL;
    int opt;

    opt=getopt(argc, argv, "s:E:b:t:");
    while(opt!= -1){
        switch(opt){
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                e = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                fp = fopen(optarg, "r");
                break;
            default:
                break;
        }
        opt = getopt( argc, argv, "s:E:b:t:" );
    }


    setNum = (int)(pow(2, s));
    Set * cache = (Set *)malloc( setNum * sizeof(Set)); 
    for (i = 0; i<setNum; i++){
        initSet(cache + i, e);
    }

    while (fgets(ins, STRLEN, fp) != NULL && ins[0] != '\n'){
        strReader(ins, &type, s, b, &tag, &setIndex, &bOffset, &len);
        if ((type == 1)|| (type == 2) )
            readWrite(cache + setIndex, tag,  &hit, &miss, &eviction);
        else{
            if (type == 3)
              modify(cache + setIndex, tag,  &hit, &miss, &eviction);
        }
    }
    printSummary(hit, miss, eviction);
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
    uint address = strtol(addP, NULL, 16); // input address
    char tagBits = 64 - s - b;
    *tagPtr = address >> (s + b);
    *setIndexPtr = (address << tagBits) >> (tagBits + b);
    *bOffsetPtr = (address << (s + tagBits)) >> (s + tagBits);

    return 1;
}

Node* creatNode(char value){
    Node* mynode = (Node *)malloc(sizeof(Node));
    mynode->key = value;
    mynode->prev = NULL;
    mynode->next = NULL;
    return mynode;
}

Queue* createQueue(char e){
    Queue* myqueue = (Queue *)malloc(sizeof(Queue));
    Node* tempNode;
    myqueue->maxCapacity = e;
    myqueue->head = creatNode(0);
    myqueue->tail = creatNode(0);
    myqueue->head->next = myqueue->tail;
    myqueue->tail->prev = myqueue->head;
    for (int i = 0; i < e; ++i){
        tempNode = creatNode(i);
        addNode(tempNode, myqueue);
    }
    return myqueue;
}

Node* getNode(char key, Queue* queuePtr){
    // search for a node with matching key
    Node* current = queuePtr->head->next;
    while(current->next != NULL){
        if (current->key == key)
            return current;
        else
            current = current->next;
    }
    return NULL;
}

int removeNode(Node* nod){
    Node* pre, * nex;
    pre = nod->prev;
    nex = nod->next;
    pre->next = nex;
    nex -> prev = pre;
    return 1;
}

int addNode(Node* nod, Queue* que){
    Node* headAfter = que->head->next;
    que->head->next = nod;
    nod->prev = que->head;
    nod->next = headAfter;
    headAfter->prev = nod;
    return 1;
}

int initSet(Set* setPtr, char e){
    // initialize every block and the queue in the set
    char i;
    Block * tempBlock;
    Queue * queuePtr = createQueue(e);
    setPtr->blocks = (Block *)malloc(e * sizeof(Block));
    for(i=0; i<e; ++i){
        tempBlock = setPtr->blocks + i;
        tempBlock->valid = 0;
        tempBlock->tag = 0;
    }
    setPtr->record = queuePtr;
    return 1;
}

int readWrite(Set* setPtr, uint tag, uint* hitPtr, uint* missPtr, uint* evicPtr){
    // read and write are equal in this simulation
    /* Step 1: search tags in all blocks; 
               if there is a match:
                 report a hit;
                 remove the tag node from the queue;
                 add the tag node into the queue;
               if there is not a match:
                 report a miss;
                 pop the tail node from the queue;
                 if the tail node's corresponding block is valid, report an eviction.
                 rewrite the tail node's block
                 add the node into the queue;
    */
   char i=0, hitTag = 0, maxCap = setPtr->record->maxCapacity;
   char hitNodeNum = 0;
   Block* blockPtr = NULL;
   Node* nodePtr = NULL;
   // searching for all tags for a match;
   for(i = 0; i<maxCap; ++i){
       blockPtr = setPtr->blocks + i;
       if ((blockPtr->tag == tag) && (blockPtr->valid)){
           hitTag = 1;
           hitNodeNum = i;
           break;
       }
   }

   if (hitTag){
       *hitPtr += 1;
       nodePtr = getNode(hitNodeNum, setPtr->record);
       assert(nodePtr);
       removeNode(nodePtr);
       addNode(nodePtr, setPtr->record);
       //printf("\t hit \n");
   }
   else{
       *missPtr += 1;
       nodePtr = setPtr->record->tail->prev;
       hitNodeNum = nodePtr->key;
       blockPtr = setPtr->blocks + hitNodeNum;
       if (blockPtr->valid){
           *evicPtr += 1;
           blockPtr->tag = tag;
           //printf("\t miss and eviction \n");
       }
       else{
           blockPtr->tag = tag;
           blockPtr->valid = 1;
           //printf("\t miss and no eviction \n");
       }
       removeNode(nodePtr);
       addNode(nodePtr, setPtr->record);
   }

   return 1;
}

int modify(Set* setPtr, uint tag, uint* hitPtr, uint* missPtr, uint* evicPtr){
    readWrite(setPtr, tag, hitPtr, missPtr, evicPtr);
    readWrite(setPtr, tag, hitPtr, missPtr, evicPtr);
    return 1;
}



