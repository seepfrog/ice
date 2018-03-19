/************************************************************************************

	ControlModule.h  Ver 2.0
			
										2008.4.3 M.Utsumi@ArcDevice

*************************************************************************************/

#ifndef CONTROL_MODULE_H
#define CONTROL_MODULE_H

#include<stdio.h>
#include<conio.h>


#define  M_PI   3.141593f
#define  M_hPI   1.570796f
#define  M_2PI   6.283185f
 
#define DEGtoRAD 0.01745329252f 
#define RADtoDEG 57.29577951f

#define RADSECtoRPM 9.549296586 
#define RPMtoRADSEC 0.1047197551f
#define kgfcmToNm		0.09807f	
#define NmTokgfcm		10.1968f

#define CM_MASTER_ID		0xFF

// Command

#define CM_OPEN 			0
#define CM_CLOSE 			1
#define CM_RESET 			2
#define CM_PING				3
#define CM_SET_ID 			4
#define CM_GET_ID	 		5
#define CM_GET_VER 			6
#define CM_BLINK 			7
#define CM_TRANSFER 		8
#define CM_SET_PARAM 		9
#define CM_GET_PARAM 		10

// Status

#define CM_NORMAL				100
#define CM_NAK					101
#define CM_CTRL_TIMEOUT			102

#define CM_COMMAND_ERROR		110
#define CM_COMMAND_UNDEFINED	110
#define CM_COMMAND_TX_TIMEOUT	111		
#define CM_COMMAND_RX_TIMEOUT	112		
#define CM_COMMAND_SUM_ERROR	113		

#define CM_REPLY_ERROR			120
#define CM_REPLY_NO				120
#define CM_REPLY_TX_TIMEOUT		121
#define CM_REPLY_RX_TIMEOUT		122
#define CM_REPLY_SUM_ERROR		123

#define CM_TRANSFER_ERROR		130
#define CM_TRANSFER_TX_TIMEOUT	131
#define CM_TRANSFER_RX_TIMEOUT	132
#define CM_TRANSFER_SUM_ERROR	133


#define ACK					0x55 // 85
#define NAK					0xAA // 170

typedef struct _CM_TR_HEADER
{
	BYTE	bPort;
	BYTE	bID;
	WORD	wTxSize;
	WORD	wRxSize;
} CM_TR_HEADER, *PCM_TR_HEADER;


typedef struct _CM_PARAM
{
	float dt;			// 制御ループの周期[sec]
	float fCtrlTimeout;	// 制御のタイムアウト[sec] 0で∞
	float fComTimeout;	// 通信のタイムアウト[sec] 0で∞
} CM_PARAM, *PCM_PARAM;

#define CM_DEFAULT_BUF_SIZE 64

class ControlModule
{
public:
	ControlModule()
	{
		pParentModule = NULL;
		bStatus = CM_NORMAL;
		TxBuffer = new BYTE[CM_DEFAULT_BUF_SIZE];
		RxBuffer = new BYTE[CM_DEFAULT_BUF_SIZE];
		CheckSumEnable = FALSE;
		bVersion = 0;
		wReqRxSize = 0;
	}
	~ControlModule()
	{
		delete TxBuffer;
		delete RxBuffer;
	}

	virtual BOOL Open( BYTE id, BOOL overlapped=FALSE )
	{
		pParentModule = NULL;
		return( TRUE );
	}
	virtual BOOL Open( HANDLE hParentModule, BYTE port, BYTE id )
	{
		BYTE b;

		pParentModule = (ControlModule*)hParentModule;
		bPort = port;
		bID   = id;
		if( !Transceive( CM_OPEN, NULL, 0, &b, 1 ) ) return( FALSE ); 
		if( b != CM_OPEN  ) return( FALSE );
	
		return( TRUE );
	}
	virtual BOOL Close( void ){
		BYTE b;
		if( !Transceive( CM_CLOSE, NULL, 0, &b, 1 ) ) return( FALSE ); 
		if( b != CM_CLOSE  ) return( FALSE );
		return( TRUE );
	}

	void AddCheckSum( void* pBuf, int size )
	{
		int i;
		BYTE sum=0;
		BYTE *ptr;

		ptr = (BYTE*)pBuf;

		for(i=0;i<size;i++){
			sum +=*ptr++;
		}
		*ptr = sum;
	}

	BOOL CheckSum(  void* pBuf, int size )
	{
		int i;
		BYTE sum=0;
		BYTE *ptr;

		ptr = (BYTE*)pBuf;

		for(i=0;i<size;i++){
			//printf("%d ",*ptr);
			sum +=*ptr++;
		}
		if( *ptr != sum ){
		//	printf("sum %d %d\r\n",sum, *ptr);
			return( FALSE );
		}
		return( TRUE );
	}

