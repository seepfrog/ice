/*************************************************************
	ControlModule.c Ver.2.0

						2010.4.8 M.Ustumi@ArcDevice

**************************************************************/

#include"stdio.h"
#include"math.h"
#include"timer.h"

#include"ControlModule.h"

volatile BYTE CM_bID = 0;
const BYTE CM_bVersion = 0x20; // version 2.0

BYTE CM_bStatus = CM_NORMAL;
BYTE CM_bCtrlStatus = CM_NORMAL;

volatile BOOL CM_ControlTrig = FALSE;	// 	周期用トリガ
volatile WORD CM_wTimeCount=0;			//  カウンタ
volatile WORD CM_wCtrlTimeout = 1000;	//  制御タイムアウト

volatile BOOL CM_SyncCommandReady = FALSE;
volatile BOOL CM_AsyncCommandReady = FALSE;

WORD wRxBufferA[ BUF_SIZE ];
WORD wRxBufferB[ BUF_SIZE ];
WORD wTxBufferA[ BUF_SIZE ];
WORD wTxBufferB[ BUF_SIZE ];
BYTE* pbRxBuffer;
BYTE* pbTxBuffer;
WORD wTxBufCountA=0;
WORD wTxBufCountB=0;
WORD* pwTxBufCount;
BOOL RxBufAEnable;
BOOL TxBufAEnable;
BYTE bChecksum;
BOOL BufferdMode;
short sIntPriority;

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

unsigned short Timer1Value = T1_ON & T1_GATE_OFF & T1_PS_1_8
				& T1_SYNC_EXT_OFF & T1_SOURCE_INT;
float dt;

void CM_SetTimer( void  )
{
	int period;
	
	DisableIntT1;
	CloseTimer1();
	
	// 周期の計算 10MHzクロック
	period = (int)(dt*1000.0f*1000.0f/(4.0f/(10.0f*8.0f)*8.0f));

	CM_ControlTrig = FALSE;
	OpenTimer1(Timer1Value, period-1 );
	ConfigIntTimer1( T1_INT_PRIOR_5 & T1_INT_ON );
}

WORD _EEDATA(2) EEP_wID=3;	//プログラム書き込み時の初期 ID

BYTE CM_GetID( void )
{
	WORD w;

	EEP_Read( 
		__builtin_tblpage(&EEP_wID),
		__builtin_tbloffset(&EEP_wID),
	 	&w, 1 );
	return( (BYTE)w );
}

void CM_SetID( BYTE bID )
{
	WORD w;

	w = bID;
	EEP_Write( 
		__builtin_tblpage(&EEP_wID),
		__builtin_tbloffset(&EEP_wID),
	 	&w, 1 );
}

BOOL CM_CommandProc( BYTE bCommand )
{
	BYTE b;
	CM_PARAM param;

	switch(bCommand){
	case CM_OPEN:
		b = CM_OPEN;
		CM_WriteBuf( &b, 1 );
		CM_Reply();
		break;
	case CM_CLOSE:
		b = CM_CLOSE;
		CM_WriteBuf( &b, 1 );
		CM_Reply();
		break;
	//case CM_RESET: break;
	case CM_PING:
		b = CM_PING;
		CM_WriteBuf( &b, 1 );

		CM_Reply();
		break;
	case CM_SET_ID:
		CM_ReadBuf( &b, 1);
		CM_SetID( b );
		CM_Reply();
		CM_bID = b;
		break;
	case CM_GET_ID:
		CM_bID = CM_GetID();
		CM_WriteBuf( (BYTE*)&CM_bID, 1 );
		CM_Reply();

		break;
	case CM_GET_VER:
		CM_WriteBuf( (BYTE*)&CM_bVersion, 1 );
		CM_Reply();
		break;
	case CM_BLINK:
		CM_ReadBuf( &b, 1 );
		CM_Reply();
		HW_Blink( b );
		break;
	case CM_SET_PARAM:
		CM_ReadBuf( (BYTE*)&param, sizeof(CM_PARAM) );
		CM_Reply();
				
		ConfigIntTimer1( T1_INT_OFF );
		dt = param.dt;
		
		CM_wCtrlTimeout = (WORD)(param.fCtrlTimeout/dt);
		ESPI1_wTimeout = (WORD)(param.fComTimeout/dt);
	
		CM_SetTimer();
		break;
		
	case CM_GET_PARAM:
		param.dt = dt;
		param.fCtrlTimeout = (float)CM_wCtrlTimeout*dt;
		param.fComTimeout  = (float)ESPI1_wTimeout*dt;

		CM_WriteBuf( (BYTE*)&param, sizeof(CM_PARAM) );
		CM_Reply();

		break;

	default:
		CM_bStatus = CM_COMMAND_UNDEFINED;
		CM_Reply();
		return( FALSE );
	}
	return( TRUE );

}

