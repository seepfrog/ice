#include"p30f4011.h"
#include"uart.h"

unsigned int UMODEValue = UART_EN & UART_IDLE_CON & //UART_ALTRX_ALTTX & 
						UART_DIS_WAKE & UART_DIS_LOOPBACK & 
						UART_DIS_ABAUD & UART_NO_PAR_8BIT & UART_1STOPBIT;
unsigned int USTAValue = UART_INT_TX_BUF_EMPTY & UART_TX_PIN_NORMAL & 
						UART_TX_ENABLE & UART_INT_RX_CHAR &
						UART_ADR_DETECT_DIS & UART_RX_OVERRUN_CLEAR;		

#define UT1BF U1STA & 0x200

void SCI1_Open( unsigned int rate  )
{
	LATCbits.LATC13   = 1;	// TX High
	TRISCbits.TRISC13 = 0;	// UART ATX,ARX setup
	OpenUART1(UMODEValue, USTAValue, rate );		
}
void SCI1_Close( void )
{
	CloseUART1();
}

void SCI1_Read( unsigned char* buf, int size )
{
	while(size--){
		while(!DataRdyUART1());
		*buf++ =  ReadUART1();
	}
}

char SCI1_getch( void )
{	
	while(!DataRdyUART1());
	return( ReadUART1() );	//ÇPï∂éöéÛêMéÊìæ
}

short SCI1_getw( void )
{
    short w;

	SCI1_Read( (unsigned char*)&w, 2 );

	return( w );
}

void SCI1_Write( unsigned char* buf, int size )
{
	while(size--){
		while( UT1BF );
		WriteUART1( *buf++ );
	}
}

void SCI1_putch( char ch )
{
	while( UT1BF );
	WriteUART1(ch);	
}

void SCI1_putw( short w )
{
	SCI1_Write( (unsigned char*)&w, 2 );	
}


unsigned char RXBuf1[256];
unsigned char* pRXBufWrite1;
unsigned char* pRXBufRead1;
int RXBufCount1=0;

volatile int SCI1_RXBufCount=0;

void SCI1_EnableBufferedRX( int priority  )
{
	SCI1_RXBufCount = 0;
	pRXBufWrite1 = RXBuf1;
	pRXBufRead1   = RXBuf1;
	SetPriorityIntU1RX( priority );
	EnableIntU1RX;	
}

void _ISRFAST _U1RXInterrupt(void)
{
	char ch;
	
	while( IFS0bits.U1RXIF ){
		IFS0bits.U1RXIF = 0;
		*pRXBufWrite1++ = ReadUART1();	//ÇPï∂éöéÛêMéÊìæ
		SCI1_RXBufCount++;
		if( pRXBufWrite1 == RXBuf1+256 )	pRXBufWrite1 = RXBuf1;
	}
}

char SCI1_getch_buf( void )
{
	char ch;

	ch = *pRXBufRead1++;
	SCI1_RXBufCount--;
	if( pRXBufRead1 == RXBuf1+256 )	pRXBufRead1 = RXBuf1;
	
	return(ch);
}

void SCI1_ReadBuffer( unsigned char* buf, int size )
{
	while(size--){
		*buf++ = *pRXBufRead1++;
		SCI1_RXBufCount--;
		if( pRXBufRead1 == RXBuf1+256 )	pRXBufRead1 = RXBuf1;
	}

}


/**************************************************************

		SCI2

***************************************************************/
#ifdef __30F3013_H

unsigned int UMODE2Value = UART_EN & UART_IDLE_CON & UART_RX_TX & 
						UART_DIS_WAKE & UART_DIS_LOOPBACK & 
						UART_DIS_ABAUD & UART_NO_PAR_8BIT & UART_1STOPBIT;
unsigned int USTA2Value = UART_INT_TX_BUF_EMPTY & UART_TX_PIN_NORMAL & 
						UART_TX_ENABLE & UART_INT_RX_CHAR &
						UART_ADR_DETECT_DIS & UART_RX_OVERRUN_CLEAR;		


#define UT2BF U1STA & 0x200

void SCI2_Open( unsigned int rate  )
{
	LATFbits.LATF5   = 1;	// TX High
	TRISFbits.TRISF5 = 0;	// UART TX,RX setup
	OpenUART2(UMODE2Value, USTA2Value, rate );		
}
void SCI2_Close( void )
{
	CloseUART2();
}


char SCI2_getch( void )
{	
	while(!DataRdyUART2());
	return( ReadUART2() );	//ÇPï∂éöéÛêMéÊìæ
}

short SCI2_getw( void )
{
	char buf[2];
    short *data;

	buf[1] = SCI2_getch();
	buf[0] = SCI2_getch();
	
	data = (short*)buf;
	return( *data );
}

void SCI2_Read( unsigned char* buf, int size )
{
	while(size--){
		while(!DataRdyUART2());
		*buf++ =  ReadUART2();
	}
}

void SCI2_putch( char ch )
{
	while( UT2BF );
	WriteUART2(ch);	
}

void SCI2_putw( short w )
{
	char *buf;
	
	buf = (char*)&w;
	
	SCI2_putch( buf[1] );
	SCI2_putch( buf[0] );
}

void SCI2_Write( unsigned char* buf, int size )
{
	while(size--){
		while( UT2BF );
		WriteUART2( *buf++ );
	}
}

#endif

