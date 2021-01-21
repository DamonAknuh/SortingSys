/*
 * project.h
 *
 * Created: 2020-11-20 9:17:46 AM
 *  Author: hunka
 */ 


#ifndef PROJECT_H_
#define PROJECT_H_

#include <stdint.h>
#include <stdbool.h>

/**********************************************************************
** ____ ____ _  _ ____ _ ____ _  _ ____ ____ ___ _ ____ _  _ ____ 
** |    |  | |\ | |___ | | __ |  | |__/ |__|  |  | |  | |\ | [__  
** |___ |__| | \| |    | |__] |__| |  \ |  |  |  | |__| | \| ___] 
**                                                                
***********************************************************************/
//===============> Configuration switches.

//        ENABLE_SMALL_STEPPER == true for small stepper values
//        ENABLE_SMALL_STEPPER == false for lab stepper values
#define ENABLE_SMALL_STEPPER    (0)

//        ENABLE_MOTOR_PROFILE == true to enable delay profiling for stepper
//        ENABLE_MOTOR_PROFILE == false to have constant delay for stepper
#define ENABLE_MOTOR_PROFILE    (0)

//        ENABLE_DEBUG_BUILD == true to enable debug builds, debug builds output further information to LCD.
//        ENABLE_DEBUG_BUILD == false to enable production builds for best performance
#define ENABLE_DEBUG_BUILD      (1)

/**********************************************************************
** ___  ____ ____ _ _  _ ____ ____
** |  \ |___ |___ | |\ | |___ [__
** |__/ |___ |    | | \| |___ ___]
** 
***********************************************************************/

#if ENABLE_DEBUG_BUILD
 
 #define DBG_DISPLAY_STATE_LCD(STATE)                                           \
    do                                                                          \
    {                                                                           \
        LCDWriteIntXY(STATE_CURSOR, CURSOR_TOP_LINE, STATE, STATE_CURSOR_SIZE); \
    }                                                                           \
    while(0)
 #define DBG_DISPLAY_LCD(X,Y,VAL,SIZE)                                          \
    do                                                                          \
    {                                                                           \
        LCDWriteIntXY(X, Y, VAL, SIZE);                                         \
    }                                                                           \
    while(0)
#else

 #define DBG_DISPLAY_LCD(X,Y,VAL,SIZE)
 #define DBG_DISPLAY_STATE_LCD(STATE)

#endif // ENABLE_DEBUG_BUILD

#define DC_MOTOR_SPEED          (0x70)

#define DEBOUNCE_DELAY_MS       (50)
#define RAMP_DELAY_S            (6)

#define DC_MOTOR_CCW            (0b0111)
#define DC_MOTOR_BRAKE          (0b1111)
#define DC_MOTOR_OFF            (0b0000)

#define OI_SENSOR_PIN           (0b0001)    // PD0 - Active low
#define HE_SENSOR_PIN           (0b0010)    // PD1 - Active low
#define OR_SENSOR_PIN           (0b0100)    // PD2 - Active high
#define EX_SENSOR_PIN           (0b1000)    // PD3 - Active low

#define SYS_PAUSE_PIN           (0b00010000)// PE4 - Active Low
#define SYS_RAMP_PIN            (0b00100000)// PE5 - Active Low

// == > ALUM TYPE INFORMATION
#define ALUM_TYPE               (1) 
#define ALUM_TH_MIN             (0)
#define ALUM_TH_MAX             (255)

// == > STEEL TYPE INFORMATION
#define STEEL_TYPE              (3)  
#define STEEL_TH_MIN            (400)
#define STEEL_TH_MAX            (700)

// == > WHITE TYPE INFORMATION
#define WHITE_TYPE              (2) 
#define WHITE_TH_MIN            (900)
#define WHITE_TH_MAX            (950)

// == > BLACK TYPE INFORMATION
#define BLACK_TYPE              (0) 
#define BLACK_TH_MIN            (956)
#define BLACK_TH_MAX            (1026)

#define NUMBER_OF_OBJ_TYPES     (4)

#define REL_SENSOR_MAX          (BLACK_TH_MAX)
#define MIN_ADC_SAMPLES         (50)

// ================= > LCD DISPLAY DEFINES
#define CURSOR_TOP_LINE         (0)
#define CURSOR_BOT_LINE         (1)

#define OBJECTS_CURSOR          (2)
#define OBJECTS_CURSOR_SIZE     (2)

#define STATE_CURSOR            (13)
#define STATE_CURSOR_SIZE       (3)

#define ADC_RST_CURSOR          (5)
#define ADC_RST_CURSOR_SIZE     (5)

#define STEEL_CURSOR            (0)
#define ALUM_CURSOR             (4)
#define BLACK_CURSOR            (8)
#define WHITE_CURSOR            (12)

#define OBJ_TYPES_CURSOR_SIZE   (2)

// == > UTIL MACROS
#define MIN(x, y)                      (y < x ? y : x)
#define COMPILE_VERIFY(EXPR, LINE)    typedef char ERROR_ ## LINE [EXPR ? 1 : -1]

/**********************************************************************
** ____ _    ____ ___  ____ _    ____
** | __ |    |  | |__] |__| |    [__
** |__] |___ |__| |__] |  | |___ ___]
**
***********************************************************************/

extern volatile uint8_t  g_ObjectTracking[NUMBER_OF_OBJ_TYPES];
extern volatile uint8_t  g_HomingFlag;
extern volatile uint8_t  g_RefOBjectAtSensor;
extern volatile uint16_t g_ADCMinResult;
extern volatile uint16_t g_ADCSample;
extern volatile uint32_t g_ADCCounter; 
extern volatile uint16_t g_Tim3CounterS; 
extern volatile uint16_t g_Tim3MaxS;

/**********************************************************************
** ____ _  _ _  _ ____ ___ _ ____ _  _ ____
** |___ |  | |\ | |     |  | |  | |\ | [__
** |    |__| | \| |___  |  | |__| | \| ___]
**
***********************************************************************/

void mTim0PWM_Init(void);
void mTim1_Init(void);
void mTim3_Init(void);
void mTim1_DelayMs(uint32_t count);
void mTim_DelayUs(double count);
void mTim3_SetWatchDogS(uint16_t count);

void mGPIO_Init(void);
void mADC1_Init(void);

void STMotorMove(bool dirCW, uint8_t quadrants);
void mTray_Init(void);

#endif /* PROJECT_H_ */

