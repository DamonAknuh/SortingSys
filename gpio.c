/*
 * gpio.c
 *
 * Created: 2020-11-20 9:32:25 AM
 *  Author: hunka
 */ 

#include <stdlib.h> // the header of the general-purpose standard library of C programming language
#include <avr/io.h>// the header of I/O port
#include <avr/interrupt.h>
#include <util/delay_basic.h>
#include <stdint.h>
#include <stdbool.h>

#include "project.h"

/**********************************************************************
** ____ _  _ _  _ ____ ___ _ ____ _  _ ____
** |___ |  | |\ | |     |  | |  | |\ | [__
** |    |__| | \| |___  |  | |__| | \| ___]
**
***********************************************************************/


void mGPIO_Init(void)
{

    // ========= SET PORT DIRECTIONS ============================
    // ==> Set PORT C to be output port.
    DDRC = 0xFF;
    
    // ==> Set PORT A to be output port. Currently using the top
    //        2 bits as debug LEDS. Rest of the bits used
    //        for stepper motor controll
    DDRA = 0xFF;
    
    // ==>    Set PWM Port PB7 to be output, bottom nibble used for
    //        DC motor controller
    DDRB = 0xFF;

    // ==>    Set PF1 to input for 10-bit ADC conversions
    DDRF = 0b00000000;

    // ==>    Set DDRD as Input port. Used for input interrupts for
    //        all the sensors. 
    DDRD = 0b11110000;

    // ==>    Set DDRE as Input port. Used for input interrupts for
    //        the system pause and system ramp buttons. 
    DDRE = 0b00000000;

    // ========= CONFIGURE INTERRUPTS ============================
    // == > Set INT0 (PORT D.0) to be interrupt on falling edge OI
    EICRA |= _BV(ISC01);
    // == > Set INT1 (PORT D.1) to be interrupt on falling edge
    EICRA |= _BV(ISC11);
    // == > Set INT2 (PORT D.2) to be interrupt on any edge. OR
    EICRA |= _BV(ISC20);
    // == > Set INT3 (PORT D.3) to be interrupt on falling edge
    EICRA |= _BV(ISC31);
    // == > Set INT4 (PORT E.3) to be interrupt on falling edge
    EICRB |= _BV(ISC41);
    // == > Set INT5 (PORT E.4) to be interrupt on falling edge
    EICRB |= _BV(ISC51);
    
    // == > Enable INT 0-5, on PD[0-3], and PE[4,5]
    EIMSK |= _BV(INT0) | _BV(INT1) | _BV(INT2) | _BV(INT3) | _BV(INT4) | _BV(INT5);
}

void mADC1_Init(void)
{
    // Todo: ramp down ADC clock. 
    g_RefOBjectAtSensor = 0; 
    
    // == > Config ADC (Analog input ADC1 / PORTF1)
    //          Manual requires ADC to run between 50kHz and 200KHz
    //          Prescale ADC CLK by / 64 to get 125kHZ
    ADCSRA |= _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1);  // ==> Enable ADC and prescale by / 64
    
    // == > ADC Multiplexer Selection Register
    //            ADLAR: Left adjust the ADC result in the register = 0. 
    //            REFS0: Set ADC voltage reference to ext cap. at AREF pin
    //            MUX0:  Set to use ADC1 input channell
    ADMUX  |= _BV(REFS0) | _BV(MUX0);

    // == > Perform First ADC conversion. This will take 25 cycles, and initializes the
    //			ADC for the rest of the program.
    ADCSRA |= _BV(ADSC);
    
	// == > Wait for first ADC conversion to completes
    while ((ADCSRA & _BV(ADIF)) == 0x00);
	
    ADCSRA |= _BV(ADIF);	// ==> Clear Flag in Interrupt
}