	virtual BOOL Transceive( BYTE bCommand, void* pTxBuffer, WORD wTxSize, 
											void* pRxBuffer, WORD wRxSize, BOOL RightNow=TRUE )	

	{
		WORD tx_size, rx_size;

		if( pParentModule ){
			pReserveBuffer = (BYTE*)pRxBuffer;
			tx_size = 1 + wTxSize; // add command 
			rx_size = wRxSize+1; // add status;

			TxBuffer[0] = bCommand;
			memcpy( &TxBuffer[1], pTxBuffer, wTxSize );
			if( CheckSumEnable ){
				AddCheckSum( TxBuffer, tx_size );
				tx_size++; // +Checksum
				rx_size++; // +Checksum
			}
			wReqRxSize = rx_size;

			if( !pParentModule->Transfer( this, TxBuffer, tx_size, rx_size, RightNow ) ){
				sprintf_s( Error, "ParentModule transfer error!\r\n");
				return( FALSE );
			}
			if( bStatus != CM_NORMAL ) return( FALSE );

		}
		return( TRUE );
	}

	virtual BOOL Transfer( HANDLE hChildModule, void* pTxBuffer, WORD wTxSize, WORD wRxSize, BOOL RightNow )
	{
		return(TRUE);
	}

	virtual BOOL Receive( BYTE *pRxBuffer, WORD wSize )
	{
		if( wSize != wReqRxSize ){
			sprintf_s( Error, "Rx Data size miss match %d %d\r\n", wReqRxSize,wSize );
		
			if( CheckSumEnable ){
				wSize--; // sub checksum;

				if( wSize == 0 ){
					bStatus = pRxBuffer[0];
					return( FALSE );
				}
			}
			bStatus = pRxBuffer[wSize-1];
			return( FALSE );
		}
		
		if( CheckSumEnable ){
			wSize--; // -CheckSum
		    if(!CheckSum( pRxBuffer, wSize )){
				bStatus = CM_REPLY_SUM_ERROR;
				return( FALSE );
			}
		}
		wSize--; // sub status;
				
		if( wSize > 0 ){
			memcpy( pReserveBuffer, pRxBuffer, wSize );
		}
		bStatus = pRxBuffer[wSize];
		
		if( bStatus!=CM_NORMAL ) return( FALSE );

		return( TRUE );
	}

	virtual BOOL Reset( void )
	{
		if( !Transceive( CM_RESET, NULL, 0, NULL, 0 ) ) return( FALSE ); 
		Sleep(1000);	//デバイスが復帰するまで時間調整
		return( TRUE );
	}

	virtual BOOL Ping( void )
	{
		BYTE ch;
		if( !Transceive( CM_PING, NULL, 0, &ch, 1 ) ) return( FALSE ); 
		if( ch != CM_PING  ) return( FALSE );
		return( TRUE );
	}

	virtual BOOL SetID( BYTE id )
	{
		if( !Transceive( CM_SET_ID, &id, 1, NULL, 0 )) return( FALSE ); 
		bID = id;
		return( TRUE );
	}

	virtual BOOL GetID( void )
	{
		if( !Transceive( CM_GET_ID, NULL, 0, &bID, 1 ) ) return( FALSE); 
		return( TRUE );
	}

	virtual BOOL GetVersion( void )
	{
		if(!Transceive( CM_GET_VER, NULL, 0, &bVersion, 1 ) ) return( FALSE );
		return( TRUE ); 
	}

	virtual BOOL Blink( BYTE bNum )		
	{
		if(!Transceive( CM_BLINK, &bNum, 1, NULL, 0 )) return(FALSE);
		return( TRUE );
	}

	virtual BOOL SetParam( PCM_PARAM pParam  )
	{
		if(!Transceive( CM_SET_PARAM, pParam, sizeof( CM_PARAM ), NULL, 0 ) ) return(FALSE);
		return( TRUE );
	}

	virtual BOOL GetParam( PCM_PARAM pParam  )
	{
		if( !Transceive( CM_GET_PARAM, NULL, 0, &pParam, sizeof( CM_PARAM ) ) )  return(FALSE);
		return( TRUE );
	}

	BYTE bPort;
	BYTE bID;
	BYTE bStatus;
	BYTE bVersion;
	char Error[256];
		
//private:

	ControlModule*	pParentModule;
	BYTE* TxBuffer;
	BYTE* RxBuffer;
	WORD wReqRxSize;
	BYTE* pReserveBuffer;
	BOOL CheckSumEnable;

};

#endif
