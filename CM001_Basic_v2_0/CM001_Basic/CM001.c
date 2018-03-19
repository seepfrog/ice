/****************************************************************

	CM001 Basic Program Ver.2.0

						2010.4.23	M.Ustumi@ArcDevice

******************************************************************/

#include"stdio.h"
#include"math.h"
#include"timer.h"

#include"ControlModule.h"
//#include"MotorController.h"
#include"Ad.h"
#include"Encoder.h"
#include"MotorPWM.h"	// PWM type
#include"MotorDA.h"

#define CM001_GET_CONFIG	50
#define CM001_EXCHANGE		150

typedef struct _CM001_INPUT_ENC
{
	long lEncoder;
	WORD wSensor[5];
} CM001_INPUT_ENC;

typedef struct _CM001_INPUT_SENSOR
{
	WORD wSensor[8];
} CM001_INPUT_SENSOR;

typedef struct _CM001_OUTPUT_PWM
{
	short sPwm1;
	short sPwm2;

} CM001_OUTPUT_PWM;

typedef struct _CM001_OUTPUT_DA
{
	short sDA;
} CM001_OUTPUT_DA;

typedef union _CM001_PIO
{
	struct{
		WORD wData;
		WORD wDir;
	} Word;
	struct{
		unsigned P0: 1;
		unsigned P1: 1;
		unsigned P2: 1;
		unsigned P3: 1;
		unsigned P4: 1;
		unsigned P5: 1;
		unsigned P6: 1;
		unsigned P7: 1;

		unsigned P8:  1;
		unsigned P9:  1;
		unsigned P10: 1;
		unsigned P11: 1;
		unsigned P12: 1;
		unsigned P13: 1;
		unsigned P14: 1;
		unsigned P15: 1;

		unsigned DIR0:  1;
		unsigned DIR1:  1;
		unsigned DIR2:  1;
		unsigned DIR3:  1;
		unsigned DIR4:  1;
		unsigned DIR5:  1;
		unsigned DIR6:  1;
		unsigned DIR7:  1;

		unsigned DIR8:  1;
		unsigned DIR9:  1;
		unsigned DIR10: 1;
		unsigned DIR11: 1;
		unsigned DIR12: 1;
		unsigned DIR13: 1;
		unsigned DIR14: 1;
		unsigned DIR15: 1;
	} Bit;

} CM001_PIO;


short sInputDataSize_PA;
short sOutputDataSize_PA;
short sInputDataSize_PB;
short sOutputDataSize_PB;
BYTE* pInputData_PA;
BYTE* pOutputData_PA;
BYTE* pOutputData_PB;
BYTE* pInputData_PB;

CM001_INPUT_ENC EncBoard;
CM001_INPUT_SENSOR SensorBoard;
CM001_OUTPUT_PWM PwmBoard;
CM001_OUTPUT_DA DaBoard;
CM001_PIO PioBoardA;
CM001_PIO PioBoardB;

/******************************************************************

 制御周期タイマ
 制御周期はタイマ1で作り，一定周期の割り込みでCM_ControlTrigをTRUEにする．
 メイン制御ルーチンはCM_ControlTrigがTRUEになるまで待ち一定周期の処理を行う．
CM_ControlTrigはメイン制御ルーチンでFALSEにする．

******************************************************************/


void _ISR _T1Interrupt( void )
{
	IFS0bits.T1IF = 0;
	CM_ControlTrig = TRUE;
	ESPI1_wTimeCount++;
	CM_wTimeCount++;

	ENCODER_Count();	// エンコーダ値を一定時間で保存
	
}



void ProcEncoder( void )
{

	EncBoard.lEncoder = ENCODER_Counter.count_32; // 32bit絶対カウント値
	
	EncBoard.wSensor[0] = AD_GetData( 0 );
	EncBoard.wSensor[1] = AD_GetData( 1 );
	EncBoard.wSensor[2] = AD_GetData( 5 );
	EncBoard.wSensor[3] = AD_GetData( 6 );
	EncBoard.wSensor[4] = AD_GetData( 7 );
}

