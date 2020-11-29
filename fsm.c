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

// == > Transition Table: This table contains a lookup table of all the tray 
//                      movements from one quadrant to the next. 
//                      negative values are CCW, positive are CW.
const int8_t s_trayTransitionTable[NUMBER_OF_OBJ_TYPES][NUMBER_OF_OBJ_TYPES] =
{
// Initial -->  0       1        2       3
/*  0*/       { 0,      1,       2,     -1},
/*  1*/       {-1,      0,       1,      2},
/*  2*/       { 2,     -1,       0,      1},
/*  3*/       { 1,      2,      -1,      0}
};

// == > Tracks the number of objects of each type. 
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
    // == > Turn the DC motor back on: Counter Clockwise
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
    LCDClear(); 

    // == > Initialize the tray on the starting position. 
    mTray_Init();
    
    mTim1_DelayMs(750);

    LCDWriteStringXY(0, CURSOR_TOP_LINE, "L:??|-----|S:???");
    LCDWriteStringXY(0, CURSOR_BOT_LINE, "??S|??A|??B|??W");

    // == > Clear the Queue
    ClearQueue();

    // == > Initialize Global Variables
    g_RefOBjectAtSensor = 0; 
    g_ADCSample         = 0xFFFF;
    g_ADCCounter        = 0; 
    g_HomingFlag        = 0; 

    // == > Reset the object tracking information
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

    DBG_DISPLAY_LCD(ADC_RST_CURSOR, CURSOR_TOP_LINE, shadowADCResult, ADC_RST_CURSOR_SIZE);

    // == > Dequeue the current node from the LL. 
    DequeueCurrentNode(&currentNode);

    // == > Check if any nodes too dequeue from LL
    if (NULL != currentNode)
    {
        // == > Classify the objects based on threshold values. increment the tracking. Update LCD
        if ( ALUM_TH_MAX >= shadowADCResult)
        {
            currentNode->data.type = ALUM_TYPE;
        }
        else if((STEEL_TH_MIN <= shadowADCResult) && (STEEL_TH_MAX >= shadowADCResult))
        {
            currentNode->data.type = STEEL_TYPE;
        }
        else if((BLACK_TH_MIN <= shadowADCResult) && (BLACK_TH_MAX >= shadowADCResult))
        {
            currentNode->data.type = BLACK_TYPE;
        }
        else if((WHITE_TH_MIN <= shadowADCResult) && (WHITE_TH_MAX >= shadowADCResult))
        {
            currentNode->data.type = WHITE_TYPE;
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
    DBG_DISPLAY_LCD(OBJECTS_CURSOR, CURSOR_TOP_LINE, SizeOfList() + 1, OBJECTS_CURSOR_SIZE);

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
    DBG_DISPLAY_LCD(OBJECTS_CURSOR, CURSOR_TOP_LINE, SizeOfList() - 1, OBJECTS_CURSOR_SIZE);

    pNode_t headNode;
    int8_t  quadrantsToMove;
    uint8_t nextQuadrant; 
    // == > Keeps track of current quadrant persistent across function calls. 
    static uint8_t prevQuadrant;

    // == > Dequeue the head node from the LinkedList. 
    DequeueHeadNode(&headNode);

    // == > Check if node is NULL. 
    if (headNode != NULL)
    {
        // == > TYPE: STEEL = 00, ALUM = 01, WHITE = 10, BLACK = 11s
        nextQuadrant = headNode->data.type;

        // == > Increment the tracking for each object when sorting
        s_ObjectTracking[nextQuadrant]++;

        // == > if not already on quadrant need to move stepper. 
        if (nextQuadrant != prevQuadrant)
        {
            // == > Grab the tray motor from the constant table above. 
            quadrantsToMove = s_trayTransitionTable[prevQuadrant][nextQuadrant];

            // == > Move stepper motor.
            STMotorMove((quadrantsToMove > 0), abs(quadrantsToMove)); 

            // == > Update PrevQudrant for next function call
            prevQuadrant =  nextQuadrant; 
        }

        // == > If Ramping state is set then reset the timer
        if (EVAL_STATE(g_CurrentState, SYSTEM_RAMP_STATE))
        {
            // == > Reset the Ramp Down clock. 
            mTim3_DelayS(RAMP_DELAY_S);
        }
        else
        {
            // == > Trigger IDLE_STATE which will turn the motor back on. 
            TRIGGER_STATE(IDLE_STATE);
        }

        // == > Finished Processing Node: free it. 
        free(headNode);
    }
    else // == > Nothing on the list, resume idle state. 
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

    // == > Turn Off DC Motor 
    PORTB =  DC_MOTOR_OFF;

    // == > Hold brake high for 20 MS
    mTim1_DelayMs(20);

    // == > Turn off DC Motor 
    PORTB =  0b0000;

    // == > Display PAUSE STATE Statistics
    LCDWriteStringXY(ADC_RST_CURSOR, CURSOR_TOP_LINE, "PAUSE");
    // == > Display number of items on conveyor. 
    LCDWriteIntXY(OBJECTS_CURSOR, CURSOR_TOP_LINE, SizeOfList(), OBJECTS_CURSOR_SIZE);
    
    // == > Display number each type of object on bottom line. 
    LCDWriteIntXY(ALUM_CURSOR, CURSOR_BOT_LINE,  s_ObjectTracking[ALUM_TYPE],  OBJ_TYPES_CURSOR_SIZE);
    LCDWriteIntXY(STEEL_CURSOR, CURSOR_BOT_LINE, s_ObjectTracking[STEEL_TYPE], OBJ_TYPES_CURSOR_SIZE);
    LCDWriteIntXY(BLACK_CURSOR, CURSOR_BOT_LINE, s_ObjectTracking[BLACK_TYPE], OBJ_TYPES_CURSOR_SIZE);
    LCDWriteIntXY(WHITE_CURSOR, CURSOR_BOT_LINE, s_ObjectTracking[WHITE_TYPE], OBJ_TYPES_CURSOR_SIZE);

    // == > Hold this state untill the state has been deasserted elsewhere. 
    while(EVAL_STATE(g_CurrentState, SYSTEM_PAUSE_STATE));

    // == > Reset LCD display
    LCDWriteStringXY(ADC_RST_CURSOR, CURSOR_TOP_LINE, "-----");
}
