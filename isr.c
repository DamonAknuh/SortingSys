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

void Project_DisplayStats()
{
    cli();
    
}

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
    TRIGGER_STATE(NEW_OBJ_STATE);
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
	TRIGGER_STATE(SAMPLING_STATE);
    if ((PIND & OR_SENSOR_PIN)== OR_SENSOR_PIN) // == > if sensor triggered : Object sighted. 
    {
		PORTC = 0xF0;
		mTim1_DelayMs(100);
		
		// == > Set Global bool for object at sensor to be true
        g_RefOBjectAtSensor = 1;
        
        // == > Reset the ADC Sampling 
        g_ADCSample = 0xFFFF; 
		
		// == > Trigger ADC Sampling.
		ADCSRA |= _BV(ADSC);
    }
    else  // == > Sensor not asserted: Object passed. 
    {
		PORTC = 0x0F;
		mTim1_DelayMs(100);
		
		// == > Set Global bool for object at sensor to be false
		g_RefOBjectAtSensor = 0;

        // Save the old ADC result
        g_ADCMinResult = g_ADCSample;

        // == > ADC Sampling Complete: State = CLASS_STATE
        TRIGGER_STATE(CLASS_STATE);
    }
}

// == > EX sensor: Optical sensor positioned at end of the conveyor belt (Active Low)
ISR(INT3_vect)
{
    TRIGGER_STATE(POS_TRAY_HARD);
}

// == > System Pause Button: Pause system (Active Low)
ISR(INT4_vect)
{
    //TRIGGER_STATE();
    // todo: or call void function.
}

// == > System Ramp Button:  (Active Low)
ISR(INT5_vect)
{
    // Todo: SYSTEM RAMP BUTTON> 


}

// == > ADC COMPL: Interrupt executed when ADC is done.
ISR(ADC_vect)
{
	uint16_t ADCLow  = ADCL;
	uint16_t ADCHigh = ADCH;
	
	uint16_t result = (ADCHigh << 8) | (ADCLow); 
	
	g_ADCSample = MIN(g_ADCSample, result);
	
	PORTC = ADCLow;
	
	if (g_RefOBjectAtSensor)
	{
		ADCSRA |= _BV(ADSC);
	}

}


ISR(BADISR_vect)
{
    PORTB = 0x00;
    PORTC = 0XFF;
	mTim1_DelayMs(1000);
	PORTC = 0x00;
	mTim1_DelayMs(500);
	PORTC = 0XFF;
	mTim1_DelayMs(1000);
	PORTC = 0x00;
	mTim1_DelayMs(500);

	PORTC = 0XFF;
    while(true);
}

