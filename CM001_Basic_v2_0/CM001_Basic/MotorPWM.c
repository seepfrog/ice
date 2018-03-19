/****************************************************************
	Motor.c 
	
	Motor control functions for PWM input motor drivers
	Contorol board CM001-A01  

							2010.4.23 M.Utsumi@ArcDevice

*****************************************************************/

#include"pwm.h"
#include"MotorPWM.h"
#include"Encoder.h"

// Pin assignment
// ch 1
// P1 VCC----VCC
// P2 DIR----RE2
// P3 ENB----RE0	// brake is active low 
// P4 PWM----RE1	// pwm 1H
// P5 GND----GND

// ch2
// P1 VCC----VCC
// P2 GND----GND
// P4 DIR----RE3
// P5 ENB----RE4	// brake is active low 
// P3 PWM----RE5	// pwm 3H
// P6 NC 
// P7 NC 
// P8 NC 
// P9 NC 


#define DIR0	_LATE2
#define ENB0	_LATE0	// L6203:ENABLE  iMDs03:BRAKE
#define DIR1	_LATE3
#define ENB1	_LATE4	// L6203:ENABLE  iMDs03:BRAKE

#define TRIS_DIR0		TRISEbits.TRISE2
#define TRIS_ENB0		TRISEbits.TRISE0	
#define TRIS_DIR1		TRISEbits.TRISE3
#define TRIS_ENB1		TRISEbits.TRISE4	
/*
#define DIR2	_LATD1 
#define ENB2	_LATD2	// L6203:ENABLE  iMDs03:BRAKE
#define TRIS_DIR2		TRISDbits.TRISD1
#define TRIS_ENB2		TRISDbits.TRISD2 
*/

void MOTOR_Open( void )
{
	unsigned int config1 = PWM_EN & PWM_IDLE_STOP & PWM_OP_SCALE1 &
						PWM_IPCLK_SCALE1 & PWM_MOD_FREE;
	unsigned int config2 = PWM_MOD3_IND & PWM_MOD2_IND & PWM_MOD1_IND &
						PWM_PEN3H  & PWM_PDIS2H & PWM_PEN1H & 
						PWM_PDIS3L & PWM_PDIS2L & PWM_PDIS1L;
	unsigned int config3 = PWM_SEVOPS1 & PWM_OSYNC_TCY & PWM_UEN;
	

	TRIS_DIR0 = 0;
	TRIS_ENB0 = 0;
	DIR0 = 1;	// CW
	ENB0 = 1;	// L6203:ENABLE=ON  iMDs03:BRAKE=OFF
 
	TRIS_DIR1 = 0;
	TRIS_ENB1 = 0;
	DIR1 = 1;	// CW
	ENB1 = 1;	// L6203:ENABLE=ON  iMDs03:BRAKE=OFF
/*
	TRIS_DIR2 = 0;
	TRIS_ENB2 = 0;
	DIR2 = 1;	// CW
	ENB2 = 1;	// L6203:ENABLE=ON  iMDs03:BRAKE=OFF
*/
	OpenMCPWM( 511, 0, config1, config2, config3 );	//(511+1)*2=1024
	
}

void MOTOR_Close( void )
{
	CloseMCPWM();
}


void MOTOR_Drive( WORD wCh, short sDuty )
{
	int dir, enb;
	short sDuty_abs;	 
	
	if( sDuty > MOTOR_MAX_VALUE ){	// motor is free or brake
		dir = 0;
		enb = 0;	//  	motor brake
		sDuty_abs = 0;
		sDuty = 0;
	}else if( sDuty==0 ){		// STOP
		dir = 0;
		enb = 1;	//
		sDuty_abs = 0;
		sDuty = 0;
	}else if( sDuty>0 ){		// CW
		dir = 1;	//
		enb = 1;
		sDuty_abs =  sDuty;
	}else{					// CCW
		dir = 0;	//
		enb = 1;
		sDuty_abs = -sDuty;
	}
	switch( wCh ){
	case 0:
		DIR0 = dir;
		ENB0 = enb;
	 	
		SetDCMCPWM( 1,  sDuty_abs, 0  );
		break;
	case 1:
		DIR1 = dir;
		ENB1 = enb;
	 	
		SetDCMCPWM( 3,  sDuty_abs, 0  );
		break;
	case 2:
/*
		DIR2 = dir;
		ENB2 = enb;
	 	
		SetDCMCPWM( 2,  sDuty_abs, 0  );
*/
		break;
	}

	if( ENCODER_SimMode ) ENCODER_SimCount(sDuty);
}

