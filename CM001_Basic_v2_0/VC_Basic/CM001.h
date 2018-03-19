/*****************************************************************************************************

	CM001.h

	CM001 basic mode Ver.1.0
	 
	
													2009.2.23 M.Utsumi@EasLogic 

******************************************************************************************************/

#ifndef CM001_H
#define CM001_H

#include<windows.h>
#include"ControlModule.h"
#include"MotorController.h"

// コマンド
#define CM001_GET_CONFIG	50
#define CM001_EXCHANGE		150

// 入力ボードID
#define CM001_ENC_BOARD 	0
#define CM001_SENSOR_BOARD	1

// 出力ボードID
#define CM001_PWM_BOARD 	0
#define CM001_DA_BOARD		1

#define CM001_PIO_BOARD		2

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



class CM001 : public MotorController
{
public:
	CM001();
	~CM001();

	BOOL Open( HANDLE hCm, int port, BYTE id );
	BOOL GetConfig( void );
	BOOL Exchange( void* pOutputData, void* pInputData, BOOL RightNow=TRUE );
	BOOL Exchange( BOOL RightNow=TRUE );
	BOOL Receive( BYTE *pRxBuffer, WORD wSize );	
	
	BYTE bBoardID_PA;
	BYTE bBoardID_PB;
	short sInputDataSize;
	short sOutputDataSize;

	short sInputDataSize_PA;
	short sOutputDataSize_PA;
	short sInputDataSize_PB;
	short sOutputDataSize_PB;

	CM001_INPUT_ENC EncBoard;
	CM001_INPUT_SENSOR SensorBoard;
	CM001_OUTPUT_PWM PwmBoard;
	CM001_OUTPUT_DA DaBoard;
	CM001_PIO PioBoardA;
	CM001_PIO PioBoardB;

	BOOL ExtMode;

};


#endif
