/****************************************************************
	Motor.h 
	
	Motor control functions for CM001
	2ch PWM output

								2010.4.23Å@M.Utsumi@ArcDevice

*****************************************************************/

#ifndef MOTOR_PWM_H
#define MOTOR_PWM_H

#include "Hardware.h"

#define MOTOR_MAX_VALUE    1024

void MOTOR_Open( void );
void MOTOR_Drive( WORD wCh, short sDuty );

#define MOTOR_Brake() MOTOR_Drive( 0, MOTOR_MAX_VALUE+1 )
#define MOTOR_Free() MOTOR_Drive( 0, -(MOTOR_MAX_VALUE+1) )

#endif
