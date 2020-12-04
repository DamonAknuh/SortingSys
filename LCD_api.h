/*
 * linkedlist_api.h
 *
 * Created: 2020-11-20 10:14:24 AM
 *  Author: matt
 */ 

#ifndef  _LCD_UTIL_H
 #define _LCD_UTIL_H

 #define _CONCAT(a,b) a##b
 #define PORT(x) _CONCAT(PORT,x)
 #define PIN(x)  _CONCAT(PIN,x)
 #define DDR(x)  _CONCAT(DDR,x)

/*_________________________________________________________________________________________*/

/************************************************
    LCD CONNECTIONS
   Uses PORTD.7 - Strobe, PORT.6 - RS, PORT.5 - RW, DataNibble PORTC.4 - PORTC.7
   Code Modified to use PORTC Only
*************************************************/

#define LCD_DATA    C       //Port PC0-PC3 are connected to D4-D7

#define LCD_E       C       //Enable OR strobe signal
#define LCD_E_POS   PC7	    //Position of enable in above port

#define LCD_RS      C
#define LCD_RS_POS  PC6

#define LCD_RW      C
#define LCD_RW_POS 	PC5


//************************************************

#define LS_BLINK    0B00000001
#define LS_ULINE    0B00000010

/***************************************************
            F U N C T I O N S
****************************************************/

void mLCD_Init      (uint8_t style);
void LCDWriteString (const char *msg);
void LCDWriteInt    (int val, unsigned int field_length);
void LCDGotoXY      (uint8_t x, uint8_t y);

//Low level
void LCDByte        (uint8_t cmd, uint8_t isData);
#define LCDCmd(c)   (LCDByte(c,0))
#define LCDData(d)  (LCDByte(d,1))

void LCDBusyLoop();


/***************************************************
            F U N C T I O N S     E N D
****************************************************/


/***************************************************
            M A C R O S
***************************************************/
#define LCDClear() LCDCmd(0b00000001)
#define LCDHome() LCDCmd(0b00000010);

#define LCDWriteStringXY(x,y,msg) do{\
 LCDGotoXY(x,y);\
 LCDWriteString(msg);\
}while(0)

#define LCDWriteIntXY(x,y,val,fl) do{\
 LCDGotoXY(x,y);\
 LCDWriteInt(val,fl);\
}while(0)

/***************************************************/

#endif // _LCD_UTIL_Hs