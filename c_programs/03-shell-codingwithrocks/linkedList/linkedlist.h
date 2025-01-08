#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mysh.h"

typedef struct listNode {
    listNode *next;
    job_t *job;

} listNode;

listNode *head;
int ll_size; // track size of ll 
/**
 * Takes a job and adds it to a linked list
 *  Requires a head node global?
 *  Returns:
 *  0 on success
 *  -1 on failure.
 */
int add_ll(job_t *job);
/**
 * Removes from front of list
 *  
 * Returns: 
 *  pointer to the job we removed
 */
job_t *pop();
/**
 * Removes from end of list.
 * 
 * Returns:
 * pointer to job removed from end
 */
job_t *dequeue();