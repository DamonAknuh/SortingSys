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
** ___  ____ ____ _ _  _ ____ ____
** |  \ |___ |___ | |\ | |___ [__
** |__/ |___ |    | | \| |___ ___]
**
***********************************************************************/
#define ENABLE_DEBUG_BUILD      (0)

#define ENABLE_ALL_SENSORS      (0)
// Configuration switch.
//        ENABLE_SMALL_STEPPER == true for small stepper values
//        ENABLE_SMALL_STEPPER == false for lab stepper values
#define ENABLE_SMALL_STEPPER    (0)

#define ENABLE_MOTOR_PROFILE    (0)

#define DISPLAY_DELAY_MS        (250)

#if     ENABLE_SMALL_STEPPER  
 #define MOTOR_START_DELAY_MS    (35)
 #define MOTOR_END_DELAY_MS      (20)
#else // !ENABLE_SMALL_STEPPER
 #define MOTOR_START_DELAY_MS    (20)
 #define MOTOR_END_DELAY_MS      (2)
#endif // !ENABLE_SMALL_STEPPER

#define DC_MOTOR_SPEED          (0x70)

#define DEBOUNCE_DELAY_MS       (75)

#define KILL_SWITCH_BIT         (0b1)
#define MOTOR_CONTR_BIT         (0b10)

#define DC_MOTOR_CCW            (0b0111)
#define DC_MOTOR_OFF            (0b1111)

#define OI_SENSOR_PIN           (0b0001)    // PD0 - Active low
#define HE_SENSOR_PIN           (0b0010)    // PD1 - Active low
#define OR_SENSOR_PIN           (0b0100)    // PD2 - Active high
#define EX_SENSOR_PIN           (0b1000)    // PD3 - Active low

#define SYS_PAUSE_PIN           (0b00010000)// PE4 - Active Low
#define SYS_RAMP_PIN            (0b00100000)// PE5 - Active Low

#define ST_MOTOR_CCW            (1)
#define ST_MOTOR_CW             (!ST_MOTOR_CCW)

#define ALUM_TYPE               (1) 
#define ALUM_TH_MIN             (0)
#define ALUM_TH_MAX             (255)

#define STEEL_TYPE              (3)  
#define STEEL_TH_MIN            (400)
#define STEEL_TH_MAX            (700)

#define WHITE_TYPE              (2) 
#define WHITE_TH_MIN            (900)
#define WHITE_TH_MAX            (950)

#define BLACK_TYPE              (0) 
#define BLACK_TH_MIN            (956)
#define BLACK_TH_MAX            (1026)

#define NUMBER_OF_OBJ_TYPES		(4)

#define REL_SENSOR_MAX          (BLACK_TH_MAX)
#define MIN_ADC_SAMPLES         (50)

#define CURSOR_TOP_LINE         (0)
#define CURSOR_BOT_LINE         (1)

#define OBJECTS_CURSOR          (2)
#define OBJECTS_CURSOR_SIZE     (2)

#define STATE_CURSOR            (13)
#define STATE_CURSOR_SIZE       (3)

#define ADC_RST_CURSOR          (5)
#define ADC_RST_CURSOR_SIZE     (5)


#define ALUM_CURSOR             (4)
#define STEEL_CURSOR            (0)
#define BLACK_CURSOR            (8)
#define WHITE_CURSOR            (12)

#define OBJ_TYPES_CURSOR_SIZE   (2)


#define MIN(x, y)                      (y < x ? y : x)
#define COMPILE_VERIFY(EXPR, LINE)    typedef char ERROR_ ## LINE [EXPR ? 1 : -1]

/**********************************************************************
** ____ _    ____ ___  ____ _    ____
** | __ |    |  | |__] |__| |    [__
** |__] |___ |__| |__] |  | |___ ___]
**
***********************************************************************/

extern volatile uint8_t  g_HomingFlag;
extern volatile uint8_t  g_RefOBjectAtSensor;
extern volatile uint16_t g_ADCMinResult;
extern volatile uint16_t g_ADCSample;
extern volatile uint32_t g_ADCCounter; 
extern volatile uint16_t g_Tim3Seconds; 
extern volatile uint16_t g_Tim3SecondsMax;

/**********************************************************************
** ____ _  _ _  _ ____ ___ _ ____ _  _ ____
** |___ |  | |\ | |     |  | |  | |\ | [__
** |    |__| | \| |___  |  | |__| | \| ___]
**
***********************************************************************/
void Project_ErrorState(void);


void mTim0PWM_Init(void);
void mTim1_Init(void);
void mTim1_DelayMs(uint32_t count);
void mTim1_DelayUs(double count);
void mTim3_DelayS(uint16_t count);

void mGPIO_Init(void);
void mADC1_Init(void);

void STMotorMove(bool dirCW, uint8_t quadrants);

void mTray_Init(void);

#endif /* PROJECT_H_ */

