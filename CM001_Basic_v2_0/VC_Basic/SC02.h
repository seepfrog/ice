/***********************************************************************************

		SC02.cpp	SC02 Control class
	
													2010.4.19 M.Utsumi@ArcDevice

************************************************************************************/

#ifndef _INC_SC02_H
#define _INC_SC02_H

#ifdef  __cplusplus
extern "C" {
#endif

#include"ControlModule.h"
#include"ftd2xx.h"

// No.20 to 127 for Modules


#define SC_RESET_MODULES	20

#define SC_BUF_SIZE 256

class SC02 : public ControlModule
{
public:

	SC02();
	~SC02();
    
	BOOL DeviceOpen( DWORD dwIndex, BYTE bTargetID, DWORD dwAttrsAndFlags );
	BOOL Open( BYTE bTargetID, BOOL Overlapped=FALSE );
	BOOL Close( void );
	void PrintData( void );

	BOOL Transceive( BYTE bCommand, void* pTxBuffer, WORD wTxSize, 
											void* pRxBuffer, WORD wRxSize, BOOL RightNow=TRUE );
    BOOL Transfer( HANDLE hChildModule, void* pTxBuffer, WORD wTxSize, WORD wRxSize, BOOL RightNow );
	BOOL ResetModules( BYTE bPortNum  );

	BYTE bComDeviceCount;

private:
	FT_HANDLE hUSB;
	FT_STATUS ftStatus;

	BYTE TransferTxBuffer[SC_BUF_SIZE];
	BYTE TransferRxBuffer[SC_BUF_SIZE];
	int iTxBufCount; 
	int iRxBufCount; 
	ControlModule* pChildModule[256];

	OVERLAPPED overlappedRead, overlappedWrite;
   	HANDLE readEvent, writeEvent;

	DWORD  dwTimeout_ms;
	DWORD  dwNumberOfBytesRW; 

};

#ifdef  __cplusplus
}
#endif


#endif


