/*****************************************************************************************************

	CM001.cpp

	CM001 basic mode Ver.1.0
	 
	
													2009.2.23 M.Utsumi@EasLogic 

******************************************************************************************************/

#include<windows.h>
#include<stdio.h>
#include<math.h>
#include"CM001.h"

CM001::CM001()
{
	CheckSumEnable = TRUE;

	bBoardID_PA = 0;
	bBoardID_PB = 0;
	sInputDataSize = 0;
	sOutputDataSize = 0;

	ExtMode = FALSE;

}

CM001::~CM001()
{

}

BOOL CM001::Open( HANDLE hCm, int port, BYTE id )
{
	ControlModule::Open( hCm, port, id );
	return( GetConfig() );
}

BOOL CM001::GetConfig( void  )
{
	BYTE buf[16];

	if( !Transceive( CM001_GET_CONFIG, NULL, 0, buf, 6 ) ) return( FALSE );

	bBoardID_PA = buf[0];
	bBoardID_PB = buf[1];
	memcpy( &sInputDataSize,  &buf[2], 2 ); // Input size
	memcpy( &sOutputDataSize, &buf[4], 2 ); // Output size

	return( TRUE );

}

BOOL CM001::Exchange( void* pOutputData, void* pInputData, BOOL RightNow )
{
	return( Transceive( CM001_EXCHANGE, pOutputData, sOutputDataSize, pInputData, sInputDataSize, RightNow ) );
}

BOOL CM001::Exchange( BOOL RightNow )
{
	BYTE buf_in[256];
	BYTE buf_out[256];
	BYTE* pBuf;

	pBuf = buf_out;
	if( bBoardID_PA == CM001_PIO_BOARD ){
		memcpy( buf_out, (BYTE*)&PioBoardA, sizeof( CM001_PIO ));
		pBuf+= sizeof( CM001_PIO );
	}
	switch( bBoardID_PB ){
	case CM001_PWM_BOARD:
		memcpy( pBuf, (BYTE*)&PwmBoard, sizeof( CM001_OUTPUT_PWM ));
		break; // PWM board
	case CM001_DA_BOARD:
		memcpy( pBuf, (BYTE*)&DaBoard, sizeof( CM001_OUTPUT_DA ));
		break; // DA board
	case CM001_PIO_BOARD:
		memcpy( pBuf, (BYTE*)&PioBoardB, sizeof( CM001_PIO ));
		break; // PIO board	
	}
	ExtMode = TRUE;
	
	if( !Transceive( CM001_EXCHANGE, buf_out, sOutputDataSize, buf_in, sInputDataSize, RightNow ) ) return( FALSE );

	return( TRUE );
}


BOOL CM001::Receive( BYTE *pRxBuffer, WORD wSize )
{
	BYTE* pBuf;

	if( wSize != wReqRxSize ){
		sprintf( Error, "Rx Data size miss match %d %d\r\n", wReqRxSize,wSize );
		
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
		if( ExtMode ){
			pBuf = pRxBuffer;
			switch( bBoardID_PA){
			case CM001_ENC_BOARD:
				memcpy( &EncBoard, pBuf, sizeof( CM001_INPUT_ENC ) );
				pBuf += sizeof( CM001_INPUT_ENC );
			break;
			case CM001_SENSOR_BOARD:
				memcpy( &SensorBoard, pBuf, sizeof( CM001_INPUT_SENSOR ));
				pBuf += sizeof( CM001_INPUT_SENSOR );
				break;
			case CM001_PIO_BOARD:
				memcpy( &PioBoardA, pBuf, sizeof( WORD ) );
				pBuf+=sizeof( WORD );
				break;
			}
			if( bBoardID_PB == CM001_PIO_BOARD ){
				memcpy( &PioBoardB, pBuf, sizeof( WORD ) );
			}
			ExtMode = FALSE;

		}else{
			memcpy( pReserveBuffer, pRxBuffer, wSize );
		}
	}

	bStatus = pRxBuffer[wSize];
	
	if( bStatus!=CM_NORMAL ) return( FALSE );

	return( TRUE );
}

