/***********************************************************************************

		SC02.cpp	SC02 Control class
	
													2010.4.19 M.Utsumi@ArcDevice

************************************************************************************/

#include"windows.h"
#include<stdio.h>
#include<conio.h>
#include<string.h>

//#define DEBUG

#include "Measurement.h"
#include "SC02.h"

DWORD  dwNumberOfBytesRW; 

SC02::SC02()
{
	hUSB = NULL;
	bComDeviceCount = 0;
	iTxBufCount = 0;
	iRxBufCount = 0;

// resize
	if( TxBuffer )	delete TxBuffer;
	if( RxBuffer )	delete RxBuffer;

	TxBuffer = new BYTE[SC_BUF_SIZE];
	RxBuffer = new BYTE[SC_BUF_SIZE];

	ZeroMemory( TxBuffer, SC_BUF_SIZE );
	ZeroMemory( RxBuffer, SC_BUF_SIZE );
	
	dwTimeout_ms = 20000;

}

SC02::~SC02()
{
	if( hUSB ) Close();
}

BOOL SC02::DeviceOpen( DWORD dwIndex, BYTE bTargetID, DWORD dwAttrsAndFlags )
{
		DWORD dwLocID;


		ftStatus = FT_ListDevices( (PVOID)dwIndex, &dwLocID, FT_LIST_BY_INDEX|FT_OPEN_BY_LOCATION);
	

		hUSB = FT_W32_CreateFile( (LPCTSTR)dwLocID,GENERIC_READ|GENERIC_WRITE,0,0,
								OPEN_EXISTING,
								dwAttrsAndFlags,
								0);
		if (hUSB == INVALID_HANDLE_VALUE) return( FALSE );
		
		ftStatus = FT_SetTimeouts( hUSB, dwTimeout_ms, dwTimeout_ms );

		if( bTargetID == CM_MASTER_ID )	return( TRUE );
		
		GetID();
		if( bTargetID != bID ){
			FT_W32_CloseHandle( hUSB );
			return( FALSE );
		}

		return( TRUE );
}

BOOL SC02::Open( BYTE bTargetID, BOOL Overlapped )
{
	FT_STATUS ftStatus;
	DWORD dwNumDevice, dwIndex, dwAttrsAndFlags;

	dbg_printf("Open Serial Hub ID %d: ", bTargetID );

	ftStatus = FT_ListDevices( &dwNumDevice, NULL, FT_LIST_NUMBER_ONLY );
	dbg_printf( "FT device num %d\r\n", dwNumDevice );
	
	if( dwNumDevice == 0 ){
		sprintf_s(  Error, "No FT device found\r\n");
		return( FALSE );
	}
	if( Overlapped ){
		dwAttrsAndFlags = FILE_ATTRIBUTE_NORMAL | FT_OPEN_BY_LOCATION | FILE_FLAG_OVERLAPPED;
		readEvent  = CreateEvent(0, FALSE, FALSE, 0);
		writeEvent = CreateEvent(0, FALSE, FALSE, 0);
		overlappedRead.Offset = 0;
		overlappedWrite.Offset = 0;
		overlappedRead.OffsetHigh = 0;
		overlappedWrite.OffsetHigh = 0;
		overlappedRead.hEvent  = readEvent;
		overlappedWrite.hEvent = writeEvent;

	}else{
		dwAttrsAndFlags = FILE_ATTRIBUTE_NORMAL | FT_OPEN_BY_LOCATION;
		readEvent = NULL;
		writeEvent = NULL;
	}

	for( dwIndex=0; dwIndex<dwNumDevice; dwIndex++){
		if( DeviceOpen( dwIndex, bTargetID, dwAttrsAndFlags ) ){
			ftStatus = FT_SetLatencyTimer( hUSB, 0 );
			return( TRUE );
		}
	}

	sprintf_s( Error, "Target ID device not found\r\n" );

	return( FALSE );
}


BOOL SC02::Close( void  )
{

	FT_W32_CloseHandle( hUSB );
	hUSB = NULL;

	return( TRUE );
}


