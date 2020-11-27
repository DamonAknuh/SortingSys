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

// == > OI sensor: Optical sensor for first object detection. (Active Low)
ISR(INT0_vect)
{
    mTim1_DelayMs(DEBOUNCE_DELAY_MS);
    if ((PIND & OI_SENSOR_PIN) == 0x00)
    {
        TRIGGER_STATE(NEW_OBJ_STATE);
    }

}

// == > HE sensor: Hall Effect sensor for homing tray (Active Low)
ISR(INT1_vect)
{
    // == > Trigger the stepper motor to stop when homed.  
    g_HomingFlag = 1; 
}

// == > OR sensor: Optical sensor for detecting object at ADC conversion (Active Hi)
ISR(INT2_vect)
{
    if ((PIND & OR_SENSOR_PIN)== OR_SENSOR_PIN && !g_ADCCounter) // == > if sensor triggered : Object sighted, and not currently processing. 
    {
        mTim1_DelayMs(100);
        
        // == > Set Global bool for object at sensor to be true
        ADCSRA |= _BV(ADIE);  // ==> Enable ADC
        
        // == > Reset the ADC Sampling variables
        g_ADCSample = 0xFFFF; 
        g_ADCCounter= 0;
        
        // == > Trigger ADC Sampling.
        ADCSRA |= _BV(ADSC);
    }
    else if(g_ADCCounter >=  MIN_ADC_SAMPLES) // == > Sensor not asserted: Object passed. 
    {
        mTim1_DelayMs(100);
        
        // == > Set Global bool for object at sensor to be false
        ADCSRA &= ~_BV(ADIE);  // ==> Enable ADC
        ADCSRA |= _BV(ADIF);   // ==> Clear Flag in Interrupt

        // Save the old ADC result
        g_ADCMinResult = g_ADCSample;

        // == > Clear Counter
        g_ADCCounter = 0;

        // == > ADC Sampling Complete: State = CLASS_STATE
        TRIGGER_STATE(CLASS_STATE);
    }
    else    // == > Bad Reading: Not enough samples to classify object. 
    {
        // == > Clear Counter
        g_ADCCounter = 0;

        // == > Set Global bool for object at sensor to be false
        ADCSRA &= ~_BV(ADIE);  // ==> Enable ADC
        ADCSRA |= _BV(ADIF);   // ==> Clear Flag in Interrupt

        // == > Change Interrupt to be only on falling edge. 
        //EICRA ^= _BV(ISC20); 
    }
    
}


// == > EX sensor: Optical sensor positioned at end of the conveyor belt (Active Low)
ISR(INT3_vect)
{
    mTim1_DelayMs(DEBOUNCE_DELAY_MS);
    if ((PIND & EX_SENSOR_PIN) == 0x00)
    {
        TRIGGER_STATE(POS_TRAY_HARD);
    }
}

// == > System Pause Button: Pause system (Active Low)
ISR(INT4_vect)
{
    mTim1_DelayMs(DEBOUNCE_DELAY_MS);
    if ((PIND & OI_SENSOR_PIN) == 0x00)
    {
        TOGGLE_STATE(SYSTEM_PAUSE_STATE);
    }
}

// == > System Ramp Button:  (Active Low)
ISR(INT5_vect)
{
    mTim1_DelayMs(DEBOUNCE_DELAY_MS);
    if ((PIND & OI_SENSOR_PIN) == 0x00)
    {
        TRIGGER_STATE(SYSTEM_RAMP_STATE);
    }
}

// == > ADC COMPL: Interrupt executed when ADC is done.
ISR(ADC_vect)
{
    uint16_t ADCLow  = ADCL;
    // == > Reading from ADCH register resets the ADC data register. 
    //      Need to read ADCL and ADCH to local variables. 
    uint16_t ADCHigh = ADCH;
    
    uint16_t result = (ADCHigh << 8) | (ADCLow); 
    
    g_ADCSample = MIN(g_ADCSample, result);

    ADCSRA |= _BV(ADSC);
	
	g_ADCCounter++;
}


ISR(BADISR_vect)
{
    PORTC = 0xFF;
    while(true);
}

