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

uint8_t s_PrevQuadrant  = 1; 


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
}

void ClassifyState()
{
#if ENABLE_DEBUG_BUILD
    PORTC = CLASS_STATE;
    mTim1_DelayMs(1000);
	PORTC = g_ADCMinResult;
	mTim1_DelayMs(2000);
#endif // ENABLE_DEBUG_BUILD

    pNode_t currentNode;
    uint16_t shadowADCResult = g_ADCMinResult;

    DequeueCurrentNode(&currentNode);
   
    if ( ALUM_TH_MAX >= shadowADCResult)
    {
        currentNode->data.type = 0b01;
    }
    else if(STEEL_TH_MIN <= shadowADCResult && STEEL_TH_MAX >= shadowADCResult)
    {
        currentNode->data.type = 0b00;
    }
    else if(BLACK_TH_MIN <= shadowADCResult && BLACK_TH_MAX >= shadowADCResult)
    {
        currentNode->data.type = 0b11;
    }
    else if(WHITE_TH_MIN <= shadowADCResult && WHITE_TH_MAX >= shadowADCResult)
    {
        currentNode->data.type = 0b10;
    }

    currentNode->data.stage = 0b01;

}

void NewObjState()
{
#if ENABLE_DEBUG_BUILD
    PORTC = NEW_OBJ_STATE;
    mTim1_DelayMs(100);
#endif // ENABLE_DEBUG_BUILD
    pNode_t newNode; 

    InitNode(&newNode);

    EnqueueNode(&newNode);
}

    // link stores classification. look at structure in link list .h file. 
    // need to track tray pos over movement.
    // need to call void mStepMotor(bool dirCW, uint32_t quadrants)

void PositionTrayState()
{
#if ENABLE_DEBUG_BUILD
    PORTC = POS_TRAY_HARD;
    mTim1_DelayMs(100);
#endif // ENABLE_DEBUG_BUILD

#if !ENABLE_DEBUG_BUILD
    pNode_t headNode;
    uint8_t classification; 

    // Turn Off DC Motor (todo brake to high VCC or turn off bottom bits )
    PORTB =  DC_MOTOR_OFF;

    // Logic to grab LL value from LL
    DequeueHeadNode(&headNode);

    headNode->data.stage = 0b10;

    // TYPE: STEEL = 00, ALUM = 01, WHITE = 10, BLACK = 11s
    type = headNode->data.type;

    uint8_t next_quatrant; 

    
    if (type == STEEL_TYPE){ // 0x00 

        next_quatrant =  1; 
    }
    else if (type == ALUM_TYPE){  //0x01 
        
        next_quatrant =  2;    
    }
    else if (type == WHITE_TYPE ){  //0x10 

        next_quatrant =  3; 
    }
    else if (type == BLACK_TYPE ){  //0x11

        next_quatrant =  4;         
    }

    bool clockwise = 1, counter_CW = 0 ; 

    
    if (( (next_quadrant  - s_PrevQuadrant)  % 3) < 2){ 

        mStepMotor(clockwise, next_quadrant); 

        s_PrevQuadrant =  next_quadrant; 



    } 
    else { 

        mStepMotor(counter_CW, next_quadrant); 

        s_PrevQuadrant =  next_quadrant; 

    }
#endif //!ENABLE_DEBUG_BUILD
}