BOOL SC02::Transceive( BYTE bCommand, void* pTxBuffer, WORD wTxSize, void* pRxBuffer, WORD wRxSize, BOOL RightNow )
{
	WORD wSize;

	if( hUSB ==NULL ) return( FALSE );

	wSize = 1+wTxSize; // Add Command

	dbg_printf("Tx Size %d\r\n", wSize );

	TxBuffer[0] = bCommand;

	if( wTxSize != 0 ) memcpy( &TxBuffer[1], pTxBuffer, wTxSize );

	if( !FT_W32_WriteFile( hUSB, TxBuffer, wSize, &dwNumberOfBytesRW, NULL ) ){
		sprintf_s( Error, "Fail in Write No. %d\r\n", FT_W32_GetLastError(hUSB) );
		return( FALSE );
	}

	wSize = wRxSize + 1; // Add SH status
	dbg_printf("TotalRx size %d\r\n",wSize );

	if( !FT_W32_ReadFile(  hUSB,  RxBuffer, wSize, &dwNumberOfBytesRW, NULL)){
		sprintf_s( Error, "Fail in Read Error No.%d\r\n", FT_W32_GetLastError(hUSB) );
		return( FALSE );
	}

	bStatus = RxBuffer[dwNumberOfBytesRW-1];

	if(wSize!=dwNumberOfBytesRW){
		if( bStatus != CM_NORMAL ){
			sprintf_s( Error, "Read size missmatch: required %d  actual %d  Status %d\r\n",wSize, dwNumberOfBytesRW, bStatus );
			return( FALSE );
		}
		wRxSize = (WORD)dwNumberOfBytesRW;
	}
	memcpy(  pRxBuffer, RxBuffer, wRxSize );

	return(TRUE);
}


BOOL SC02::Transfer( HANDLE hChildModule, void* pTxBuffer, WORD wTxSize, WORD wRxSize, BOOL RightNow )
{
	int size, i;

// Header registration 
	if(bComDeviceCount ==256 ) return( FALSE );
	pChildModule[ bComDeviceCount ] = (ControlModule*)hChildModule;

// Check transmit data size
	if( iTxBufCount==0 ){
		// device count + bPort + bID + wTxSize + wRxSize + Tx data size 
    	size = 1 + 2 + 4 + wTxSize;
		iTxBufCount = 1; // skip 1bytes data. device count
		iRxBufCount = 0;
	}else{
		size = iTxBufCount + 2 + 4 + wTxSize;
	}
	if( size > SC_BUF_SIZE ){
		sprintf_s( Error, "TX size exceed at port %d id %d module",
				pChildModule[ bComDeviceCount ]->bPort, pChildModule[ bComDeviceCount ]->bID );
		return( FALSE );
	}

// Check receive data size  RxSize
	size =	iRxBufCount +  2 + 2 + wRxSize;
	if( size > SC_BUF_SIZE ){
		sprintf_s( Error, "RX size exceed at port %d id %d module",
				pChildModule[ bComDeviceCount ]->bPort, pChildModule[ bComDeviceCount ]->bID );
		return( FALSE );
	}

////////////////////////////////////////////////////////////////////////////////////
	// Copy transmit header
	TransferTxBuffer[ iTxBufCount++ ] = pChildModule[ bComDeviceCount ]->bPort;
	TransferTxBuffer[ iTxBufCount++ ] = pChildModule[ bComDeviceCount ]->bID;
	memcpy( &TransferTxBuffer[ iTxBufCount ], &wTxSize, 2 );
	iTxBufCount += 2;
	memcpy( &TransferTxBuffer[ iTxBufCount ], &wRxSize, 2 );
	iTxBufCount += 2;

	// Copy transmit data and add TX and RX data size
	memcpy( &TransferTxBuffer[ iTxBufCount ], pTxBuffer, wTxSize );

	iTxBufCount += wTxSize;
	iRxBufCount += 4 + wRxSize; // add header
	bComDeviceCount++;

////////////////////////////////////////////////////////////////////////////////////

// Transceive 

	if( RightNow ){
		TransferTxBuffer[ 0 ] = bComDeviceCount;

		if( !Transceive( CM_TRANSFER, TransferTxBuffer, iTxBufCount, TransferRxBuffer, iRxBufCount ) ){
			dbg_printf("Transceive Error %s \r\n", Error );
			return( FALSE );
		}

	// Copy received data to registrated pointers
		BYTE* pBuffer;
		BYTE id, port;
		WORD rx_size;

		pBuffer = TransferRxBuffer;
		
		for(i=0; i<bComDeviceCount; i++ ){
			port = *pBuffer++;
			id = *pBuffer++;
			memcpy( &rx_size, pBuffer, 2 );
			pBuffer +=2;

			pChildModule[i]->Receive( pBuffer, rx_size );
			pBuffer += rx_size;
		}
	// End process
		bComDeviceCount = 0;
		iTxBufCount = 0;
		iRxBufCount = 0;
	}

	return(TRUE);
}

BOOL SC02::ResetModules( BYTE bPortNum  )
{
	return( Transceive( SC_RESET_MODULES, &bPortNum, 1, NULL, 0 ) );
}

