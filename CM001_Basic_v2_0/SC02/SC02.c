/*
	SC02 Program Ver.1.0

						2010.4.6 M.Ustumi@ArcDevice

*/

#include"stdio.h"
#include"math.h"

#include"timer.h"
#include"Hardware.h"
#include"ESPIx16.h"
#include"USB.h"

// Command

#define CM_OPEN 			0
#define CM_CLOSE 			1
#define CM_RESET 			2
#define CM_PING			3
#define CM_SET_ID 			4
#define CM_GET_ID	 		5
#define CM_GET_VER 		6
#define CM_BLINK 			7
#define CM_TRANSFER 		8
#define CM_SET_PARAM 		9
#define CM_GET_PARAM 		10


#define CM_NORMAL				100
#define CM_NAK				101
#define CM_CTRL_TIMEOUT		102

#define CM_COMMAND_ERROR		110
#define CM_COMMAND_UNDEFINED	110
#define CM_COMMAND_TX_TIMEOUT	111		
#define CM_COMMAND_RX_TIMEOUT	112		
#define CM_COMMAND_SUM_ERROR	113		

#define CM_REPLY_ERROR			120
#define CM_REPLY_NO			120
#define CM_REPLY_TX_TIMEOUT		121
#define CM_REPLY_RX_TIMEOUT		122
#define CM_REPLY_SUM_ERROR		123

#define CM_TRANSFER_ERROR		130
#define CM_TRANSFER_TX_TIMEOUT	131
#define CM_TRANSFER_RX_TIMEOUT	132
#define CM_TRANSFER_SUM_ERROR	133

#define SH_RESET_MODULES	20

#define ACK					0x55 // 85
#define NAK					0xAA // 170

typedef struct _TR_HEADER
{
	BYTE	bPort;
	BYTE	bID;
	WORD	wTxSize;
	WORD	wRxSize;
} TR_HEADER, *PTR_HEADER;


volatile BYTE CM_bID = 0;
BYTE Version = 0x20;
BYTE bStatus = CM_NORMAL;
volatile WORD CM_wTimeCount=0;			//  ƒJƒEƒ“ƒ^

void CM_SetID( BYTE bID )
{
	WORD w;
	w = bID;
	EEP_Write( &w, 1 );
}

BYTE CM_GetID( void )
{	
	WORD w;
	EEP_Read( &w, 1 );
	return( (BYTE)w );
}


WORD wBlinkCount=0;
WORD wBlinkHalfPeriod_ms;
BOOL BilnkAccessMode = FALSE;

void CM_StatusBlink( WORD wHalfPeriod_ms, WORD wNum, BOOL AccessMode  )
{
	if( AccessMode ){
		if(!BilnkAccessMode){
			wBlinkHalfPeriod_ms = wHalfPeriod_ms;
			wBlinkCount = wNum*2+1;
			BilnkAccessMode = TRUE;
			LED_ON();
		}
	}
	if( CM_wTimeCount > wBlinkHalfPeriod_ms ){
		CM_wTimeCount = 0;
		if( wBlinkCount-- ){
			LED = ~LED;
		}else{
			wBlinkHalfPeriod_ms = wHalfPeriod_ms;
			wBlinkCount = wNum*2+1;
			if(BilnkAccessMode) BilnkAccessMode = FALSE;
			LED_ON();
		}
	}
}


void _ISR _T1Interrupt( void )
{
	IFS0bits.T1IF = 0;

	USB_wTimeCount++;
	ESPI1_wTimeCount++;
	CM_wTimeCount++;

}

void SetTimer( float period )
{
	int count;
	WORD config = T1_ON & T1_GATE_OFF & T1_PS_1_8
					& T1_SYNC_EXT_OFF & T1_SOURCE_INT;

	count = (int)( period*1000.0f*1000.0f/(4.0f/(10.0f*8.0f)*8.0f));

	DisableIntT1;
	CloseTimer1();
	OpenTimer1( config, count-1 );
	ConfigIntTimer1( T1_INT_PRIOR_5 & T1_INT_ON );
}


BYTE TxBuffer[256];
BYTE* pTxBuffer;

int 	BufferCount=0;


BOOL InitBuffer( void )
{
	pTxBuffer = TxBuffer;
	BufferCount= 0;
}

BOOL WriteBuffer( BYTE* pBuffer, int size )
{

	if( BufferCount+size >= 256 ) return( FALSE );
	BufferCount+=size;
	
	while( size-- ){
		*pTxBuffer++ = *pBuffer++;
	}

	return( TRUE );	
}

