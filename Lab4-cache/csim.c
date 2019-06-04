#include "cachelab.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

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

int main(int argc, char ** argv)
{
    char s, e, b; 
    uint type, len, tag, setIndex, bOffset;
    FILE *fp = NULL;
    char ins[STRLEN];
    Queue* queuePtr;
    s = atoi(argv[1]);
    e = atoi(argv[2]);
    b = atoi(argv[3]);
    fp = fopen(argv[4], "r");

    //Set * cache = (Set *)malloc( (int)(pow(2, s)) * sizeof(Set)); 
    queuePtr = createQueue(0);
    printf("%d\n", queuePtr->maxCapacity);

    while (fgets(ins, STRLEN, fp) != NULL && ins[0] != '\n'){
        strReader(ins, &type, s, b, &tag, &setIndex, &bOffset, &len);
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

