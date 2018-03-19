/****************************************************************

	USB functions for FT245

					2009.8.21 M.Utsumi@ArcDevice
*****************************************************************/

#ifndef USB_H
#define USB_H

#include "hardware.h"

#define RD	_LATE5
#define WR	_LATE4
#define RXF	PORTFbits.RF0
#define TXE	PORTFbits.RF1

#define TRIS_RD	TRISEbits.TRISE5
#define TRIS_WR	TRISEbits.TRISE4

/*
D0-D3	RD0-RD3
D4-D7	RE0-RE7
*/

#define USB_NORMAL  		0
#define USB_TIMEOUT  		1

extern WORD USB_wTimeout; 
extern volatile WORD USB_wTimeCount; 
extern volatile BYTE USB_bStatus;

void USB_Open( void );
BOOL USB_Write( BYTE* pBuffer, int size );
BOOL USB_Read( BYTE* pBuffer, int size );

#define USB_RxReady() RXF==0

#endif
