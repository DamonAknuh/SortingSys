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

#include "LCD_api.h"

/**********************************************************************
** ____ _    ____ ___  ____ _    ____
** | __ |    |  | |__] |__| |    [__
** |__] |___ |__| |__] |  | |___ ___]
**
***********************************************************************/

static uint8_t s_PrevQuadrant;

const int8_t s_trayTransitionTable[NUMBER_OF_OBJ_TYPES][NUMBER_OF_OBJ_TYPES] =
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
    LCDWriteIntXY(STATE_CURSOR, CURSOR_TOP_LINE, IDLE_STATE, STATE_CURSOR_SIZE);
#endif // ENABLE_DEBUG_BUILD

    PORTB =  DC_MOTOR_CCW;
}

/**********************************************************************
**  _ _  _ _ ___     ____ ___ ____ ___ ____ 
**  | |\ | |  |      [__   |  |__|  |  |___ 
**  | | \| |  |  ___ ___]  |  |  |  |  |___
**                                          
***********************************************************************/

void InitState()
{

    
    LCDWriteStringXY(0, 0,"Homing...");

    // == > Initialize the tray on the starting position. 
    mTray_Init();

    LCDClear(); 
    
    LCDWriteStringXY(0, 0, "Homed!");
    mTim1_DelayMs(750);

    LCDWriteStringXY(0, CURSOR_TOP_LINE, "L:--|-----|S:---");
    LCDWriteStringXY(0, CURSOR_BOT_LINE, "00S|00A|00B|00W");
    
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
    pNode_t  currentNode;
    uint16_t shadowADCResult = g_ADCMinResult;

#if ENABLE_DEBUG_BUILD
    LCDWriteIntXY(STATE_CURSOR, CURSOR_TOP_LINE, CLASS_STATE, STATE_CURSOR_SIZE);
    LCDWriteIntXY(ADC_RST_CURSOR, CURSOR_TOP_LINE, shadowADCResult, ADC_RST_CURSOR_SIZE);
#endif // ENABLE_DEBUG_BUILD

    // == > Dequeue the current node from the LL. 
    DequeueCurrentNode(&currentNode);

    // == > Check if any nodes too dequeue from LL
    if (NULL != currentNode)
    {
        // == > Classify the objects based on threshold values. increment the tracking. Update LCD
        if ( ALUM_TH_MAX >= shadowADCResult)
        {
            currentNode->data.type = ALUM_TYPE;
            s_ObjectTracking[ALUM_TYPE]++;
            LCDWriteIntXY(ALUM_CURSOR, CURSOR_BOT_LINE, s_ObjectTracking[ALUM_TYPE], OBJ_TYPES_CURSOR_SIZE);
        }
        else if((STEEL_TH_MIN <= shadowADCResult) && (STEEL_TH_MAX >= shadowADCResult))
        {
            currentNode->data.type = STEEL_TYPE;
            s_ObjectTracking[STEEL_TYPE]++;
            LCDWriteIntXY(STEEL_CURSOR, CURSOR_BOT_LINE, s_ObjectTracking[STEEL_TYPE], OBJ_TYPES_CURSOR_SIZE);
        }
        else if((BLACK_TH_MIN <= shadowADCResult) && (BLACK_TH_MAX >= shadowADCResult))
        {
            currentNode->data.type = BLACK_TYPE;
            s_ObjectTracking[BLACK_TYPE]++;
            LCDWriteIntXY(BLACK_CURSOR, CURSOR_BOT_LINE, s_ObjectTracking[BLACK_TYPE], OBJ_TYPES_CURSOR_SIZE);
        }
        else if((WHITE_TH_MIN <= shadowADCResult) && (WHITE_TH_MAX >= shadowADCResult))
        {
            currentNode->data.type = WHITE_TYPE;
            s_ObjectTracking[WHITE_TYPE]++;
            LCDWriteIntXY(WHITE_CURSOR, CURSOR_BOT_LINE, s_ObjectTracking[WHITE_TYPE], OBJ_TYPES_CURSOR_SIZE);
        }
        else
        {
            s_ObjectTracking[BLACK_TYPE]++;
        }
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
    LCDWriteIntXY(STATE_CURSOR, CURSOR_TOP_LINE, NEW_OBJ_STATE, STATE_CURSOR_SIZE);
    LCDWriteIntXY(OBJECTS_CURSOR, CURSOR_TOP_LINE, SizeOfList() + 1, OBJECTS_CURSOR_SIZE);
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
    LCDWriteIntXY(STATE_CURSOR, CURSOR_TOP_LINE, POS_TRAY_HARD, 3);
    LCDWriteIntXY(OBJECTS_CURSOR, CURSOR_TOP_LINE, SizeOfList() - 1, OBJECTS_CURSOR_SIZE);
#endif // ENABLE_DEBUG_BUILD

    pNode_t headNode;
    int8_t  quadrantsToMove;
    uint8_t nextQuadrant; 

    // == > Dequeue the head node from the LinkedList. 
    DequeueHeadNode(&headNode);

    // == > Check if node is NULL. 
    if (headNode != NULL)
    {
        // TYPE: STEEL = 00, ALUM = 01, WHITE = 10, BLACK = 11s
        nextQuadrant = headNode->data.type;

        // == > if not already on quadrant need to move stepper. 
        if (nextQuadrant != s_PrevQuadrant)
        {
            // == > Grab the tray motor from the constant table above. 
            quadrantsToMove = s_trayTransitionTable[s_PrevQuadrant][nextQuadrant];

            // == > Move stepper motor.
            STMotorMove((quadrantsToMove > 0), abs(quadrantsToMove)); 

            s_PrevQuadrant =  nextQuadrant; 
        }


        // == > If processed last node, and ramping state is asserted, set state to PAUSE_STATE.
        if ((headNode->next == NULL) && EVAL_STATE(g_CurrentState, SYSTEM_RAMP_STATE))
        {
            TRIGGER_STATE(SYSTEM_PAUSE_STATE);
        }
        else
        {
            TRIGGER_STATE(IDLE_STATE);
        }
        // == > Finished Processing Node: free it. 
        free(headNode);
    }
    // == > IF node is NULL, and ramp button has been pressed, trigger system end state. 
    else if (EVAL_STATE(g_CurrentState, SYSTEM_RAMP_STATE))
    {
        TRIGGER_STATE(SYSTEM_PAUSE_STATE);
    }
    else
    {
        TRIGGER_STATE(IDLE_STATE);
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
    LCDWriteStringXY(ADC_RST_CURSOR, CURSOR_TOP_LINE, "PAUSE");
#if ENABLE_DEBUG_BUILD
    LCDWriteIntXY(STATE_CURSOR, CURSOR_TOP_LINE, SYSTEM_PAUSE_STATE, STATE_CURSOR_SIZE);
    LCDWriteIntXY(OBJECTS_CURSOR, CURSOR_TOP_LINE, SizeOfList(), OBJECTS_CURSOR_SIZE);
#endif // ENABLE_DEBUG_BUILD

    // Turn Off DC Motor (todo brake to high VCC or turn off bottom bits )
    PORTB =  DC_MOTOR_OFF;

    mTim1_DelayMs(20);

    PORTB =  0b0000;

    while(EVAL_STATE(g_CurrentState, SYSTEM_PAUSE_STATE));

    LCDWriteStringXY(ADC_RST_CURSOR, CURSOR_TOP_LINE, "-----");
}
