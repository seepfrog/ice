/*
	MotorDA.c 
	
	Motor control functions for Analog input Drivers
	FB002-A01

								2010.4.23Å@M.Utsumi@ArcDevice
*/

#include"pwm.h"

#include"Encoder.h"
#include"MotorDA.h"

// DA
// SDIN------RD0 OUT
// ~SYNC-----RD1 OUT active low
// SCLK------RD2 OUT
// ALARM-----RE4 IN  active low
// CTRL1-----RE2 OUT active low open corrector
// CTRL2-----RE0 OUT active low


// DA
#define SDIN	  	_LATD0
#define SYNC	  	_LATD1	
#define SCLK		_LATD2
#define ALARM 	_RE4
#define CTRL1  	_LATE2
#define CTRL2  	_LATE0

#define TRIS_SDIN		_TRISD0
#define TRIS_SYNC		_TRISD1	
#define TRIS_SCLK		_TRISD2	
#define TRIS_ALARM		_TRISE4	
#define TRIS_CTRL1		_TRISE2
#define TRIS_CTRL2		_TRISE0	


#ifdef DRV_HPTECH_uSD12
#define 	ServoON()		CTRL1 = 1; 
#define 	ServoOFF()	CTRL1 = 0; 	 
#endif

#ifdef DRV_MAXON_DES70
#define Enable() 		CTRL2 = 1;
#define Disable() 		CTRL2 = 0;
#endif

#ifdef DRV_TITECH_JW143_2
#define BrakeON()	 	CTRL1 = 1;
#define BrakeOFF()	 	CTRL1 = 0;
#define FreeON() 		CTRL2 = 0;
#define FreeOFF() 		CTRL2 = 1;
#endif


BOOL MOTOR_DA_Alarm( void )
{
	// active low
	if( ALARM ) return( FALSE );
	return( TRUE );
}

void MOTOR_DA_SetVoltage( WORD wValue )
{
	int bits = 16;

	wValue = ( wValue << 2 ) & 0x3FFC; // 00xx xxxx xxxx xx00			
	
	SYNC = 0;
	while(bits--){
		SCLK = 1;
		if( wValue & 0x8000 ) SDIN = 1;
		else SDIN = 0;
		wValue =wValue<<1; 
		SCLK = 0;
	}			
	SYNC = 1;
	SDIN = 0;
}

#ifdef DRV_HPTECH_uSD12

void MOTOR_DA_Open( void )
{
	ServoOFF();	 // servo off 

	TRIS_SDIN = 0;
	TRIS_SYNC = 0;
	TRIS_SCLK = 0;
	TRIS_ALARM = 1; // input
	TRIS_CTRL1 = 0;
	TRIS_CTRL2 = 0;

	SDIN = 0;
	SYNC = 1;
	SCLK = 0;

}

void MOTOR_DA_Close( void )
{
	ServoOFF();	 // servo off 
}

void MOTOR_DA_Drive( short sDuty )
{
	if( sDuty > MOTOR_DA_MAX_VALUE ){	// motor disable
		sDuty = 0;
		ServoOFF();
	}else if( sDuty==0 ){		// STOP
		ServoOFF();
	}else if( sDuty>0 ){		// CW
		ServoON();
	}else{					// CCW
		ServoON();
	}

	MOTOR_DA_SetVoltage( sDuty + MOTOR_DA_MAX_VALUE  );

	if( ENCODER_SimMode ) ENCODER_SimCount( sDuty );

}
#endif


#ifdef DRV_MAXON_DES70

void MOTOR_DA_Open( void )
{
	Disable();	 // servo off 

	TRIS_SDIN = 0;
	TRIS_SYNC = 0;
	TRIS_SCLK = 0;
	TRIS_ALARM = 1; // input
	TRIS_CTRL1 = 0;
	TRIS_CTRL2 = 0;

	SDIN = 0;
	SYNC = 1;
	SCLK = 0;
}

void MOTOR_DA_Close( void )
{
	Disable();	 // servo off 
}

void MOTOR_DA_Drive( short sDuty )
{
	if( sDuty > MOTOR_DA_MAX_VALUE ){	// motor disable
		sDuty = 0;
		Disable();
	}else if( sDuty==0 ){		// STOP
		Disable();
	}else if( sDuty>0 ){		// CW
		Enable();
	}else{					// CCW
		Enable();
	}

	MOTOR_DA_SetVoltage( sDuty + MOTOR_DA_MAX_VALUE  );

	if( ENCODER_SimMode ) ENCODER_SimCount( sDuty );

}
#endif


#ifdef DRV_TITECH_JW143_2

short OffsetVal;

void MOTOR_DA_Open( void )
{
	BrakeOFF(); 
	FreeON();	 

	TRIS_SDIN = 0;
	TRIS_SYNC = 0;
	TRIS_SCLK = 0;
	TRIS_ALARM = 1; // input
	TRIS_CTRL1 = 0;
	TRIS_CTRL2 = 0;

	SDIN = 0;
	SYNC = 1;
	SCLK = 0;

	OffsetVal = MOTOR_DA_FULLSCALE + MOTOR_DA_MAX_VALUE;

}

void MOTOR_DA_Close( void )
{
	BrakeOFF(); 
	FreeON();	 
}

void MOTOR_DA_Drive( short sDuty )
{
	if( sDuty > MOTOR_DA_MAX_VALUE ){	// motor disable
		sDuty = 0;
		BrakeON();
	}else if( sDuty==0 ){		// STOP
		FreeON();
	}else if( sDuty>0 ){		// CW
		BrakeOFF();
		FreeOFF();
	}else{					// CCW
		BrakeOFF();
		FreeOFF();
	}

	MOTOR_DA_SetVoltage( sDuty + OffsetVal  );

	if( ENCODER_SimMode ) ENCODER_SimCount( sDuty );

}

#endif


