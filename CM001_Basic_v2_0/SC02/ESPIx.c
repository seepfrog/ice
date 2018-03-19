/*
	ESPIx.c

			2008.11.20 M.Utsumi@EasLogic

*/

#include"ESPIx.h"

WORD ESPI1_wTimeout; 
volatile BYTE ESPI1_bStatus=0; 
volatile BYTE ESPI1_bRxCheckSum=0;
volatile BYTE ESPI1_bTxCheckSum=0;
volatile BYTE ESPI_ID = 0; // default
volatile WORD ESPI1_wTimeCount=0;


void ESPI1_Open( char mode )
{
	unsigned int SPICONValue;
	unsigned int SPISTATValue = SPI_ENABLE & SPI_IDLE_CON & 
			SPI_RX_OVFLOW_CLR;
	
	switch( mode ){
	case SPI_MASTER:
		ADPCFGbits.PCFG2 = 1;
		TRIS_DIR_DE = 0;
		TRIS_DIR_RE = 0;
		DIR_DE = SPI_RX;
		DIR_RE = SPI_RX;
		TRIS_CS = 0;
		CS = 1;
		
		SPICONValue = FRAME_ENABLE_OFF & FRAME_SYNC_OUTPUT &
			ENABLE_SDO_PIN & SPI_MODE16_OFF & SPI_SMP_OFF & SPI_CKE_OFF
			&  CLK_POL_ACTIVE_HIGH & SLAVE_ENABLE_OFF &
			MASTER_ENABLE_OFF & SEC_PRESCAL_2_1 & PRI_PRESCAL_1_1;
		
		break;
	case SPI_SLAVE:
		ADPCFGbits.PCFG2 = 1;
		TRIS_DIR_DE = 0;
		TRIS_DIR_RE = 0;
		DIR_DE = SPI_RX;
		DIR_RE = SPI_RX;
		TRIS_CS = 1;		
		CS = 1;
		
		SPICONValue = FRAME_ENABLE_OFF & FRAME_SYNC_OUTPUT &
			ENABLE_SDO_PIN & SPI_MODE16_OFF & SPI_SMP_OFF & SPI_CKE_OFF
			&  CLK_POL_ACTIVE_HIGH & SLAVE_ENABLE_ON &
			MASTER_ENABLE_OFF & SEC_PRESCAL_2_1 & PRI_PRESCAL_1_1;
		
		break;
	}
	OpenSPI1(SPICONValue, SPISTATValue);
	SPI1STATbits.SPIEN =0; // disable

	ESPI1_bStatus = ESPI_NORMAL;
	ESPI1_wTimeout = 100; // ms

#ifdef SH002

	ESPI1_SetAdr( 0 );
	TRIS_SADR = TRIS_SADR & 0xFC;	// Output
#endif

}

void ESPI1_Close( void )
{
	CloseSPI1();
}

inline BOOL WaitTxBufReady( void )
{
	ESPI1_wTimeCount = 0;
	while( SPI1STATbits.SPITBF ){
		if( ESPI1_wTimeCount > ESPI1_wTimeout ){
			ESPI1_bStatus = ESPI_TX_TIMEOUT;
			return( FALSE );	
		}
	}
	return( TRUE );
}

inline BOOL WaitRxBufReady( void )
{
	ESPI1_wTimeCount = 0;
	while( !SPI1STATbits.SPIRBF ){
		if( ESPI1_wTimeCount > ESPI1_wTimeout ){
			ESPI1_bStatus = ESPI_RX_TIMEOUT;
			return( FALSE );	
		}
	}
	return( TRUE );
}

BOOL ESPI1_ReadEnable( void )
{
	ESPI1_wTimeCount = 0;

	while(SPI_SCK){
		if( ESPI1_wTimeCount > ESPI1_wTimeout ){
			ESPI1_bStatus = ESPI_SCK_HUNGUP;
			return( FALSE );	
		}
	}

	SPI1STATbits.SPIEN =1;

	return( TRUE );
}

void ESPI1_ReadDisable( void )
{
	SPI1STATbits.SPIEN =0;
}


BOOL ESPI1_Read( unsigned char* buf, int size )
{
	unsigned char dumy;

	while(size-->0){
		if( !WaitTxBufReady() ){
			ESPI1_ReadDisable();
			return( FALSE );
		}
		SPI1BUF = dumy;
		if( !WaitRxBufReady() ){
			ESPI1_ReadDisable();
			return( FALSE );
		}
		*buf = SPI1BUF;
		ESPI1_bRxCheckSum += *buf++;

	}
	ESPI1_bStatus = ESPI_NORMAL;
	return( TRUE );
}

