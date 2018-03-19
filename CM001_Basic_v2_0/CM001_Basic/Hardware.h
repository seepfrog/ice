/**************************************************************
	Hardware.h Hardware settings and utilitys

						2010.4.23 M.Ustumi@Arcdevice

***************************************************************/

#ifndef HARDWARE_H
#define HARDWARE_H

#include"p30f4011.h"


// 基板の種類
#define CM001 

// 動作クロック
//#define FOSC_7_3728MHz
#define FOSC_10MHz

// 入力ボードID
#define ENC_BOARD 		0
#define SENSOR_BOARD	1

// 出力ボードID
#define PWM_BOARD 		0
#define DA_BOARD		1

// 入出力ボードID
#define PIO_BOARD		2

// DAボードの種類
#define DRV_HPTECH_uSD12
//#define DRV_MAXON_DES70
//#define DRV_TITECH_JW143_2

// 入出力ポートの設定
// ポートA RB0,RB1,RB3-RB7
#define PA0_IN 		_RB0
#define PA1_IN 		_RB1
#define PA2_IN 		_RB3
#define PA3_IN 		_RB4
#define PA4_IN 		_RB5
#define PA5_IN 		_RB6
#define PA6_IN 		_RB7
#define PA7_IN 		_RB8
#define PA8_IN 		_RC13
#define PA9_IN 		_RC14

#define PA0_OUT		_LATB0
#define PA1_OUT		_LATB1
#define PA2_OUT		_LATB3
#define PA3_OUT		_LATB4
#define PA4_OUT		_LATB5
#define PA5_OUT		_LATB6
#define PA6_OUT		_LATB7
#define PA7_OUT		_LATB8
#define PA8_OUT		_LATC13
#define PA9_OUT		_LATC14

#define PA0_DIR		_TRISB0
#define PA1_DIR		_TRISB1
#define PA2_DIR		_TRISB3
#define PA3_DIR		_TRISB4
#define PA4_DIR		_TRISB5
#define PA5_DIR		_TRISB6
#define PA6_DIR		_TRISB7
#define PA7_DIR		_TRISB8
#define PA8_DIR		_TRISC13
#define PA9_DIR		_TRISC14

// ポートB RE0-RE5, RD0-RD3
#define PB0_IN 		_RE0		
#define PB1_IN 		_RE1		
#define PB2_IN 		_RE2		
#define PB3_IN 		_RE3		
#define PB4_IN 		_RE4		
#define PB5_IN 		_RE5		
#define PB6_IN 		_RD0		
#define PB7_IN 		_RD1		
#define PB8_IN 		_RD2		
#define PB9_IN 		_RD3		
#define PB10_IN 	_RF4		
#define PB11_IN 	_RF5		

#define PB0_OUT 		_LATE0	
#define PB1_OUT 		_LATE1	
#define PB2_OUT 		_LATE2	
#define PB3_OUT 		_LATE3	
#define PB4_OUT 		_LATE4	
#define PB5_OUT 		_LATE5	
#define PB6_OUT 		_LATD0	
#define PB7_OUT 		_LATD1	
#define PB8_OUT 		_LATD2	
#define PB9_OUT 		_LATD3	
#define PB10_OUT 		_LATF4	
#define PB11_OUT 		_LATF5	

#define PB0_DIR		_TRISE0
#define PB1_DIR		_TRISE1
#define PB2_DIR		_TRISE2
#define PB3_DIR		_TRISE3
#define PB4_DIR		_TRISE4
#define PB5_DIR		_TRISE5
#define PB6_DIR		_TRISD0
#define PB7_DIR		_TRISD1
#define PB8_DIR		_TRISD2
#define PB9_DIR		_TRISD3
#define PB10_DIR		_TRISF4
#define PB11_DIR		_TRISF5


// LED の設定
#define TRIS_LED 	_TRISE8
#define LED 		_LATE8
#define LED_ON() 	LED = 0
#define LED_OFF() 	LED = 1

#define TRUE 1
#define FALSE 0

#define UCHAR unsigned char
#define USHORT unsigned short 
#define UINT   unsigned int 
#define ULONG  unsigned long

#define BYTE 	unsigned char 
#define WORD 	unsigned short 
#define DWORD unsigned long 
#define BOOL int

extern BYTE bBoardID_PA;
extern BYTE bBoardID_PB;

void HW_Wait_us( WORD us  );
void HW_Wait_ms( WORD ms );
void HW_Blink( WORD t );
//void HW_ZeroMemory( void* pBuf, WORD wSize );

void FLASH_Erace_32dw( WORD wAddr );
void FLASH_Write_4dw( WORD wAddr, WORD* pwCode );
void FLASH_Read_dw( WORD wAddr, WORD* pwCode );

void EEP_Read( WORD wPage, WORD wOffset, WORD* pwBuf, WORD wSize );
void EEP_Write( WORD wPage, WORD wOffset, WORD* pwBuf, WORD wSize );


#endif
