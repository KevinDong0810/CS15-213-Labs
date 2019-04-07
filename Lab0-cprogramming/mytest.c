#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

int main(){

    queue_t * queue_ptr;
    queue_ptr = q_new();
    printf("The address of newly assigned queue is %p", queue_ptr);

    return 0;
}