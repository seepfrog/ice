/****************************************************************

	USB functions for FT245

					2009.8.21 M.Utsumi@ArcDevice
*****************************************************************/

#include"USB.h"

WORD USB_wTimeout; 
volatile WORD USB_wTimeCount=0; 
volatile BYTE USB_bStatus;

void USB_Open( void )
{
	RD = 1;
	WR = 1;
	TRIS_RD = 0;
	TRIS_WR = 0;

	TRISD = 0xFF; // ALL input
	TRISE = 0x0F | TRISE ; // RE0-RE3 input
}
 
BOOL USB_Read( BYTE* pBuffer, int size  )
{
	BYTE d;

	TRISD = 0xFF; // ALL input
	TRISE = 0x0F | TRISE ; // RE0-RE3 input

	while(size--){
		while(RXF);
		RD = 0;
		Nop();
		Nop();
		d = PORTE<<4;		
		*pBuffer++= ( (d&0xF0) | (PORTD & 0x0F) );
		RD = 1;
	}
	return(1);
}

BOOL USB_Write( BYTE* pBuffer, int size )
{
	WORD w;

	TRISD = 0xF0; // RD0-RD3 output
	TRISE = 0xF0 & TRISE ; // RE2-RE5 output

	while(size--){
		while(TXE);
		WR = 1;
		w = *pBuffer++;
		LATD = w;
		LATE = (LATE & 0xFFF0) | (0x000F & (w >> 4));
		WR = 0;
	}
	WR = 1;
	return(1);

}