TR_HEADER Header;
WORD wRxBuffer[128];
WORD wTxBuffer[128];
WORD wRxSize;

BOOL SyncTransceive( void )
{
	BYTE *ptr, bCommand;
	WORD wWordSize, wRet;

// Start Hand shake
	bCommand = *(BYTE*)wTxBuffer;

	ESPI1_WriteEnable();
	if( !ESPI1_SetCommand( Header.bID, bCommand ) )	return( FALSE );

	ESPI1_WriteDisable();
	// ACK or NAK
	if( !ESPI1_ReadEnable() ) return( FALSE );
	
	if( !ESPI1_Read( &wRet, 1))	return( FALSE );
	
	ESPI1_ReadDisable();
// End Hand shake

	if( wRet == ACK ){
	// Transmit
		ESPI1_WriteEnable();
		
		if(!ESPI1_Write( &Header.wTxSize,  1 )) return( FALSE );
		wait_us(2);
		wWordSize = Header.wTxSize+1;
		wWordSize = wWordSize>>1;
		if( !ESPI1_Write( wTxBuffer, wWordSize ) )	return( FALSE );
		ESPI1_WriteDisable();

	// Receive

		if( !ESPI1_ReadEnable() ) return( FALSE );
		
		if( !ESPI1_Read( &wRxSize, 1) ) return( FALSE );
		wWordSize = wRxSize + 1;
		wWordSize = wWordSize>>1;
		if( !ESPI1_Read( wRxBuffer, wWordSize ) ) return( FALSE );
		ESPI1_ReadDisable();
	}else{ // NAK
		wRxSize = 2;
		ptr = (BYTE*)wRxBuffer;
		ptr[0] = CM_NAK;
		ptr[1] = CM_NAK;
		blink(2);

	}
	return( TRUE );
}

BOOL AsyncTransceive( void )
{
	BYTE bCommand;
	WORD wWordSize;

	bCommand = *(BYTE*)wTxBuffer;

	ESPI1_WriteEnable();
	if( !ESPI1_SetCommand( Header.bID, bCommand ) )	return( FALSE );
	ESPI1_WriteDisable();

// Receive
	if( !ESPI1_ReadEnable() ) return( FALSE );	
	if(!ESPI1_Read( &wRxSize, 1)) return( FALSE );
	
	wWordSize = wRxSize+1;
	wWordSize = wWordSize>>1;
	if(!ESPI1_Read( wRxBuffer, wWordSize)) return( FALSE );
	ESPI1_ReadDisable();

// Transmit
	ESPI1_WriteEnable();
		
	if(!ESPI1_Write( &Header.wTxSize,  1 )) return( FALSE );
	wWordSize = Header.wTxSize+1;
	wWordSize = wWordSize>>1;
	if( !ESPI1_Write( wTxBuffer, wWordSize ) )	return( FALSE );
	ESPI1_WriteDisable();

	return( TRUE );
}

BOOL Transfer( void )
{
	BYTE bComNum, bDevStatus, bCommand;
	WORD wWordSize, wSize;
	
	InitBuffer();

	if(!USB_Read( &bComNum, 1 ) ){
		bStatus = CM_COMMAND_ERROR + USB_bStatus;
	}else{
		while( bComNum-- ){
			if(!USB_Read( (BYTE*)&Header, sizeof( TR_HEADER ) )){
				bStatus = CM_COMMAND_ERROR + USB_bStatus;
				return( FALSE );
			}
			if( !USB_Read( (BYTE*)wTxBuffer, Header.wTxSize ) ){
				bStatus = CM_COMMAND_ERROR + USB_bStatus;
				return( FALSE );
			}
			if( !WriteBuffer( (BYTE*)&Header.bPort, 1 ) ){
				bStatus = CM_REPLY_ERROR;
				return( FALSE );
			}
			if( !WriteBuffer( &Header.bID, 1 ) ){
				bStatus = CM_REPLY_ERROR;
				return( FALSE );
			}

			bCommand = *(BYTE*)wTxBuffer; 
			if( bCommand < 100 ){
				if(!SyncTransceive()){
					bDevStatus = CM_TRANSFER_ERROR + ESPI1_bStatus;
					wRxSize = 2;
					if( !WriteBuffer( (BYTE*)&wRxSize, 2 ) ) return( FALSE );
					if( !WriteBuffer( &bDevStatus, 1 ) ) return( FALSE );
					// dumy for checksum	
					if( !WriteBuffer( &bDevStatus, 1 ) )	return( FALSE );
				}else{
					if( !WriteBuffer( (BYTE*)&wRxSize, 2 ) ) return( FALSE );
					if( !WriteBuffer( (BYTE*)wRxBuffer, wRxSize ) ) return( FALSE );
				}
			}else{
				if(!AsyncTransceive()){
					bDevStatus = CM_TRANSFER_ERROR + ESPI1_bStatus;
					wRxSize = 2;
					if( !WriteBuffer( (BYTE*)&wRxSize, 2 ) ) return( FALSE );
					if( !WriteBuffer( &bDevStatus, 1 ) ) return( FALSE );
					// dumy for checksum	
					if( !WriteBuffer( &bDevStatus, 1 ) ) return( FALSE );
				}else{
					if( !WriteBuffer( (BYTE*)&wRxSize, 2 ) ) return( FALSE );
					if( !WriteBuffer( (BYTE*)wRxBuffer, wRxSize ) ) return( FALSE );
				}
			}

		}
	}	
	if(!WriteBuffer( &bStatus, 1 ) ){
		bStatus = CM_COMMAND_ERROR + USB_bStatus;
		return( FALSE );
	}else{
		bStatus = CM_NORMAL;
	}
	if(!USB_Write( TxBuffer, BufferCount ) ){
		bStatus = CM_COMMAND_ERROR + USB_bStatus;
		return( FALSE );
	}else{
		bStatus = CM_NORMAL;
	}

	return( TRUE );
}

