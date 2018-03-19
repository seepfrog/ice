/*
	Hardware.h

	基板構成で変更するハード固有のセッティング等
　　
						2008.11.20 M.Ustumi@EasLogic
*/

#ifndef HARDWARE_H
#define HARDWARE_H

#include"p30f4011.h"

// 基板の種類
#define SC02 

// 動作クロック
//#define FOSC_7_3728MHz
#define FOSC_10MHz

// LED の設定
#define TRIS_LED 	_TRISE8
#define LED 		_LATE8
#define LED_ON() 	_LATE8 = 0
#define LED_OFF() 	_LATE8 = 1

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


void wait_us( int us  );
void wait_ms( int ms );

void blink( int t );
void ZeroMemory( void* buf, int size );

void EEP_Read( WORD* buf, int size );
void EEP_Write( WORD* buf, int size );


#endif