void CM_Open( float delta_t )
{
	HW_ZeroMemory( wRxBufferA, BUF_SIZE  );
	HW_ZeroMemory( wRxBufferB, BUF_SIZE  );
	HW_ZeroMemory( wTxBufferA, BUF_SIZE  );
	HW_ZeroMemory( wTxBufferB, BUF_SIZE  );

	RxBufAEnable = FALSE;
	pbRxBuffer = (BYTE*)wRxBufferA;
	TxBufAEnable = FALSE;
	pbTxBuffer = (BYTE*)wTxBufferA;
	wTxBufCountA = 0;
	wTxBufCountB = 0;
	pwTxBufCount = &wTxBufCountA;
	bChecksum = 0;

	BufferdMode = FALSE;

	ESPI1_Open();
	ESPI1_IDReceiveEnable();

	dt =  delta_t;
	CM_SetTimer(); // 1msecで割り込み
}

void CM_EnableBufferedMode( int priority  )
{

	BufferdMode = TRUE;

	switch( priority ){
	case 0: sIntPriority = SPI_INT_PRI_0; break;
	case 1: sIntPriority = SPI_INT_PRI_1; break;
	case 2: sIntPriority = SPI_INT_PRI_2; break;
	case 3: sIntPriority = SPI_INT_PRI_3; break;
	case 4: sIntPriority = SPI_INT_PRI_4; break;
	case 5: sIntPriority = SPI_INT_PRI_5; break;
	case 6: sIntPriority = SPI_INT_PRI_6; break;
	case 7: sIntPriority = SPI_INT_PRI_7; break;
	}
	ConfigIntSPI1( SPI_INT_EN & sIntPriority  );
}

void CM_ChangeRxBuffer( void )
{
	if( RxBufAEnable ){
		RxBufAEnable = FALSE;
		pbRxBuffer = (BYTE*)wRxBufferA;
	}else{
		RxBufAEnable = TRUE;
		pbRxBuffer = (BYTE*)wRxBufferB;
	}
}

void CM_ChangeTxBuffer( void )
{
	if( TxBufAEnable ){
		TxBufAEnable = FALSE;
		pbTxBuffer = (BYTE*)wTxBufferA;
		wTxBufCountA = 0;
		pwTxBufCount = &wTxBufCountA;
	}else{
		TxBufAEnable = TRUE;
		pbTxBuffer = (BYTE*)wTxBufferB;
		wTxBufCountB = 0;
		pwTxBufCount = &wTxBufCountB;
	}
}

void CM_ReadBuf( BYTE* pbBuf, WORD wSize )
{
	while( wSize-->0){
		*pbBuf++ = *pbRxBuffer++;
	}
}

void CM_WriteBuf( BYTE* pbBuf, WORD wSize )
{
	while( wSize-->0){
		*pbTxBuffer++ = *pbBuf;
		bChecksum += *pbBuf++;
		(*pwTxBufCount)++;
	}
}

BOOL CM_CheckSum( BYTE* pbBuffer, WORD wSize )
{
	BYTE bSum=0;

	wSize--;
	while( wSize-- ){
		bSum += *pbBuffer++;	
	}
	if( bSum != *pbBuffer )	return( FALSE );

	return( TRUE );
}

void CM_AddChecksum( void )
{
	*pbTxBuffer++ = bChecksum;
	(*pwTxBufCount)++;
	bChecksum = 0;

}

BOOL CM_SyncTransceive( void )
{
	WORD wSize, wWordSize, w;
	WORD *pwBuf;
	BYTE *ptr;

// 通信可能であることを伝える

	ESPI1_WriteEnable();
	w = ACK;
	if( !ESPI1_Write( &w, 1 ) ) return( FALSE );
	ESPI1_WriteDisable();

// 読み込みに切り替え
	if( !ESPI1_ReadEnable() ) return( FALSE );
	
// 受信データサイズを読み込む	
	if( !ESPI1_Read( &wSize, 1 ) ) return( FALSE );

// 指令値などのデータを一回全てbufに溜め込む	
// 受信バッファの切り替え	
	if( RxBufAEnable ) pwBuf = wRxBufferA;
	else pwBuf = wRxBufferB;

// Rx Data 
	wWordSize = wSize+1;
	wWordSize = wWordSize >> 1;
	if( !ESPI1_Read( pwBuf, wWordSize ) ) return( FALSE );
	ESPI1_ReadDisable();

// チェックサム
	if( !CM_CheckSum( (BYTE*)pwBuf, wSize ) ){
		CM_bStatus = CM_COMMAND_SUM_ERROR;
		
		wSize = 2; // status + checksum
		ptr = (BYTE*)&w;
		ptr[0] = CM_bStatus;
		ptr[1] = CM_bStatus;

		ESPI1_WriteEnable();
		if( !ESPI1_Write( &wSize, 1 ) ) return( FALSE );
		if( !ESPI1_Write( &w, 1 ) ) return( FALSE );
		ESPI1_WriteDisable();

		return( FALSE );
	}
	return( TRUE );
}

