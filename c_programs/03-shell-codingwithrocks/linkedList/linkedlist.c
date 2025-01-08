#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mysh.h"

typedef struct listNode {
    struct listNode *next;
    job_t *job;

} listNode;
// gcc -g -Wall -o llMSH linkededlist.c
listNode *head = NULL;
int ll_size = 0; // track size of ll 
/**
 * Adds to the front of our list. (it adds from the head)
 *  Requires a head node global?
 *  Returns:
 *  0 on success
 *  -1 on failure.
 */
int add_ll(job_t *job)
{
    listNode *newNode = (listNode *)malloc(sizeof(listNode));
    if(newNode == NULL)
    {
        fprintf(stderr, "error alloc space for newNode\n");
        exit(1);
    }
    newNode->job = job;

    if(ll_size == 0)
    {
        head = newNode;
        ll_size++;
    }
    else if (ll_size > 0)
    {
        newNode->next = head;
        head = newNode;
        ll_size++;
    }
    else {
        fprintf(stderr, "error adding node\n");
        return -1;
    }
    return 0;
}
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

void printList(){
    listNode *temp = head;

    while(temp != NULL)
    {
        printf("JID: %d\n", temp->job->jID);
        temp = temp->next;
    }
}

// int main(void)
// {
//     int counter = 0;
//     job_t jobList[5];
//     int i;

//     for(i = 0; i < 5; i++)
//     {
//         jobList[i].jID = i;
//         printf("%d\n", jobList[i].jID);
//         add_ll(&jobList[i]);
//     }
//     printList(); 
// }