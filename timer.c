/*
 * timer.c
 *
 * Created: 2020-11-20 9:30:06 AM
 *  Author: hunka
 */ 



#include <stdlib.h> // the header of the general-purpose standard library of C programming language
#include <avr/io.h>// the header of I/O port
#include <avr/interrupt.h>
#include <util/delay_basic.h>
#include <stdint.h>
#include <stdbool.h>

#include "project.h"

volatile uint16_t g_Tim3CounterS; 
volatile uint16_t g_Tim3MaxS;

/**********************************************************************
** ____ _  _ _  _ ____ ___ _ ____ _  _ ____
** |___ |  | |\ | |     |  | |  | |\ | [__
** |    |__| | \| |___  |  | |__| | \| ___]
**
***********************************************************************/

void mTim1_Init()
{

    // == > Sets waveform Generation Bit Mode to PWM,
    //        CTC mode for count up (mode 4)
    //        TCCR = Timer Counter Control Register 1 B
    TCCR1B |= _BV(WGM12);

    // == > Set Output Compare Register = 1000 cycles = 1ms = 0x03E8
    //        Value is continuous compared with counter value
    //        Match generates an Output compare interrupt.
    OCR1A = 0x03E8;

    // == > Set pre-scalar to 8 in B timer control register
    // Need to transfer line to next file
    // Timer Counter Control Register
    TCCR1B |= _BV(CS11);

}

// == > Busy Wait Timer 1
void mTim1_DelayMs(uint32_t count)
{
    uint32_t index = 0;

    // == > Set Timer Counter Initial Value = 0x0000
    TCNT1 &= 0x0;
    
    // == > Timer Interrupt Flag Register:
    //        OCF01 is cleared when writing the following statement.
    //        Needs SW as flag is HW controlled, and needs SW toggle
    TIFR1 |= ( 1 << OCF1A);
    
    // == > Poll the timer to determine when the timer is 1000mS
    while ( index < count )
    {
        // == > Bit 2 set when TCNT1 = OCR1A (Output Compare Register)
        if ( TIFR1 & 0x02 )
        {
            TIFR1 |= ( 1 << OCF1A);
            index++;
        }
    }
}

void mTim3_Init()
{
    TCCR3B |= _BV(CS32) | _BV(CS30);

    // == > Sets waveform Generation Bit Mode to PWM,
    //        CTC mode for count up (mode 4)
    //        TCCR = Timer Counter Control Register 1 B
    TCCR3B |= _BV(WGM12);

    // == > Set Output Compare Register = 1000 cycles = 1ms = 0x03E8
    //        Value is continuous compared with counter value
    //        Match generates an Output compare interrupt.
    OCR3A = 7812;
}

// == > Interrupt Driven Timer 3
void mTim3_SetWatchDogS(uint16_t count)
{
    // == > Reset the global variables associated with Tim3Seconds
    g_Tim3CounterS = 0; 
    // == > Set max count to track
    g_Tim3MaxS = count;

    // == > Set Timer Counter Initial Value = 0x0000
    TCNT3 &= 0x0;
    
    // == > Timer Interrupt Flag Register:
    //        OCF01 is cleared when writing the following statement.
    //        Needs SW as flag is HW controlled, and needs SW toggle
    TIFR3 |= ( 1 << OCF3A);

    // == > Timer Interrupt Mask Register:
    //      Enable on the interrupt for the timer
    TIMSK3 |= (_BV(OCIE3A));
}


void mTim0PWM_Init()
{
    TCCR0A |= (_BV(WGM01) | _BV(WGM00));
    
    // Set the compare match output mode  to clear (change to 0) on a compare match +
    // set the output compare A (change to 1) when the timer reaches TOP
    // (the value of the timer that you wanted to match the output compare A register to) (Hint, we are using Fast PWM mode).
    TCCR0A |= _BV(COM0A1); // COM0A0 == 0
    
    // Set the Clock Select bits (clock pre-scaler) in the Timer/Counter Control
    // Register B (pg 129 and onward) to a reasonable value for the period of the
    // PWM (T) signal for now, and experiment by changing this value later. Note,
    // the ranges of PWM that the Pololu motor driver can accept is up to 20 kHz
    TCCR0B |= (_BV(CS01) | _BV(CS00));
    
    // Set the duty cycle to  %
    OCR0A  = DC_MOTOR_SPEED;
}

// == > 1 us timer for LCD 
//       f = 16MHz Systtem clock/ 2 ( prescaler) 
//       T = 1/ f =  125 ns * 8 = 1 us 
void mTim_DelayUs(double count)
{
    uint32_t index = 0;
    
    while ( index < (8 * count))
    {
        index++;
    }
}

