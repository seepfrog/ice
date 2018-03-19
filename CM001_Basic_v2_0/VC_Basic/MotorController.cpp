/***********************************************************************************

		MB_MC_.cpp

		Functions for MB-MC_002 Motor Controller
	
													2007.2.5 M.Utsumi@SymphonyMax B 

************************************************************************************/

#include<windows.h>
#include<stdio.h>
#include<conio.h>
#include<string.h>

#include "Measurement.h"
#include "MotorController.h"

MotorController::MotorController()
{
	ZeroMemory( &Data, sizeof(MC_DATA) );
	ZeroMemory( &Ref,  sizeof(MC_REFERENCE) );
	CheckSumEnable = TRUE;

}

MotorController::~MotorController()
{
}


void MotorController::PrintData( void )
{
	printf("Ctrl %4d Sensor %04x %04x %04x\r\n",
		Data.Status.bControl, Data.Status.bSensor1, Data.Status.bSensor2, Data.Status.bSensor3 );

	printf("pos %5.1f rpm %5.3f trq %5.3f count %x\r\n",RADtoDEG*Data.fPosition, RADSECtoRPM*Data.fVelocity, Data.fTorque, Data.lCount );
 
}

void MotorController::PrintCtrlParam( PMC_CTRL_PARAM pParam )
{
	printf("kp %5.2f kv %5.2f kd %5.2f ki %5.2f\r\n"
		,pParam->kp,pParam->kd,pParam->kv,pParam->ki  );
	printf("MinEn %5.2f[deg] PosLimit high %5.2f[deg] low %5.2f[deg]\r\n"
		,pParam->fMinEn*RADtoDEG, pParam->fPosLimit_h*RADtoDEG, pParam->fPosLimit_l*RADtoDEG  );
	printf("DutyLimit %5.2f\r\n"
		,pParam->fDutyLimit );
}

void MotorController::PrintMotorParam( PMC_MOTOR_PARAM pMotParam )
{
	printf("ke %5.2f[mV/rpm] kt %5.2f[mNm/A] rm %5.2f[orm] vcc %5.2f[V]\r\n"
		,pMotParam->ke/RADSECtoRPM*1000.0f, pMotParam->kt, pMotParam->rm, pMotParam->vcc  );
	printf("kg %5.2f[N:1]\r\n"
		,pMotParam->kg);
	printf("ken %5.2f[pulse/rev] kpt %5.2f[deg/ad] pot offset %5.2f[deg]\r\n"
		,pMotParam->ken, pMotParam->kpt*RADtoDEG, pMotParam->fPotOffset*RADtoDEG );
}

BOOL MotorController::Stop( void )
{
	dbg_printf("Stop: ");

	return( Transceive(MC_STOP, NULL, 0, NULL, 0 ));
}

BOOL MotorController::SetCtrlParam( PMC_CTRL_PARAM pParam )
{
	BOOL ret;
	dbg_printf("Set Param: ");

	ret =  Transceive(MC_SET_CTRL_PARAM, pParam, sizeof(MC_CTRL_PARAM ), NULL, 0 );
	Sleep(100);
	return( ret );
}

BOOL MotorController::GetCtrlParam( PMC_CTRL_PARAM pParam )
{
	dbg_printf("Get Param: ");

	return(  Transceive( MC_GET_CTRL_PARAM, NULL, 0, pParam, sizeof( MC_CTRL_PARAM ) ) );
}

BOOL MotorController::SetMotorParam( PMC_MOTOR_PARAM pParam )
{
	BOOL ret;

	dbg_printf("Set Param: ");

	ret = Transceive(MC_SET_MOTOR_PARAM, pParam, sizeof(MC_MOTOR_PARAM ), NULL, 0   );
	Sleep(100);
	return( ret );
}

BOOL MotorController::GetMotorParam( PMC_MOTOR_PARAM pParam )
{
	dbg_printf("Get Param: ");

	return( Transceive( MC_GET_MOTOR_PARAM, NULL, 0, pParam, sizeof( MC_MOTOR_PARAM ) ) );
}

BOOL MotorController::SetPosition( void )
{

	dbg_printf("Set Position: ");

	return( Transceive(MC_SET_POSITION, &Ref, sizeof(MC_REFERENCE ), &Data, sizeof(MC_DATA ) ) );
}

BOOL MotorController::SetVelocity( void )
{
	dbg_printf("Set Velocity: ");

	return( Transceive(MC_SET_VELOCITY, &Ref, sizeof(MC_REFERENCE ), &Data, sizeof(MC_DATA ) ));
}

BOOL MotorController::GetData( void )
{
	dbg_printf("Get Data: ");

	return( Transceive(MC_GET_DATA, NULL, 0, &Data, sizeof(MC_DATA ) ));
}

BOOL MotorController::ClearData( void )
{
	dbg_printf("Clear Data: ");

	return( Transceive(MC_CLEAR_DATA, NULL, 0, NULL, 0 ));
}

BOOL MotorController::SetTorque( void  )
{
	dbg_printf("Set Current: ");

	return( Transceive(MC_SET_TORQUE, &Ref.fTorque, sizeof( float ), &Data, sizeof(MC_DATA ) ));
}

BOOL MotorController::SetDuty( SHORT sDuty  )
{

	dbg_printf("Set Duty: ");
	return( Transceive(MC_SET_DUTY, &sDuty, sizeof( SHORT ),  &Data, sizeof(MC_DATA ) ));
}

BOOL MotorController::AdjustEnc( void )
{
	dbg_printf("Adjust Encoder: ");

	return( Transceive(MC_ADJUST_ENC, NULL, 0, NULL, 0 ));
}

BOOL MotorController::SetPosSensor( BYTE bSensorType )
{
	dbg_printf("SetPotMode: ");

	return( Transceive(MC_SET_POS_SENSOR, &bSensorType, 1, NULL, 0 ));
}

BOOL MotorController::TestMode( BOOL on )
{
	return( Transceive( MC_TEST_MODE, &on, 1, NULL, 0 ) );
}

BOOL MotorController::SetSimMotor( float fLoad )
{
	dbg_printf("SetSimMotor: ");

	return( Transceive(MC_SIM_MOTOR, &fLoad, sizeof(float), NULL, 0 ));
}

BOOL MotorController::ResetSimMotor( void )
{
	dbg_printf("ResimMotor: ");
	return( SetSimMotor( -1.0f) );
}