void ProcSensor( void )
{
	short ch;

	for(ch=0;ch<8;ch++ ) SensorBoard.wSensor[ch] = AD_GetData( ch );

}

void ProcPWM( void )
{
	MOTOR_Drive( 0, PwmBoard.sPwm1 );
	MOTOR_Drive( 1, PwmBoard.sPwm2 );
}

void ProcDA( void )
{
	MOTOR_DA_Drive( DaBoard.sDA );
}

void ProcPIOA( void )
{
	// 出力データの格納
	PA0_OUT = PioBoardA.Bit.P0;
	PA1_OUT = PioBoardA.Bit.P1;
	PA2_OUT = PioBoardA.Bit.P2;
	PA3_OUT = PioBoardA.Bit.P3;
	PA4_OUT = PioBoardA.Bit.P4;
	PA5_OUT = PioBoardA.Bit.P5;
	PA6_OUT = PioBoardA.Bit.P6;
	PA7_OUT = PioBoardA.Bit.P7;

	// 入出力方向決定
	PA0_DIR = PioBoardA.Bit.DIR0;
	PA1_DIR = PioBoardA.Bit.DIR1;
	PA2_DIR = PioBoardA.Bit.DIR2;
	PA3_DIR = PioBoardA.Bit.DIR3;
	PA4_DIR = PioBoardA.Bit.DIR4;
	PA5_DIR = PioBoardA.Bit.DIR5;
	PA6_DIR = PioBoardA.Bit.DIR6;
	PA7_DIR = PioBoardA.Bit.DIR7;

	// 入力データ格納
	PioBoardA.Bit.P0 = PA0_IN;
	PioBoardA.Bit.P1 = PA1_IN;
	PioBoardA.Bit.P2 = PA2_IN;
	PioBoardA.Bit.P3 = PA3_IN;
	PioBoardA.Bit.P4 = PA4_IN;
	PioBoardA.Bit.P5 = PA5_IN;
	PioBoardA.Bit.P6 = PA6_IN;
	PioBoardA.Bit.P7 = PA7_IN;
}

void ProcPIOB( void )
{
	// 出力データの格納
	PB0_OUT = PioBoardB.Bit.P0;
	PB1_OUT = PioBoardB.Bit.P1;
	PB2_OUT = PioBoardB.Bit.P2;
	PB3_OUT = PioBoardB.Bit.P3;
	PB4_OUT = PioBoardB.Bit.P4;
	PB5_OUT = PioBoardB.Bit.P5;
	PB6_OUT = PioBoardB.Bit.P6;
	PB7_OUT = PioBoardB.Bit.P7;
	PB8_OUT = PioBoardB.Bit.P8;
	PB9_OUT = PioBoardB.Bit.P9;

	// 入出力方向決定
	PB0_DIR = PioBoardB.Bit.DIR0;
	PB1_DIR = PioBoardB.Bit.DIR1;
	PB2_DIR = PioBoardB.Bit.DIR2;
	PB3_DIR = PioBoardB.Bit.DIR3;
	PB4_DIR = PioBoardB.Bit.DIR4;
	PB5_DIR = PioBoardB.Bit.DIR5;
	PB6_DIR = PioBoardB.Bit.DIR6;
	PB7_DIR = PioBoardB.Bit.DIR7;
	PB8_DIR = PioBoardB.Bit.DIR8;
	PB9_DIR = PioBoardB.Bit.DIR9;

	// 入力データ格納
	PioBoardB.Bit.P0 = PB0_IN;
	PioBoardB.Bit.P1 = PB1_IN;
	PioBoardB.Bit.P2 = PB2_IN;
	PioBoardB.Bit.P3 = PB3_IN;
	PioBoardB.Bit.P4 = PB4_IN;
	PioBoardB.Bit.P5 = PB5_IN;
	PioBoardB.Bit.P6 = PB6_IN;
	PioBoardB.Bit.P7 = PB7_IN;
	PioBoardB.Bit.P8 = PB8_IN;
	PioBoardB.Bit.P9 = PB9_IN;

}