char ESPI1_getch( void )
{
	unsigned char ch;
	
	if( !WaitTxBufReady() ){
		ESPI1_ReadDisable();
		return( FALSE );
	}
	SPI1BUF = ch;
	if( !WaitRxBufReady() ){
		ESPI1_ReadDisable();
		return( FALSE );
	}
	ch = SPI1BUF;
	ESPI1_bRxCheckSum += ch;

	ESPI1_bStatus = ESPI_NORMAL;

	return( ch );
}

BOOL ESPI1_CheckSum( void )
{
	BYTE ch;

	if( !WaitTxBufReady() ){
		ESPI1_ReadDisable();
		return( FALSE );
	}
	SPI1BUF = 0;
	if( !WaitRxBufReady() ){
		ESPI1_ReadDisable();
		return( FALSE );
	}
	ch = SPI1BUF;
	if( ESPI1_bRxCheckSum != ch ){
		ESPI1_bStatus = ESPI_DATA_ERROR;
		ESPI1_ReadDisable();
		return( FALSE ); 
	}
	return( TRUE );
}


BOOL ESPI1_IDReceiveEnable( void )
{
	SPI1STATbits.SPIEN =0; // disable
	ESPI1_wTimeCount = 0;
	
	while(!CS){
		if( ESPI1_wTimeCount > ESPI1_wTimeout ){
			ESPI1_bStatus = ESPI_CS_HUNGUP;
			return( FALSE );	
		}
	}

	SPI1CONbits.SSEN =1;
	SPI1STATbits.SPIEN =1;
	return( TRUE );
}

void ESPI1_IDReceiveDisable( void )
{
	SPI1STATbits.SPIEN =0;
	SPI1CONbits.SSEN = 0;
}

BOOL ESPI1_SetID( BYTE id )
{
	unsigned char dumy;

	CS = 0;

	if( !WaitTxBufReady() ){
		ESPI1_WriteDisable();
		return( FALSE );
	}
	SPI1BUF = id;
	if( !WaitRxBufReady() ){
		ESPI1_WriteDisable();
		return( FALSE );
	}
	dumy = SPI1BUF;

	CS = 1;
	
	ESPI1_bStatus = ESPI_NORMAL;
	return( TRUE );
}

#ifdef SH002

void ESPI1_SetAdr( char ch )
{
	SADR = (SADR & 0xFC) | ch;
}
#else

void ESPI1_SetAdr( char ch )
{
}

#endif



void ESPI1_WriteEnable( void )
{
	DIR_RE = SPI_TX;	// all high Z
	SPI1CONbits.SMP =1;
	SPI1CONbits.MSTEN =1;
	
	SPI1STATbits.SPIEN =1;
	DIR_DE = SPI_TX;	// Tx ON
	
	wait_us(2);	// M†ˆÀ’è‘Ò‚¿
}

void ESPI1_WriteDisable( void )
{
	DIR_DE = SPI_RX;	// TX disable

	SPI1STATbits.SPIEN =0;
	DIR_RE = SPI_RX;	// RX enable

	SPI1CONbits.MSTEN =0;
	SPI1CONbits.SMP =0;

	wait_us(2); // M†ˆÀ’è‘Ò‚¿

}


BOOL ESPI1_Write(unsigned char* buf, int size )
{
	unsigned char dumy;
	
	while(size-->0){
		if( !WaitTxBufReady() ){
			ESPI1_WriteDisable();
			return( FALSE );
		}
		SPI1BUF = *buf;
		ESPI1_bTxCheckSum += *buf++;
		if( !WaitRxBufReady() ){
			ESPI1_WriteDisable();
			return( FALSE );
		}
		dumy = SPI1BUF;
	}
	
	ESPI1_bStatus = ESPI_NORMAL;
	return( TRUE );

} 		

BOOL ESPI1_putch( char ch  )
{
	unsigned char dumy;

	if( !WaitTxBufReady() ){
		ESPI1_WriteDisable();
		return( FALSE );
	}
	SPI1BUF = ch;
	ESPI1_bTxCheckSum += ch;
	if( !WaitRxBufReady() ){
		ESPI1_WriteDisable();
		return( FALSE );
	}
	dumy = SPI1BUF;

	ESPI1_bStatus = ESPI_NORMAL;
	return( TRUE );
} 		

BOOL ESPI1_AddCheckSum( void )
{
	BYTE dumy;

	if( !WaitTxBufReady() ){
		ESPI1_WriteDisable();
		return( FALSE );
	}
	SPI1BUF = ESPI1_bTxCheckSum;
	if( !WaitRxBufReady() ){
		ESPI1_WriteDisable();
		return( FALSE );
	}
	dumy = SPI1BUF;
	
	ESPI1_bStatus = ESPI_NORMAL;
	return( TRUE );
}

