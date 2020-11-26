/*
 * fsm.c
 *
 * Created: 2020-11-20 11:27:42 AM
 *  Author: hunka
 */ 


#include <stdlib.h> // the header of the general-purpose standard library of C programming language
#include <avr/io.h>// the header of I/O port
#include <avr/interrupt.h>
#include <util/delay_basic.h>
#include <stdint.h>
#include <stdbool.h>

#include "project.h"
#include "linkedlist_api.h"
#include "fsm_api.h"

/**********************************************************************
** ____ _    ____ ___  ____ _    ____
** | __ |    |  | |__] |__| |    [__
** |__] |___ |__| |__] |  | |___ ___]
**
***********************************************************************/

static uint8_t s_PrevQuadrant  = 0; 

const uint8_t  s_trayTransitionTable[NUMBER_OF_OBJ_TYPES][NUMBER_OF_OBJ_TYPES] =
{ 
// Initial -->  0       1        2       3
/*  0*/       { 0,      1,       2,     -1},
/*  1*/       {-1,      0,       1,      2},
/*  2*/       { 2,     -1,       0,      1},
/*  3*/       { 1,      2,      -1,      0}
};

uint8_t s_ObjectTracking[NUMBER_OF_OBJ_TYPES];

/**********************************************************************
**  _______ _______ _______ _______ _______ _______
**  |______    |    |_____|    |    |______ |______
**  ______|    |    |     |    |    |______ ______|
** 
***********************************************************************/


/**********************************************************************
** _ ___  _    ____     ____ ___ ____ ___ ____ 
** | |  \ |    |___     [__   |  |__|  |  |___ 
** | |__/ |___ |___ ___ ___]  |  |  |  |  |___ 
**                                          
***********************************************************************/
void IdleState()
{
#if ENABLE_DEBUG_BUILD
    PORTC = IDLE_STATE;
#endif // ENABLE_DEBUG_BUILD

    PORTB =  DC_MOTOR_CCW;
}

void InitState()
{
#if ENABLE_DEBUG_BUILD
    PORTC = INIT_STATE;
#endif // ENABLE_DEBUG_BUILD

    // == > Initialize the tray on the starting position. 
    mTray_Init();
    
    // == > Turn off Interrupt Associated with Hall Effect Sensor.
    EIMSK &= ~_BV(INT1);

    // todo: see if clearQ imp is good for null Qs
    ClearQueue();

    // == > Initialize Global Variables
    g_RefOBjectAtSensor = 0; 
    g_ADCSample         = 0xFFFF;
    g_ADCCounter        = 0; 

    s_ObjectTracking[BLACK_TYPE] =0;
    s_ObjectTracking[ALUM_TYPE]  =0;
    s_ObjectTracking[WHITE_TYPE] =0;
    s_ObjectTracking[STEEL_TYPE] =0;
}

/**********************************************************************
**  ___ _    ____ ____ ____     ____ ___ ____ ___ ____ 
** |    |    |__| [__  [__      [__   |  |__|  |  |___ 
** |___ |___ |  | ___] ___] ___ ___]  |  |  |  |  |___ 
** 
***********************************************************************/

void ClassifyState()
{
#if ENABLE_DEBUG_BUILD
    PORTC = CLASS_STATE;
    mTim1_DelayMs(1000);
    PORTC = g_ADCMinResult;
    OCR0A = g_ADCMinResult >> 2;
    mTim1_DelayMs(2000);
#endif // ENABLE_DEBUG_BUILD

    pNode_t currentNode;
    uint16_t shadowADCResult = g_ADCMinResult;

    // == > Dequeue the current node from the LL. 
    DequeueCurrentNode(&currentNode);

    // == > Check if any nodes too dequeue from LL
    if (NULL != currentNode)
    {
        // == > Classify the objects based on threshold values. increment the tracking. 
        if ( ALUM_TH_MAX >= shadowADCResult)
        {
            currentNode->data.type = ALUM_TYPE;
            s_ObjectTracking[ALUM_TYPE]++;
        }
        else if(STEEL_TH_MIN <= shadowADCResult && STEEL_TH_MAX >= shadowADCResult)
        {
            currentNode->data.type = STEEL_TYPE;
            s_ObjectTracking[STEEL_TYPE]++;

        }
        else if(BLACK_TH_MIN <= shadowADCResult && BLACK_TH_MAX >= shadowADCResult)
        {
            currentNode->data.type = BLACK_TYPE;
            s_ObjectTracking[BLACK_TYPE]++;

        }
        else if(WHITE_TH_MIN <= shadowADCResult && WHITE_TH_MAX >= shadowADCResult)
        {
            currentNode->data.type = WHITE_TYPE;
            s_ObjectTracking[WHITE_TYPE]++;
        }

        // == > Increment the stage of the node. 
        currentNode->data.stage = 0b01;
    }

}

