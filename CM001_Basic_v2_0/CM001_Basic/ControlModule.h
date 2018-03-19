/*
	ControlModule.h 

						2008.11.20 M.Ustumi@EasLogic
*/

#ifndef CONTROL_MODULE_H
#define CONTROL_MODULE_H

#include"Hardware.h"
#include"ESPIx16.h"

#define CM_MASTER_ID		0xFF

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

#define CM_TEST_STATE			200

#define ACK		0x55
#define NAK		0xAA

typedef struct _CM_PARAM
{
	float dt;			// 制御ループの周期[sec]
	float fCtrlTimeout;	// 制御のタイムアウト[sec] 0で∞
	float fComTimeout;	// 通信のタイムアウト[sec] 0で∞
} CM_PARAM, *PCM_PARAM;


extern volatile BYTE CM_bID;
extern const BYTE CM_bVersion; 
extern BYTE CM_bStatus; 
extern BYTE CM_bCtrlStatus; 
extern volatile BOOL CM_ControlTrig;	// 	周期用トリガ
extern volatile WORD CM_wTimeCount;
extern volatile WORD CM_wCtrlTimeout;	//  制御タイムアウト

extern volatile BOOL CM_SyncCommandReady;
extern volatile BOOL CM_AsyncCommandReady;

#define BUF_SIZE		128
extern WORD wRxBufferA[ BUF_SIZE ];
extern WORD wRxBufferB[ BUF_SIZE ];
extern WORD wTxBufferA[ BUF_SIZE ];
extern WORD wTxBufferB[ BUF_SIZE ];

void CM_ChangeRxBuffer( void );
void CM_ChangeTxBuffer( void );

void CM_StatusBlink( WORD wHalfPeriod_ms, WORD wNum, BOOL AccessMode  );
void CM_SetID( BYTE bID );
BYTE CM_GetID( void );
void CM_SetTimer( void  );
BOOL CM_CommandProc( BYTE bCommand );
void CM_Open( float delta_t );
void CM_ReadBuf( BYTE* pbBuf, WORD wSize );
void CM_WriteBuf( BYTE* pbBuf, WORD wSize );
BOOL CM_Command( void );
BOOL CM_Reply( void );
void CM_Rearm( void );
void CM_EnableBufferedMode( int priority  );

#endif