void Control(void)
{
	if( sOutputDataSize_PA > 0 )
		CM_ReadBuf( pOutputData_PA, sOutputDataSize_PA );
	if( sOutputDataSize_PB > 0 )
		CM_ReadBuf( pOutputData_PB, sOutputDataSize_PB );

	switch(bBoardID_PA){
		case ENC_BOARD: 	 ProcEncoder();break;
		case SENSOR_BOARD: ProcSensor(); break;
		case PIO_BOARD: 	 ProcPIOA();   break;
		default: ; break;
		}
	switch( bBoardID_PB ){
		case PWM_BOARD: ProcPWM();  break;
		case DA_BOARD:  ProcDA();   break;
		case PIO_BOARD: ProcPIOB(); break;
		default: ; break;
	}
	
	if( sInputDataSize_PA  > 0 )
		CM_WriteBuf( pInputData_PA, sInputDataSize_PA );
	if( sInputDataSize_PB  > 0 )
		CM_WriteBuf( pInputData_PB, sInputDataSize_PB );

// 送信データセット
	CM_WriteBuf( &CM_bStatus, 1 );
	CM_AddChecksum();
	CM_ChangeTxBuffer();
}

void Open( void )
{

	sInputDataSize_PA = 0;
	pInputData_PA = NULL;
	sOutputDataSize_PA = 0;
	pOutputData_PA = NULL;
	sInputDataSize_PB = 0;
	pInputData_PB = NULL;
	sOutputDataSize_PB = 0;
	pOutputData_PB = NULL;

	bBoardID_PA = (BYTE)( 0x03& (PORTC >> 13));
	bBoardID_PB = (BYTE)( 0x03& (PORTF >> 4));

	switch( bBoardID_PA){
	case ENC_BOARD:
		AD_Open();
		ENCODER_Open();

		sInputDataSize_PA = sizeof( CM001_INPUT_ENC );
		pInputData_PA = (BYTE*)&EncBoard;
		break;
	case SENSOR_BOARD:
		AD_Open();

		sInputDataSize_PA = sizeof( CM001_INPUT_SENSOR );
		pInputData_PA = (BYTE*)&SensorBoard;
		break;
	case PIO_BOARD:
		sInputDataSize_PA = sizeof( WORD );
		pInputData_PA = (BYTE*)&PioBoardA;
		sOutputDataSize_PA = sizeof( CM001_PIO );
		pOutputData_PA = (BYTE*)&PioBoardA;
		ADPCFG = 0xffff; //
		break;
	default:
		bBoardID_PA = PIO_BOARD;
		sInputDataSize_PA = sizeof( WORD );
		pInputData_PA = (BYTE*)&PioBoardA;
		sOutputDataSize_PA = sizeof( CM001_PIO );
		pOutputData_PA = (BYTE*)&PioBoardA;
		break;
	}
	switch( bBoardID_PB ){
	case PWM_BOARD:
		MOTOR_Open();

		sOutputDataSize_PB = sizeof( CM001_OUTPUT_PWM );
		pOutputData_PB = (BYTE*)&PwmBoard;
		break; // PWM board
	case DA_BOARD:
		MOTOR_DA_Open();

		sOutputDataSize_PB = sizeof( CM001_OUTPUT_DA );
		pOutputData_PB = (BYTE*)&DaBoard;
		break; // DA board
	case PIO_BOARD:
		sInputDataSize_PB = sizeof( WORD );
		pInputData_PB = (BYTE*)&PioBoardB;
		sOutputDataSize_PB = sizeof( CM001_PIO );
		pOutputData_PB = (BYTE*)&PioBoardB;
		break; // PIO board
	default:
		bBoardID_PB = PIO_BOARD;	
		sInputDataSize_PB = sizeof( WORD );
		pInputData_PB = (BYTE*)&PioBoardB;
		sOutputDataSize_PB = sizeof( CM001_PIO );
		pOutputData_PB = (BYTE*)&PioBoardB;
		break; // PIO board
	}

	
//	コントロールモジュールの設定　制御周期タイマ開始
	CM_Open( 0.001f );	// ID 周期1ms
	CM_EnableBufferedMode( 4  );

	Control();
}