/**********************************************************************
** _  _ ____ _ _ _     ____ ___   _     ____ ___ ____ ___ ____ 
** |\ | |___ | | |     |  | |__]  |     [__   |  |__|  |  |___ 
** | \| |___ |_|_| ___ |__| |__] _| ___ ___]  |  |  |  |  |___ 
**                                                             
***********************************************************************/

void NewObjState()
{
#if ENABLE_DEBUG_BUILD
    PORTC = NEW_OBJ_STATE;
    mTim1_DelayMs(100);
#endif // ENABLE_DEBUG_BUILD

    pNode_t newNode; 

    // == > Initialze the node
    InitNode(&newNode);

    // == > Enqueue the node onto the list
    EnqueueNode(&newNode);
}


/**********************************************************************
** ___  ____ ____     ___ ____ ____ _   _     _  _ ____ ____ ___  
** |__] |  | [__       |  |__/ |__|  \_/      |__| |__| |__/ |  \ 
** |    |__| ___] ___  |  |  \ |  |   |   ___ |  | |  | |  \ |__/ 
**                                                             
***********************************************************************/

void PositionTrayState()
{
#if ENABLE_DEBUG_BUILD
    PORTC = POS_TRAY_HARD;
    mTim1_DelayMs(100);
#endif // ENABLE_DEBUG_BUILD

    pNode_t  headNode;
    int16_t  quadrantsToMove;
    uint16_t nextQuadrant; 

    // Turn Off DC Motor (todo brake to high VCC or turn off bottom bits )
    PORTB =  DC_MOTOR_OFF;

    // == > Dequeue the head node from the LinkedList. 
    DequeueHeadNode(&headNode);

    // == > Check if node is NULL. 
    if (headNode != NULL)
    {
        // == > Increment the stage of the node. 
        headNode->data.stage = 0b10;
        // TYPE: STEEL = 00, ALUM = 01, WHITE = 10, BLACK = 11s
        nextQuadrant = headNode->data.type + 1;

        // == > Finished Processing Node: free it. 
        free(headNode);

        // == > if not already on quadrant need to move stepper. 
        if (nextQuadrant != s_PrevQuadrant)
        {
            // == > Grab the tray motor from the constant table above. 
            quadrantsToMove =  s_trayTransitionTable[s_PrevQuadrant][nextQuadrant];
        
            // == > Move stepper motor.
            StepMotorMove((quadrantsToMove > 0), abs(quadrantsToMove)); 

            s_PrevQuadrant =  nextQuadrant; 
        }
    }
    // == > IF node is NULL, and ramp button has been pressed, trigger system end state. 
    else if (EVAL_STATE(SYSTEM_RAMP_STATE))
    {
        TRIGGER_STATE(SYSTEM_END_STATE);
    }
}

/******************************************************************************************
** ____ _   _ ____ ___ ____ _  _     ____ _  _ ___      ____ ___ ____ ___ ____ 
** [__   \_/  [__   |  |___ |\/|     |___ |\ | |  \     [__   |  |__|  |  |___ 
** ___]   |   ___]  |  |___ |  | ___ |___ | \| |__/ ___ ___]  |  |  |  |  |___ 
**                                                                          
******************************************************************************************/

void SystemEndState()
{
#if ENABLE_DEBUG_BUILD
    PORTC = SYSTEM_END_STATE;
    mTim1_DelayMs(100);
#endif // ENABLE_DEBUG_BUILD

    cli();

    // TODO MATT. 
    // DISPLAY STATS

    while(1);


}
