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
#define ENABLE_DEBUG_BUILD      (1)

#define ENABLE_ALL_SENSORS		(0)
// Configuration switch.
//		ENABLE_SMALL_STEPPER == true for small stepper values
//		ENABLE_SMALL_STEPPER == false for lab stepper values
#define ENABLE_SMALL_STEPPER	(1)

#define DISPLAY_DELAY_MS		(250)
#define MOTOR_START_DELAY_MS	(20)
#define MOTOR_END_DELAY_MS		(5)
#define DEBOUNCE_DELAY_MS		(150)

#define KILL_SWITCH_BIT			(0b1)
#define MOTOR_CONTR_BIT			(0b10)

#define DC_MOTOR_CCW            (0b0111)
#define DC_MOTOR_OFF            (0b0000)

#define OI_SENSOR_PIN			(0b0001)    // PD0 - Active low
#define HE_SENSOR_PIN			(0b0010)    // PD1 - Active low
#define OR_SENSOR_PIN			(0b0100)    // PD2 - Active high
#define EX_SENSOR_PIN			(0b1000)    // PD3 - Active low


#define ALUM_TH_MIN				(0)
#define ALUM_TH_MAX				(255)

#define STEEL_TH_MIN			(400)
#define STEEL_TH_MAX			(700)

#define WHITE_TH_MIN			(900)
#define WHITE_TH_MAX			(950)

#define BLACK_TH_MIN			(956)
#define BLACK_TH_MAX			(1026)

extern volatile uint8_t  g_HomingFlag;

extern volatile uint8_t  g_RefOBjectAtSensor;
extern volatile uint16_t g_ADCMinResult;
extern volatile uint16_t g_ADCSample;


#define STEEL_TYPE              (00) 
#define ALUM_TYPE               (01) 
#define WHITE_TYPE              (10) 
#define BLACK_TYPE              (11)  

#define MIN(x, y)					  (y < x ? y : x)
#define COMPILE_VERIFY(EXPR, LINE)    typedef char ERROR_ ## LINE [EXPR ? 1 : -1]


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

void mGPIO_Init(void);
void mADC1_Init(void);

void mStepMotor(bool dirCW, uint8_t quadrants);

void mTray_Init(void);

#endif /* PROJECT_H_ */

