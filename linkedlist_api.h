/*
 * linkedlist_api.h
 *
 * Created: 2020-11-20 10:14:24 AM
 *  Author: hunka
 */ 


#ifndef LINKEDLIST_API_H_
#define LINKEDLIST_API_H_
#include "project.h"
#include "stdint.h"

/**********************************************************************
** ___  ____ ____ _ _  _ ____ ____
** |  \ |___ |___ | |\ | |___ [__
** |__/ |___ |    | | \| |___ ___]
**
***********************************************************************/

typedef union
{
	struct
	{
		uint8_t type    : 2; // == > TYPE: STEEL = 00, ALUM = 01, WHITE = 10, BLACK = 11s
		uint8_t resv    : 2; // == > RESERVED
		uint8_t stage   : 4; // == > Stage, 3 stages, NEW = 00, CLASSED = 01, PROCESSED = 10
	};

	uint8_t    bits;	
}nodeData_t;

typedef struct node_s
{
    nodeData_t data;
    struct node_s * next; 
}node_t, *pNode_t;

typedef struct
{
    uint8_t count; 
    pNode_t lHead;
    pNode_t lCurr;
    pNode_t lTail;
}linkedList_t, * pLinkedList_t;

COMPILE_VERIFY(sizeof(nodeData_t) == 1, nodeData_t);
COMPILE_VERIFY(sizeof(pNode_t) == 2,	pNode_t);
COMPILE_VERIFY(sizeof(node_t) == 3,		node_t);

/**********************************************************************
** ____ _  _ _  _ ____ ___ _ ____ _  _ ____
** |___ |  | |\ | |     |  | |  | |\ | [__
** |    |__| | \| |___  |  | |__| | \| ___]
**
***********************************************************************/

void    InitNode            (pNode_t * newNode);
void    EnqueueNode         (pNode_t * newNode);
void    DequeueHeadNode     (pNode_t * result);
void    DequeueCurrentNode  (pNode_t * result);
void    mLinkedList_Init    (void);
void    ClearQueue          (void);
uint8_t GetFirstNodeValue   (void);
char    IsListEmpty         (void);
int     SizeOfList          (void);



#endif /* LINKEDLIST_API_H_ */
