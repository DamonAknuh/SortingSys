/*
 * motor_util.c
 *
 * Created: 2020-11-20 8:17:15 PM
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
#include "LCD_api.h"

/**********************************************************************
** ____ _    ____ ___  ____ _    ____
** | __ |    |  | |__] |__| |    [__
** |__] |___ |__| |__] |  | |___ ___]
**
***********************************************************************/

static uint8_t  s_CurrentMotorStep;

// ==> g_HomingFlag = True when tray is homed.
//                  else = false. 
volatile uint8_t  g_HomingFlag; 

#if ENABLE_SMALL_STEPPER

 #define MOTOR_START_DELAY_MS    (35)
 #define MOTOR_END_DELAY_MS      (20)
 #define MOTOR_STEPS_REV        (2048)
 #define MOTOR_QUARTER_STEPS    (MOTOR_STEPS_REV / 4)
 #define MOTOR_RAMP_STEPS       (MOTOR_QUARTER_STEPS / 3)
 #define MOTOR_CONS_STEPS       (MOTOR_RAMP_STEPS)
 #define MOTOR_RAMP_CONS        (MOTOR_RAMP_STEPS + MOTOR_CONS_STEPS)  

// Small Stepper step table
const uint8_t s_motorStepTable[] = 
{
    0b110000, 
    0b101000,
    0b000110, 
    0b000101
};

inline void STMotorDelayProfile(uint32_t stepNum, uint8_t quadrants)
{
    mTim1_DelayMs(MOTOR_START_DELAY_MS);
}

#elif !ENABLE_SMALL_STEPPER 

 #define MOTOR_START_DELAY_MS   (20)
 #define MOTOR_END_DELAY_MS     (5)
 #define MOTOR_STEPS_REV        (200)
 #define MOTOR_QUARTER_STEPS    (MOTOR_STEPS_REV / 4)
 #define MOTOR_RAMP_STEPS       (15)
 #define MOTOR_CONS_STEPS       (30)
 #define MOTOR_RAMP_CONS        (MOTOR_RAMP_STEPS + MOTOR_CONS_STEPS)  

// Large Stepper step table
const uint8_t s_motorStepTable[] =
{
// == > 2-Phase Full Stepper motor operation
    0b110101,   // 0b000101 | 0b110000
    0b110110,   // 0b110000 | 0b000110
    0b101110,   // 0b000110 | 0b101000
    0b101101,   // 0b101000 | 0b000101
};


// == > Store the information for the ST motor profiler
const uint8_t s_profilerStepLimits[2][3] = 
{
    {15,  45,   43},    // PROFILE 1 for moving only 1 quadrant
    {15,  95,   93}     // PROFILE 2 for moving 2 quadrants
};

inline void STMotorDelayProfile(uint32_t stepNum, uint8_t profile)
{
 #if !ENABLE_MOTOR_PROFILE
    // == > Trapezoidal Acceleration Profiling.
    if (stepNum < s_profilerStepLimits[profile][0])
    {
        mTim1_DelayMs(MOTOR_START_DELAY_MS - stepNum);
    }
    else if (stepNum < s_profilerStepLimits[profile][1])
    {
        mTim1_DelayMs(MOTOR_END_DELAY_MS);
    }
    else
    {
        mTim1_DelayMs(3 * (stepNum - s_profilerStepLimits[profile][2]));
    }
 #else 
    mTim1_DelayMs(MOTOR_START_DELAY_MS);
 #endif // ENABLE_MOTOR_PROFILE
}

#endif // ENABLE_SMALL_STEPPER

#define SIZEOF_MOTOR_TABLE    (sizeof(s_motorStepTable)/sizeof(s_motorStepTable[0])) 

/**********************************************************************
** ____ _  _ _  _ ____ ___ _ ____ _  _ ____
** |___ |  | |\ | |     |  | |  | |\ | [__
** |    |__| | \| |___  |  | |__| | \| ___]
**
***********************************************************************/

void STMotorMove(bool dirCW, uint8_t quadrants)
{
    uint32_t i;
    uint32_t steps = quadrants * MOTOR_QUARTER_STEPS;

    if (dirCW)
    {
        // == > Increment Current Step to next step. 
        s_CurrentMotorStep++;

        for (i = 0; i < steps; i++)
        {
            // the steps in the step table (1,2,3,4,1,2...) to turn motor CW
            PORTA = s_motorStepTable[(s_CurrentMotorStep + i) % SIZEOF_MOTOR_TABLE];

            STMotorDelayProfile(i, quadrants - 1);
        }

        s_CurrentMotorStep = ((s_CurrentMotorStep + i - 1) % SIZEOF_MOTOR_TABLE);
    }
    else if (!dirCW)
    {
        // == > Decrement Current Step to next step. 
        s_CurrentMotorStep = SIZEOF_MOTOR_TABLE - s_CurrentMotorStep;

        for (i = 0; i < steps; i++)
        {
            // Reverse the steps in the step table (4,3,2,1,4,3...) to turn motor CCW
            PORTA = s_motorStepTable[3 -  ((s_CurrentMotorStep + i ) % SIZEOF_MOTOR_TABLE)];

            STMotorDelayProfile(i, quadrants - 1);
        }

        s_CurrentMotorStep = 3 - ((s_CurrentMotorStep + i - 1) % SIZEOF_MOTOR_TABLE);
    }

}

void mTray_Init(void)
{
    uint16_t i;

    // == > Ensure the interrupt for the HE sensor is ON. 
    EIMSK |= _BV(INT1);

    s_CurrentMotorStep++;

    for (i = 0; !g_HomingFlag; i++)
    {
        // the steps in the step table (1,2,3,4,1,2...) to turn motor CW
        PORTA = s_motorStepTable[(s_CurrentMotorStep + i) % SIZEOF_MOTOR_TABLE];

        // == > For Homing stage just use constant delay
        mTim1_DelayMs(MOTOR_START_DELAY_MS);
    }

    s_CurrentMotorStep = ((s_CurrentMotorStep + i - 1) % SIZEOF_MOTOR_TABLE);

    // == > Turn off Interrupt Associated with Hall Effect Sensor.
    EIMSK &= ~_BV(INT1);

    // == > Display to LCD
    LCDWriteStringXY(ADC_RST_CURSOR, CURSOR_TOP_LINE, "HOMED");
    mTim1_DelayMs(750);
}

