/* 
 * Code for basic C skills diagnostic.
 * Developed for courses 15-213/18-213/15-513 by R. E. Bryant, 2017
 */

/*
 * This program implements a queue supporting both FIFO and LIFO
 * operations.
 *
 * It uses a singly-linked list to represent the set of queue elements
 */

#include <stdlib.h>
#include <stdio.h>

#include "harness.h"
#include "queue.h"

/*
  Create empty queue.
  Return NULL if could not allocate space.
*/
queue_t *q_new()
{
    /* Remember to handle the case if malloc returned NULL */
    queue_t *queue_ptr;

    queue_ptr = (queue_t *)malloc(1 * sizeof(queue_t));
    if (queue_ptr == NULL){
      fprintf(stderr, "Error - unable to allocate required memory\n");
      return NULL;
      }
    queue_ptr->head = NULL;
    queue_ptr->tail = NULL;
    queue_ptr->q_size = 0;

    return queue_ptr;
}

/* Free all storage used by queue */
void q_free(queue_t *q)
{
    /* Remember to free the queue structue and list elements */
    if (q == NULL) return;
    list_ele_t *p = q->head, *cur;

    while(p != NULL){
      cur = p->next;
      free(p);
      p = cur;
    }
    q->head = NULL;
    q->tail = NULL;

    free(q);
}

/*
  Attempt to insert element at head of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
 */
bool q_insert_head(queue_t *q, int v)
{
    /* What should you do if the q is NULL? */
    if (q == NULL) return false;
    list_ele_t *ele = (list_ele_t *)malloc(sizeof(list_ele_t));
    /* What if malloc returned NULL? */
    if (ele == NULL) return false;

    ele->value = v;
    ele->next = q->head;
    q->head = ele;
    /* if queue is empty, also update the tail pointer */
    if (q->q_size == 0) q->tail = ele;
    q->q_size += 1;
    return true;
}


/*
  Attempt to insert element at tail of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
 */
bool q_insert_tail(queue_t *q, int v)
{
    if (q == NULL) return false;
    list_ele_t *ele = (list_ele_t *)malloc(sizeof(list_ele_t));
    /* What if malloc returned NULL? */
    if (ele == NULL) return false;

    ele->value = v;
    ele->next = NULL;
    if (q->q_size == 0) {
      q->head = ele;
      q->tail = ele;
    }
    else{
      q->tail->next = ele;
      q->tail = ele;
    }
    q->q_size += 1;
    /* Remember: It should operate in O(1) time */
    return true;
}

/*
  Attempt to remove element from head of queue.
  Return true if successful.
  Return false if queue is NULL or empty.
  If vp non-NULL and element removed, store removed value at *vp.
  Any unused storage should be freed
*/
bool q_remove_head(queue_t *q, int *vp)
{
    if (q == NULL || q->q_size == 0) return false;
    list_ele_t * old_head = q->head;
    if (vp != NULL){
      *vp = old_head->value;
    }
    q->head = old_head->next;
    free(old_head);
    old_head = NULL;

    if (q->q_size == 1){
      q->tail = NULL;
    }
    q->q_size -= 1;
    return true;
}

/*
  Return number of elements in queue.
  Return 0 if q is NULL or empty
 */
int q_size(queue_t *q)
{
    /* Remember: It should operate in O(1) time */
    if (q == NULL) return 0;

    return q->q_size;
}

/*
  Reverse elements in queue.

  Your implementation must not allocate or free any elements (e.g., by
  calling q_insert_head or q_remove_head).  Instead, it should modify
  the pointers in the existing data structure.
 */
void q_reverse(queue_t *q)
{
  if (q == NULL || q->q_size <= 1) return;
  list_ele_t *prev= q->head, *cur = prev->next, *next;
  prev->next = NULL;
  while (cur != NULL){
    next = cur -> next;
    cur -> next = prev;
    prev = cur;
    cur = next;
  }
  list_ele_t * temp;
  temp = q->head;
  q->head = q->tail;
  q->tail = temp;

  return;
}

