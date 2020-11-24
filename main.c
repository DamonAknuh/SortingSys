/*########################################################################
# MILESTONE:    5
# PROGRAM:        Final Project
# PROJECT:        Conveyor Belt System
# GROUP:        10
# NAME 1:        Damon, Hunka,            V00863155
# NAME 2:        Matt,  MacLeod,            V00868600
# DESC:            This program is the demo for the DC motor controller
# DATA
# REVISED
########################################################################*/

/**********************************************************************
** _ _  _ ____ _    _  _ ___  ____ ____
** | |\ | |    |    |  | |  \ |___ [__
** | | \| |___ |___ |__| |__/ |___ ___]
**
***********************************************************************/

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

volatile uint8_t  g_CurrentState;



/**********************************************************************
** _  _ ____ _ _  _
** |\/| |__| | |\ |
** |  | |  | | | \|
**
***********************************************************************/

int main(void)
{
    // ________________________________________________________
    // == > ~~ SYSTEM INITIALIZATION ~~~~~~~~~~~~~~~~~~~~~~~~~~
    // ==> Bit 7 must be changed to 1 to enable the
    //       change of CLK Pre-scalar bits
    CLKPR = _BV(CLKPCE);

    // == > Required to set CPU clock to 8 MHz
    CLKPR = 0x01;

    // == > Disable all system Interrupts
    cli();

    // == > Initialize CNT timer 1 used for busy waits
    // todo: might want to change timer to correct usage. 
    mTim1_Init();
    mTim1_DelayMs(10);

    // == > Initialize the PWM timer
    mTim0PWM_Init();

    // == > Initialize the GPIO used by the project
    // todo: change to correct GPIO
    // todo: setup all interrupts
    mGPIO_Init();

    // == > Initialize the ADC used by the project
    // todo: change to ADC1 10-bit 
    mADC1_Init();

    // == > Set Current State to INIT STATE for 
    g_CurrentState = INIT_STATE;

    // == > Enable SW interrupts
    sei();
    
    // ________________________________________________________
    // == > ~~ KERNEL LOOP ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    uint8_t shadowState;

    while (1)
    {
        shadowState = g_CurrentState;
#if ENABLE_DEBUG_BUILD
        PORTC = 0;
#endif // ENABLE_DEBUG_BUILD
        if (shadowState & CLASS_STATE)
        {
            // == > Classifying the Object. Deassert the State
            PROCESS_STATE(CLASS_STATE);

            ClassifyState();
        }
        else if (shadowState & NEW_OBJ_STATE)
        {
            // == > Processed new object deassert object
            PROCESS_STATE(NEW_OBJ_STATE);

            NewObjState();
        }
        else if (shadowState & POS_TRAY_HARD)
        {
            PROCESS_STATE(POS_TRAY_HARD);

            PositionTrayState();
            

            TRIGGER_STATE(IDLE_STATE);
        }
        else if (shadowState & INIT_STATE)
        {
            // == > Initialized System deassert object
            PROCESS_STATE(INIT_STATE);

            InitState();

            TRIGGER_STATE(IDLE_STATE);
        }
        else if (shadowState & IDLE_STATE)
        {
            PROCESS_STATE(IDLE_STATE);

            IdleState();
        }
    }

}

/**********************************************************************
** ____ _  _ _  _ ____ ___ _ ____ _  _ ____
** |___ |  | |\ | |     |  | |  | |\ | [__
** |    |__| | \| |___  |  | |__| | \| ___]
**
***********************************************************************/


