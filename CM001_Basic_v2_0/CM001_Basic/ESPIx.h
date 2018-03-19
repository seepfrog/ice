
/*
	ESPIx.h
			2008.11.20 M.Utsumi@EasLogic
*/

#ifndef ESPI_H
#define ESPI_H
#include "spi.h"
#include "Hardware.h"

#define SPI_MASTER 0
#define SPI_SLAVE  1

#ifndef SH002

#define DIR_DE		_LATF0
#define TRIS_DIR_DE	_TRISF0
#define DIR_RE		_LATF1
#define TRIS_DIR_RE	_TRISF1

#else

#define DIR_DE		_LATF4
#define TRIS_DIR_DE	_TRISF4
#define DIR_RE		_LATF5
#define TRIS_DIR_RE	_TRISF5
#define SADR		LATF
#define TRIS_SADR	TRISF

#endif

#ifdef CM001
#define CS		PORTBbits.RB2
#define TRIS_CS   _TRISB2
#else // SH001, SH002
#define CS		PORTEbits.RE4
#define TRIS_CS   _TRISE4
#endif

#define SPI_TX	1
#define SPI_RX	0
#define SPI_SCK	_RF6

#define ESPI_NORMAL  			0
#define ESPI_TX_TIMEOUT  		1
#define ESPI_RX_TIMEOUT  		2
#define ESPI_DATA_ERROR 		3
#define ESPI_OVERFLOW 			4
#define ESPI_SCK_HUNGUP		5
#define ESPI_CS_HUNGUP			6

extern WORD ESPI1_wTimeout; 
extern volatile BYTE ESPI1_bStatus; 
extern volatile BYTE ESPI1_bRxCheckSum;
extern volatile BYTE ESPI1_bTxCheckSum;
extern volatile WORD ESPI1_wTimeCount;

void ESPI1_Open( char mode );
void ESPI1_Close( void );

BOOL ESPI1_ReadEnable( void );
void ESPI1_ReadDisable( void );
BOOL ESPI1_Read( unsigned char* buf, int size );
char ESPI1_getch( void );
BOOL ESPI1_CheckSum( void );

BOOL ESPI1_IDReceiveEnable( void );
void ESPI1_IDReceiveDisable( void );
BOOL ESPI1_SetID( BYTE id );
void ESPI1_SetAdr( char ch );

void ESPI1_WriteEnable( void );
void ESPI1_WriteDisable( void );
BOOL ESPI1_Write(unsigned char* buf, int size );
BOOL ESPI1_putch( char ch  );
BOOL ESPI1_AddCheckSum( void );


#define ESPI1_DataReady()	SPI1STATbits.SPIRBF

#endif
