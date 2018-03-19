/*
	SCI.c

	SCI functions
				
				2008.11.20 M.Utsumi@EasLogic 
*/

#ifndef SCI_H
#define SCI_H

#include"uart.h"
#include Hardware.h"

#ifdef FOSC_10MHz
#define B9600 		129
#define B19200 		64
#define B38400 		32
#define B57600 		21
#define B115200 	10
#endif

/*
#ifdef FOSC_7_3728MHz
#define B9600 		191
#define B19200 		95
#define B38400 		47
#define B57600 		31
#define B115200 	15
#define B230400 	7
#define B460800 	3
#define B921600 	1
#endif
*/

#ifdef FOSC_7_3728MHz
#define B9600 		95
#define B19200 		47
#define B38400 		23
#define B57600 		15
#define B115200 	7
#define B230400 	3
#define B460800 	1
#define B921600 	1
#endif


/**************************************************************

		SCI1

***************************************************************/
void SCI1_Open( unsigned int rate  );
void SCI1_Close( void );
char SCI1_getch( void );
short SCI2_getw( void );
void SCI1_Read( unsigned char* buf, int size );
void SCI1_putch( char ch );
void SCI2_putw( short w );
void SCI1_Write( unsigned char* buf, int size );

extern volatile int SCI1_RXBufCount;

void SCI1_EnableBufferedRX( int priority );
char SCI1_getch_buf( void );
void SCI1_ReadBuffer( unsigned char* buf, int size );

#define UT1BF U1STA & 0x200
#define SCI1_DataReady()	DataRdyUART1()
#define SCI1_puts( _X_ ) 	putsUART1( _X_ )

/**************************************************************

		SCI2

***************************************************************/
#ifdef __30F3013_H

void SCI2_Open( unsigned int rate  );
void SCI2_Close( void );
char SCI2_getch( void );
short SCI2_getw( void );
void SCI2_Read( unsigned char* buf, int size );
void SCI2_putch( char ch );
void SCI2_putw( short w );
void SCI2_Write( unsigned char* buf, int size );

#define UT2BF U2STA & 0x200
#define SCI2_DataReady()	DataRdyUART2()
#define SCI2_puts( _X_ ) 	putsUART2( _X_ )

#endif

#endif
