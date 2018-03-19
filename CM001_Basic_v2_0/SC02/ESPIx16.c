/*
	ESPIx.c 16bit mode for SC001

			2009.10.22 M.Utsumi@ArcDevice

*/

#include"ESPIx16.h"

WORD ESPI1_wTimeout; 
volatile BYTE ESPI1_bStatus=0; 
volatile WORD ESPI1_bRxCheckSum=0;
volatile WORD ESPI1_bTxCheckSum=0;
volatile WORD ESPI1_wTimeCount=0;


void ESPI1_Open( void )
{

	ADPCFGbits.PCFG2 = 1; // PB2 digital port
	TRIS_DIR_DE = 0;
	TRIS_DIR_RE = 0;
	DIR_DE = SPI_RX;
	DIR_RE = SPI_RX;
	TRIS_CS = 1;
	CS = 1;

	SPI1STATbits.SPIEN =0; 	// disable
	SPI1STATbits.SPISIDL = 0;	// continue
	SPI1STATbits.SPIROV = 0;	// clear over fllow
	
	SPI1CONbits.FRMEN = 0;		// frame disable	 	
	SPI1CONbits.SPIFSD = 0;	// frame sync output
	SPI1CONbits.DISSDO = 0;	// SDO pin enable
	SPI1CONbits.MODE16 = 1;	// 16bit mode
	SPI1CONbits.SMP = 0;		// master:sample phase center 
	SPI1CONbits.CKE = 0;		// data change timing idle -> active	
	SPI1CONbits.SSEN = 0;		// SS pin disable (initial state)
	SPI1CONbits.CKP = 0;		// clock pole active high
	SPI1CONbits.MSTEN = 0;		// slave mode (initial state)
	SPI1CONbits.SPRE = 0x07;	// secondary prescale 2:1
	SPI1CONbits.PPRE = 0x03;	// primary prescale 1:1
	

	ESPI1_bStatus = ESPI_NORMAL;
	ESPI1_wTimeout = 5000; // ms

}

void ESPI1_Close( void )
{
	CloseSPI1();
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


BOOL ESPI1_Read( WORD* pwBuffer, WORD wSize )
{
	while( wSize--){
		ESPI1_wTimeCount = 0;
		while( !SPI1STATbits.SPIRBF){
			if( ESPI1_wTimeCount > ESPI1_wTimeout ){
				ESPI1_bStatus = ESPI_RX_TIMEOUT;
				return( FALSE );	
			}
		}
		*pwBuffer++ = SPI1BUF;
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

BOOL ESPI1_SetCommand( BYTE bId, BYTE bCommand )
{
	WORD w;
	BYTE* ptr;

	ptr = (BYTE*)&w;
	ptr[0] = bId;
	ptr[1] = bCommand;
		
	TRIS_CS = 0;
	CS = 0;

	ESPI1_wTimeCount = 0;
	while( SPI1STATbits.SPITBF ){
		if( ESPI1_wTimeCount > ESPI1_wTimeout ){
			ESPI1_bStatus = ESPI_TX_TIMEOUT;
			return( FALSE );	
		}
	}
	SPI1BUF = w;

	ESPI1_wTimeCount = 0;
	while( !SPI1STATbits.SPIRBF ){
		if( ESPI1_wTimeCount > ESPI1_wTimeout ){
			ESPI1_bStatus = ESPI_RX_TIMEOUT;
			return( FALSE );	
		}
	}
	w = SPI1BUF;

	CS = 1;
	TRIS_CS = 1;
	
	ESPI1_bStatus = ESPI_NORMAL;
	return( TRUE );
}



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


BOOL ESPI1_Write( WORD* pwBuffer, WORD wSize )
{
	WORD dumy;
	
	while(wSize--){
		ESPI1_wTimeCount = 0;
		while( SPI1STATbits.SPITBF ){
			if( ESPI1_wTimeCount > ESPI1_wTimeout ){
				ESPI1_bStatus = ESPI_TX_TIMEOUT;
				return( FALSE );	
			}
		}
		SPI1BUF = *pwBuffer++;
		ESPI1_wTimeCount = 0;
		while( !SPI1STATbits.SPIRBF ){
			if( ESPI1_wTimeCount > ESPI1_wTimeout ){
				ESPI1_bStatus = ESPI_RX_TIMEOUT;
				return( FALSE );	
			}
		}
		dumy = SPI1BUF;
	}
	
	ESPI1_bStatus = ESPI_NORMAL;
	return( TRUE );

} 		