BOOL CM_AsyncTransceive( void )
{
	WORD wSize, wWordSize, w;
	WORD *pwBuf;
	BYTE *ptr;

	if( TxBufAEnable ){
		pwBuf = wTxBufferA;
		wSize = wTxBufCountA;
	}else{
		pwBuf = wTxBufferB;
		wSize = wTxBufCountB;
	}

	ESPI1_WriteEnable();
	if( !ESPI1_Write( &wSize, 1 ) ){
		CM_bStatus = CM_REPLY_ERROR + ESPI1_bStatus;
		return(FALSE);
	}

	wWordSize = wSize + 1;
	wWordSize = wWordSize >> 1;

	if( !ESPI1_Write( pwBuf, wWordSize ) ){
		CM_bStatus = CM_REPLY_ERROR + ESPI1_bStatus;
		return(FALSE);
	}
	ESPI1_WriteDisable();
	
// 読み込みに切り替え
	if(!ESPI1_ReadEnable() ) return( FALSE );

// 受信データサイズを読み込む	
	if( !ESPI1_Read( &wSize, 1 ) ) return( FALSE );

// 指令値などのデータを一回全てbufに溜め込む	
// 受信バッファの切り替え	
	if( RxBufAEnable ) pwBuf = wRxBufferA;
	else pwBuf = wRxBufferB;

	wWordSize = wSize + 1;
	wWordSize = wWordSize >> 1;
// Rx Data 
	if( !ESPI1_Read( pwBuf, wWordSize ) ) return( FALSE );
	ESPI1_ReadDisable();
	CM_Rearm();	// 送受信の準備	

// チェックサム
	if( !CM_CheckSum( (BYTE*)pwBuf, wSize ) ){
		CM_bStatus = CM_COMMAND_SUM_ERROR;
		return( FALSE );
	}	
	return( TRUE );
}

BOOL CM_Reply( void )
{
	WORD wSize, wWordSize;
	WORD *pwBuf;
	BYTE *ptr;

// 送信バッファのセット

	CM_WriteBuf( &CM_bStatus, 1 );
	CM_AddChecksum();
	CM_ChangeTxBuffer();
	
	if( TxBufAEnable ){
		pwBuf = wTxBufferA;
		wSize = wTxBufCountA;
	}else{
		pwBuf = wTxBufferB;
		wSize = wTxBufCountB;
	}
	
	ESPI1_WriteEnable();
	if( !ESPI1_Write( &wSize, 1 ) ){
		CM_bStatus = CM_REPLY_ERROR + ESPI1_bStatus;
		return(FALSE);
	}

	wWordSize = wSize + 1;
	wWordSize = wWordSize >> 1;

	if( !ESPI1_Write( pwBuf, wWordSize ) ){
		CM_bStatus = CM_REPLY_ERROR + ESPI1_bStatus;
		return(FALSE);
	}
	ESPI1_WriteDisable();

	return( TRUE );	
}


void CM_Rearm( void )
{
	if( SPI1CONbits.SSEN == 0 ){
		ESPI1_IDReceiveEnable();
		if( BufferdMode )	ConfigIntSPI1( SPI_INT_EN & sIntPriority  );
	}
}


void _ISRFAST _SPI1Interrupt( void )
{
	WORD w;
	BYTE *pb;
	BYTE* pBuf;

	//IDのチェック
	ESPI1_Read( &w, 1 );
	IFS0bits.SPI1IF = 0;
	
	pb = (BYTE*)&w;
	
	if( (pb[0] != CM_bID)&&(pb[0] != CM_MASTER_ID) ) return;
	
	ESPI1_IDReceiveDisable();
	ConfigIntSPI1( SPI_INT_DIS  );
	
	if( pb[1] < 100 ){
		if( !CM_SyncTransceive() ){
			CM_bStatus = CM_COMMAND_ERROR + ESPI1_bStatus;
			CM_Rearm();
			return;
		}
		CM_SyncCommandReady = TRUE;
		CM_AsyncCommandReady = FALSE;

	}else{
		if( !CM_AsyncTransceive() ){
			CM_bStatus = CM_COMMAND_ERROR + ESPI1_bStatus;
			CM_Rearm();
			return;
		}
		CM_SyncCommandReady = FALSE;
		CM_AsyncCommandReady = TRUE;
	}	
}


BOOL CM_Command( void )
{
	WORD w;
	BYTE *pb;

	if( !ESPI1_DataReady() ) return( FALSE );

//IDのチェック
	ESPI1_Read( &w, 1 );
	pb = (BYTE*)&w;
	if( (pb[0] != CM_bID)&&(pb[1] != CM_MASTER_ID) ) return( FALSE );
		
	ESPI1_IDReceiveDisable();
	
	if( !CM_SyncTransceive() ){
		CM_bStatus = CM_COMMAND_ERROR + ESPI1_bStatus;
		CM_Rearm();
		return( FALSE );
	}
	CM_ChangeRxBuffer();

	CM_bStatus = CM_NORMAL;
	return( TRUE );
}



