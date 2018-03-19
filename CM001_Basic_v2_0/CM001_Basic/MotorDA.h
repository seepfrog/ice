/****************************************************************
	MotorDA.h 
	
	Motor control functions for Anlog input drivers

	FB002-A01
							2010.4.23Å@M.Utsumi@ArcDevice

****************************************************************/

#ifndef MOTOR_DA_H
#define MOTOR_DA_H

#include "Hardware.h"

#ifdef DRV_TITECH_JW143_2
#define MOTOR_DA_MAX_VALUE   0x03FF
#define MOTOR_DA_FULLSCALE   0x07FF
#else
#define MOTOR_DA_MAX_VALUE   0x07FF
#endif

void MOTOR_DA_Open( void );
BOOL MOTOR_DA_Alarm( void );
void MOTOR_DA_Drive( short sDuty );

#define MOTOR_DA_Brake() MOTOR_DA_Drive( MOTOR_DA_MAX_VALUE+1 )
#define MOTOR_DA_Free() MOTOR_DA_Drive( 0 )

#endif
