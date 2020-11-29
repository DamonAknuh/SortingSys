/*########################################################################
# MILESTONE:    5
# PROGRAM:      Final Project
# PROJECT:      Conveyor Belt System
# GROUP:        B01: 10 * (group 1 in one drive)
# NAME 1:       Damon, Hunka,            V00863155
# NAME 2:       Matt,  MacLeod,          V00868600
# DESC:         This program is the demo for the DC motor controller
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
#include "LCD_api.h"

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
    mTim1_Init();
    mTim1_DelayMs(10);

    // == > Initialize CNT timer 3 used for interrupt driven waits
    mTim3_Init();

    // == > Initialize the PWM timer
    mTim0PWM_Init();

    // == > Initialize the GPIO used by the project
    mGPIO_Init();

    // == > Initialize the ADC used by the project
    mADC1_Init();

    // == > Initailize the LCD for the project
    mLCD_Init(LS_BLINK | LS_ULINE);

    // == > Clear the screen
    LCDClear();

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

        if (EVAL_STATE(shadowState, SYSTEM_PAUSE_STATE))
        {
            DBG_DISPLAY_STATE_LCD(SYSTEM_PAUSE_STATE);

            SystemEndState();
        }

        if (EVAL_STATE(shadowState, POS_TRAY_HARD))
        {
            DBG_DISPLAY_STATE_LCD(POS_TRAY_HARD);

            // == > Positioning the object. Deassert the State
            PROCESS_STATE(POS_TRAY_HARD);

            PositionTrayState();
        }

        if (EVAL_STATE(shadowState, CLASS_STATE))
        {
            DBG_DISPLAY_STATE_LCD(CLASS_STATE);

            // == > Classifying the Object. Deassert the State
            PROCESS_STATE(CLASS_STATE);

            ClassifyState();
        }

        if (EVAL_STATE(shadowState, NEW_OBJ_STATE))
        {
            DBG_DISPLAY_STATE_LCD(NEW_OBJ_STATE);

            // == > Processed new object deassert object
            PROCESS_STATE(NEW_OBJ_STATE);

            NewObjState();
        }

        if (EVAL_STATE(shadowState, INIT_STATE))
        {
            DBG_DISPLAY_STATE_LCD(INIT_STATE);

            // == > Initialized System deassert object
            PROCESS_STATE(INIT_STATE);

            InitState();
        }
    }

}


