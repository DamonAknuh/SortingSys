/*
 * isr.c
 *
 * Created: 2020-11-20 9:14:39 AM
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

volatile uint16_t g_ADCSample;
volatile uint16_t g_ADCMinResult;
volatile uint32_t g_ADCCounter; 
volatile uint8_t g_RefOBjectAtSensor; 

/**********************************************************************
** _____ _______  ______
**   |   |______ |_____/
** __|__ ______| |    \_
**
***********************************************************************/


/******************************************************************************************
** _ _  _ ___     _  _ ____ ____ ___     ____ _ 
** | |\ |  |      |  | |___ |     |      |  | | 
** | | \|  |  ___  \/  |___ |___  |  ___ |__| |                                              
**                                                                          
******************************************************************************************/
// == > OI sensor: Optical sensor for first object detection. (Active Low)
ISR(INT0_vect)
{
    if ((PIND & OI_SENSOR_PIN) == 0x00)
    {
        TRIGGER_STATE(NEW_OBJ_STATE);
    }
}


/******************************************************************************************
** _ _  _ ___     _  _ ____ ____ ___     _  _ ____ 
** | |\ |  |      |  | |___ |     |      |__| |___ 
** | | \|  |  ___  \/  |___ |___  |  ___ |  | |___ 
**                                                                          
******************************************************************************************/

// == > HE sensor: Hall Effect sensor for homing tray (Active Low)
ISR(INT1_vect)
{
    // == > Trigger the stepper motor to stop when homed.  
    g_HomingFlag = 1; 
    // == > Disable the HE sensor interrupt from firing again. 
    EIMSK &= ~_BV(INT1);
}   


/******************************************************************************************
** _ _  _ ___     _  _ ____ ____ ___     ____ ____ 
** | |\ |  |      |  | |___ |     |      |  | |__/ 
** | | \|  |  ___  \/  |___ |___  |  ___ |__| |  \ 
**                                                 
******************************************************************************************/
// == > OR sensor: Optical sensor for detecting object at ADC conversion (Active Hi)
//                  NOTE: Triggered on any edge change. 
ISR(INT2_vect)
{
    if (((PIND & OR_SENSOR_PIN) == OR_SENSOR_PIN) && !g_ADCCounter) // == > if sensor triggered : Object sighted, and not currently processing. 
    {
        // == > Enable the ADC interrupt
        ADCSRA |= _BV(ADIE);  // ==> Enable ADC
        
        // == > Reset the ADC Sampling variable
        g_ADCSample = 0xFFFF;
        
        // == > Trigger ADC Sampling.
        ADCSRA |= _BV(ADSC);
    }
    else if(g_ADCCounter >=  MIN_ADC_SAMPLES) // == > Sensor not asserted: Object passed. 
    {
        // == > Set Global bool for object at sensor to be false
        ADCSRA &= ~_BV(ADIE);  // ==> Enable ADC
        ADCSRA |= _BV(ADIF);   // ==> Clear Flag in Interrupt

        // == > Save the objects minimum ADC result for processing. 
        g_ADCMinResult = g_ADCSample;
        // == > Clear Counter
        g_ADCCounter = 0;

        // == > ADC Sampling Complete: State = CLASS_STATE
        TRIGGER_STATE(CLASS_STATE);
    } 
    else    // == > Bad Reading: Not enough samples to classify object. 
    {
        g_ADCCounter = 0;

        // == > Set Global bool for object at sensor to be false
        ADCSRA &= ~_BV(ADIE);  // ==> Enable ADC
        ADCSRA |= _BV(ADIF);   // ==> Clear Flag in Interrupt
    }
    
}

/******************************************************************************************
** _ _  _ ___     _  _ ____ ____ ___     ____ _  _ 
** | |\ |  |      |  | |___ |     |      |___  \/  
** | | \|  |  ___  \/  |___ |___  |  ___ |___ _/\_ 
**                                                 
******************************************************************************************/
// == > EX sensor: Optical sensor positioned at end of the conveyor belt (Active Low)
ISR(INT3_vect)
{
    if ((PIND & EX_SENSOR_PIN) == 0x00)
    {
        // == > Brake the DC motor to VCC
        PORTB =  DC_MOTOR_OFF;

        TRIGGER_STATE(POS_TRAY_HARD);
    }
} 

