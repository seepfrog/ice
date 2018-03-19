/***********************************************************************************

		MotorController.h

		Motor Control Functions
	
													2008.3.26 M.Utsumi@SymphonyMax 

************************************************************************************/

#ifndef _INC_MOTOR_CONTROLLER_H
#define _INC_MOTOR_CONTROLLER_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "ControlModule.h"

#define MC_STOP 				20
#define MC_EMG_STOP				21
#define MC_SET_CTRL_PARAM 		22
#define MC_GET_CTRL_PARAM 		23
#define MC_SET_MOTOR_PARAM 		24
#define MC_GET_MOTOR_PARAM 		25
#define MC_SET_POSITION			26
#define MC_SET_VELOCITY 		27
#define MC_SET_TORQUE 			28
#define MC_SET_DUTY 			29
#define MC_GET_DATA	 			30
#define MC_CLEAR_DATA	 		31
#define MC_SET_POS_SENSOR		32		
#define MC_ADJUST_ENC			33
#define MC_SIM_MOTOR 			34
#define MC_TEST_MODE	 		35
#define MC_ECHO					36


#define MC_SENSOR_ERROR			150
#define MC_DA_ALARM				151

#define MC_POS_SENS_POTENTIO	0
#define MC_POS_SENS_ENCODER		1


typedef struct _MC_CTRL_PARAM
{
	float kp;			// 位置制御のPゲイン
	float kd;			// 位置制御のDゲイン
	float kv;			// 速度制御のPゲイン
	float ki;			// 速度制御のIゲイン
	float fMinEn;		// 位置制御の最小誤差[rad]or[m]or[mm]
	float fPosLimit_h;	// 位置リミットHigh[rad]or[m]or[mm]
	float fPosLimit_l;	// 位置リミットLow[rad]or[m]or[mm]
	float fDutyLimit;	// 設定できるDuty値（DAの電圧）の制限（アラーム対策等）
						// 0でデフォルトの最大値に設定
} MC_CTRL_PARAM, *PMC_CTRL_PARAM;

typedef struct _MBMC_MOTOR_PARAM
{
	float ke;			// 逆起電力定数[V/(rad/sec)]
	float kt;			// トルク定数[mNm/A]
	float rm;			// 端子間抵抗[orm]
	float vcc;			// 公称電圧[V]
	float kg;			// ギア比(output/motor)
	float ken;			// エンコーダパルス数[pulse/rev]
	float kpt;			// ポテンショメータ変換係数[rad/ad]or[m/ad]or[mm/ad]
	float fPotOffset;	// ポテンショメータの位置オフセット[rad]or[m]or[mm]
} MC_MOTOR_PARAM, *PMC_MOTOR_PARAM;


typedef struct _MC_STATUS 
{
	BYTE bControl;
	BYTE bSensor1;
	BYTE bSensor2;
	BYTE bSensor3;
} MC_STATUS, *PMC_STATUS;

typedef struct _MC_DATA
{
	MC_STATUS Status;
	float fPosition;
	float fVelocity;
	float fTorque;
	long  lCount;
} MC_DATA, *PMC_DATA;


typedef struct _MC_REFERENCE
{
	float	fPosition;
	float	fVelocity;
	float	fTorque;
} MC_REFERENCE, *PMC_REFERENCE;


class MotorController : public ControlModule
{
public:

	MotorController();
	~MotorController();

	void PrintData( void );
	void PrintCtrlParam( PMC_CTRL_PARAM pParam );
	void PrintMotorParam( PMC_MOTOR_PARAM pMotParam );
	
	BOOL Stop( void  );
	BOOL SetCtrlParam( PMC_CTRL_PARAM pParam );
	BOOL GetCtrlParam( PMC_CTRL_PARAM pParam );
	BOOL SetMotorParam( PMC_MOTOR_PARAM pParam );
	BOOL GetMotorParam( PMC_MOTOR_PARAM pParam );
	BOOL SetPosition( void );
	BOOL SetVelocity( void );
	BOOL SetTorque( void  );
	BOOL SetDuty( SHORT sDuty );
	BOOL GetData( void );
	BOOL ClearData( void  );
	BOOL AdjustEnc( void  );
	BOOL SetPosSensor( BYTE bSensorType );
	BOOL SetSimMotor( float fLoad );
	BOOL ResetSimMotor( void );
	BOOL TestMode( BOOL on );
	BOOL Echo( char ch, char* ech );

	MC_DATA Data;
	MC_REFERENCE Ref;
};

#ifdef  __cplusplus
}
#endif


#endif


