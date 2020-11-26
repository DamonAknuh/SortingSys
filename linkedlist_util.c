/*
 * Util_LinkedList.c
 *
 * Created: 2020-10-15 3:21:21 PM
 *  Author: hunka
 */ 

#include <stdlib.h>
#include <stdint.h>    
#include <ASSERT.h>
#include <avr/io.h>// the header of I/O port

#include "project.h"
#include "linkedlist_api.h"

#define ASSERT(expr) assert(expr);

/**********************************************************************
** ____ _    ____ ___  ____ _    ____
** | __ |    |  | |__] |__| |    [__
** |__] |___ |__| |__] |  | |___ ___]
**
***********************************************************************/

static linkedList_t s_LinkedList;

/**********************************************************************
** ____ _  _ _  _ ____ ___ _ ____ _  _ ____
** |___ |  | |\ | |     |  | |  | |\ | [__
** |    |__| | \| |___  |  | |__| | \| ___]
**
***********************************************************************/


/**************************************************************************************
* DESC: initializes the linked queue to 'NULL' status
* INPUT: None
*/
void mLinkedList_Init(void)
{
    s_LinkedList.count = 0;
    s_LinkedList.lHead = NULL;
    s_LinkedList.lCurr = NULL;
    s_LinkedList.lTail = NULL;
}

/**************************************************************************************
* DESC: This initializes a link and returns the pointer to the new link or NULL if error
* INPUT: the new node to initialize
*/
void InitNode (pNode_t *newNode)
{
    *newNode = (pNode_t) malloc(sizeof(node_t));

    ASSERT(NULL != newNode);
        
    (*newNode)->data.bits = 0; 
    (*newNode)->next = NULL;
}

/****************************************************************************************
*  DESC: Accepts as input a new link by reference, and assigns the head and tail        
*  of the queue accordingly                
*  INPUT: The Node to enqueueu
*  NOTE: newNode needs to be initialized prior with InitNode 
*/
void EnqueueNode(pNode_t* newNode)
{
    //==> Ensure that linked list structure is initialized. 
    ASSERT(NULL != newNode);

    // if TAIL is EMPTY
    if (NULL != s_LinkedList.lTail)
    {
        s_LinkedList.lTail->next = (*newNode); 
    }
    
    // if HEAD is EMPTY
    if (NULL == s_LinkedList.lHead)
    {
        s_LinkedList.lHead = (*newNode);
        s_LinkedList.lCurr = (*newNode);
    }
    else if (NULL == s_LinkedList.lCurr)
    {
        s_LinkedList.lCurr = (*newNode);
    }

    s_LinkedList.lTail = (*newNode);    
    s_LinkedList.count++;
}


/**************************************************************************************
* DESC : Removes the link from the head of the list and assigns it to DequeueNodedLink
* INPUT: The result address
*/
void DequeueHeadNode(pNode_t * result)
{
    *result = s_LinkedList.lHead;
    
    if (NULL != s_LinkedList.lHead)
    {
        s_LinkedList.lHead = s_LinkedList.lHead->next;
        s_LinkedList.count--;
    }

    // Check if New head == NULL
    if (NULL == s_LinkedList.lHead)
    {
        s_LinkedList.lTail = NULL;
        s_LinkedList.lCurr = NULL;
    }

    return;
}

/**************************************************************************************
* DESC : Removes the link from the current of the list and assigns it to DequeueNodedLink
* INPUT: The result address
*/
void DequeueCurrentNode(pNode_t * result)
{
    *result = s_LinkedList.lCurr;
    
    if(NULL != s_LinkedList.lCurr)
    {
        s_LinkedList.lCurr = s_LinkedList.lCurr->next;    
    }
}



/**************************************************************************************
* DESC: Returns the address of the data in the linked list. 
* INPUT: None
* RETURNS: The element contained within the queue
*/
/* This simply allows you to peek at the head element of the queue and returns a NULL pointer if empty */
uint8_t GetFirstNodeValue(void)
{
    ASSERT(NULL != s_LinkedList.lHead);
    
    return s_LinkedList.lHead->data.bits;
}

/**************************************************************************************
* DESC: deallocates (frees) all the memory consumed by the Queue
* INPUT: None
*/
/* This clears the queue */
void ClearQueue(void)
{
    pNode_t tempNode;

    while (NULL != s_LinkedList.lHead)
    {
        tempNode = s_LinkedList.lHead;

        s_LinkedList.lHead = tempNode->next;

        free(tempNode);
    }

    /* Last but not least set the tail to NULL */
    s_LinkedList.lTail = NULL;
    s_LinkedList.lCurr = NULL;
    s_LinkedList.count = 0;
}

/**************************************************************************************
* DESC: Checks to see whether the queue is empty or not
* INPUT: None
* RETURNS: 1:if the queue is empty, and 0:if the queue is NOT empty
*/
/* Check to see if the queue is empty */
char IsListEmpty(void)
{
    return(s_LinkedList.lHead == NULL);
}


/**************************************************************************************
* DESC: Obtains the number of links in the queue
* INPUT: None
* RETURNS: An integer with the number of links in the queue
*/
/* returns the size of the queue*/
int SizeOfList(void)
{
    return (s_LinkedList.count);
}