/******************************************************************************************
** _ _  _ ___     _  _ ____ ____ ___     ___  ____ _  _ ____ ____ 
** | |\ |  |      |  | |___ |     |      |__] |__| |  | [__  |___ 
** | | \|  |  ___  \/  |___ |___  |  ___ |    |  | |__| ___] |___ 
**                                                                
******************************************************************************************/
// == > System Pause Button: Pause system (Active Low)
ISR(INT4_vect)
{
    mTim1_DelayMs(50);
    if ((PINE & SYS_PAUSE_PIN) == 0x00)
    {
        // == > Brake the DC motor to VCC
        PORTB =  DC_MOTOR_OFF;

        TOGGLE_STATE(SYSTEM_PAUSE_STATE);
    }
}

/******************************************************************************************
** _ _  _ ___     _  _ ____ ____ ___     ____ ____ _  _ ___  
** | |\ |  |      |  | |___ |     |      |__/ |__| |\/| |__] 
** | | \|  |  ___  \/  |___ |___  |  ___ |  \ |  | |  | |    
**                                                           
******************************************************************************************/
// == > System Ramp Button:  (Active Low)
ISR(INT5_vect)
{
    if ((PINE & SYS_RAMP_PIN) == 0x00)
    {
        TRIGGER_STATE(SYSTEM_RAMP_STATE);
    }
}

/******************************************************************************************
** ____ ___  ____     _  _ ____ ____ ___     ____ ____ _  _ ___  
** |__| |  \ |        |  | |___ |     |      |    |  | |\/| |__] 
** |  | |__/ |___ ___  \/  |___ |___  |  ___ |___ |__| |  | |    
**                                                               
******************************************************************************************/
// == > ADC COMPL: Interrupt executed when ADC is done.
ISR(ADC_vect)
{
    uint16_t ADCLow  = ADCL;
    // == > Reading from ADCH register resets the ADC data register. 
    //      Need to read ADCL and ADCH to local variables. 
    uint16_t ADCHigh = ADCH;
    
    uint16_t result = (ADCHigh << 8) | (ADCLow); 
    
    // == > Save the current minimum value of the result
    g_ADCSample = MIN(g_ADCSample, result); // g_ADCSample = 0xFFF initally 
    
    // == > Restarts ADC Conversion 
    ADCSRA |= _BV(ADSC);

    // == > Increment global counter for each ADC sample 
    g_ADCCounter++;
}

/******************************************************************************************
** ___ _ _  _ ____ ____     ____ ____ _  _ ___  ____     _  _ ____ ____ ___ 
**  |  | |\/| |___ |__/     |    |  | |\/| |__] |__|     |  | |___ |     |  
**  |  | |  | |___ |  \ ___ |___ |__| |  | |    |  | ___  \/  |___ |___  |  
**                                                                          
******************************************************************************************/
// ==> TIM 3 COMP A: Interrupt executed when timer counts 1 second. 
ISR(TIMER3_COMPA_vect)
{
    if (g_Tim3Seconds < g_Tim3SecondsMax)
    {
        PORTC = g_Tim3Seconds;
        g_Tim3Seconds++;  
    }
    else
    {
        TRIGGER_STATE(SYSTEM_PAUSE_STATE);
        TIMSK3 &= ~_BV(OCIE3A);
    }
}

/******************************************************************************************
** ___  ____ ___  _ ____ ____     _  _ ____ ____ ___ 
** |__] |__| |  \ | [__  |__/     |  | |___ |     |  
** |__] |  | |__/ | ___] |  \ ___  \/  |___ |___  |  
**                                                   
******************************************************************************************/
ISR(BADISR_vect)
{
    PORTC = 0xFF;
    while(true);
}