void SyncCommandProc(void)
{
	BYTE bCommand;
	WORD wSize;

	// バッファに読み込まれたコマンドの読み出しと送信
	CM_ChangeRxBuffer();
	CM_ReadBuf( &bCommand, 1 );
	switch(bCommand){
	case CM001_GET_CONFIG:
		CM_WriteBuf( &bBoardID_PA, 1 );
		CM_WriteBuf( &bBoardID_PB, 1 );
		wSize = sInputDataSize_PA + sInputDataSize_PB;
		CM_WriteBuf( (BYTE*)&wSize, 2 );
		wSize = sOutputDataSize_PA + sOutputDataSize_PB;
		CM_WriteBuf( (BYTE*)&wSize, 2 );
		
		CM_Reply();
		break;
	default:
		// モータ制御コマンド，基本コマンドの処理
		CM_CommandProc( bCommand );
		break;
	}

	CM_Rearm();	// 送受信の準備
}

// 非同期制御コマンド処理
void AsyncCommandProc(void)
{
	BYTE b, bCommand;

	// バッファに読み込まれたコマンドの読み出しと送信

	CM_ChangeRxBuffer();
	CM_ReadBuf( &bCommand, 1 );

	switch( bCommand ){
	case CM001_EXCHANGE:
		Control();
		break;
	default:
		break;
	}
}

int main(void)
{
	TRIS_LED = 0; // output
		
	CM_bID = CM_GetID();	// ID

	Open();
	HW_Blink(2);
	
	while(1){
		if( CM_SyncCommandReady ){ // 同期型処理　命令受信→入出力→データ送信
			SyncCommandProc();
			Control(); // データのプリセット
			CM_SyncCommandReady = FALSE;
			CM_StatusBlink(50, 1, TRUE);
		}else if( CM_AsyncCommandReady ){ // 非同期型処理　命令受信→データ送信　連続入出力
			AsyncCommandProc();
			CM_AsyncCommandReady = FALSE;
			CM_StatusBlink(50, 1, TRUE);
		}

		if( CM_ControlTrig ){
			if( bBoardID_PA == ENC_BOARD ) ENCODER_GetRelativeCount();
			CM_ControlTrig = FALSE;
		}

		switch( CM_bStatus){
		case CM_NORMAL: CM_StatusBlink(1000, 1, FALSE); break;
		case CM_COMMAND_UNDEFINED: CM_StatusBlink(250, 1, FALSE); break;
		case CM_COMMAND_RX_TIMEOUT: CM_StatusBlink(250, 2, FALSE ); break;	
		case CM_COMMAND_SUM_ERROR: CM_StatusBlink(250, 3, FALSE );  break;
		case CM_REPLY_TX_TIMEOUT: CM_StatusBlink(250, 4, FALSE ); break;	
		case CM_REPLY_ERROR+ESPI_SCK_HUNGUP:
		case CM_COMMAND_ERROR+ESPI_SCK_HUNGUP: CM_StatusBlink( 250, 5, FALSE); break;
		case CM_REPLY_ERROR+ESPI_CS_HUNGUP:
		case CM_COMMAND_ERROR+ESPI_CS_HUNGUP: CM_StatusBlink( 250, 6, FALSE); break;
		//case CM_TEST_STATE: CM_StatusBlink( 250, 3, FALSE); break;
		}	

	}
}