BOOL CommandProc( void )
{
	BYTE bCommand, b;
	WORD wSize;

	if(!USB_Read( &bCommand, 1 )){
		bStatus = CM_COMMAND_ERROR + USB_bStatus;
		return( FALSE );
	}

	switch(bCommand){
	//case CM_PING: break;
	//case CM_SET_PARAM: break;
	//case CM_GET_PARAM: break;
	case CM_SET_ID:
		if(!USB_Read( &b, 1 )){
			bStatus = CM_COMMAND_ERROR + USB_bStatus;
			return( FALSE );
		}
		if(!USB_Write( &bStatus, 1 )){
			bStatus = CM_COMMAND_ERROR + USB_bStatus;
			return( FALSE );
		}
		CM_SetID( b );
		CM_bID = b;
		break;
	case CM_GET_ID:
		if(!USB_Write( (BYTE*)&CM_bID, 1 )){
			bStatus = CM_COMMAND_ERROR + USB_bStatus;
			return( FALSE );
		}
		if(!USB_Write( &bStatus, 1 )){
			bStatus = CM_COMMAND_ERROR + USB_bStatus;
			return( FALSE );
		}
		break;
	case CM_TRANSFER:
		if( !Transfer() ) return( FALSE );
		break;
	case CM_BLINK:
		if(!USB_Read( &b, 1 )){
			bStatus = CM_COMMAND_ERROR + USB_bStatus;
			return( FALSE );
		}
		if(!USB_Write( &bStatus, 1 )){
			bStatus = CM_COMMAND_ERROR + USB_bStatus;
			return( FALSE );
		}
		blink(b);			
		break;
	case CM_GET_VER:
		if( !USB_Write( &Version, 1 )){
			bStatus = CM_COMMAND_ERROR + USB_bStatus;
			return( FALSE );
		}
		if( !USB_Write( &bStatus, 1 )){
			bStatus = CM_COMMAND_ERROR + USB_bStatus;
			return( FALSE );
		}
		break;

	case SH_RESET_MODULES:
		if(!USB_Read( &b, 1 )){
			bStatus = CM_COMMAND_ERROR + USB_bStatus;
			return( FALSE );
		}
		blink(b);			
		if(!USB_Write( &bStatus, 1 )){
			bStatus = CM_COMMAND_ERROR + USB_bStatus;
			return( FALSE );
		}
		break;
	default:
		bStatus = CM_COMMAND_UNDEFINED;
		if( !USB_Write( &bStatus, 1 )){
			bStatus = CM_COMMAND_ERROR + USB_bStatus;
			return( FALSE );
		}
		return( FALSE );
	}

	bStatus = CM_NORMAL;
	return( TRUE );
}


int main(void)
{
	BYTE ch;

	TRIS_LED = 0;
	blink(3);

	USB_Open();
	SetTimer( 0.001f ); // set 1ms timer

	bStatus = CM_NORMAL;
	CM_bID = CM_GetID();

	ESPI1_Open();


	while(1){

		if( USB_RxReady() ){
			CommandProc();
			CM_StatusBlink( 50, 1, TRUE );
		}
		if( bStatus == CM_NORMAL )CM_StatusBlink( 1000, 1, FALSE ); 
		else CM_StatusBlink( 250, 1, FALSE );

	}

}


