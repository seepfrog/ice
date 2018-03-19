/*
	Motor Control Functions
						
					2008.8.27 M.Ustumi@EasLogic

*/

#ifndef MOTOR_CTRL_H
#define MOTOR_CTRL_H

#define MC_STOP 				20
#define MC_EMG_STOP			21
#define MC_SET_CTRL_PARAM 		22
#define MC_GET_CTRL_PARAM 		23
#define MC_SET_MOTOR_PARAM 		24
#define MC_GET_MOTOR_PARAM 		25
#define MC_SET_POSITION		26
#define MC_SET_VELOCITY 		27
#define MC_SET_TORQUE 			28
#define MC_SET_DUTY 			29
#define MC_GET_DATA	 		30
#define MC_CLEAR_DATA	 		31
#define MC_SET_POS_SENSOR		32		
#define MC_ADJUST_ENC			33
#define MC_SIM_MOTOR 			34
#define MC_TEST_MODE	 		35
#define MC_ECHO				36

#define MC_SENSOR_ERROR		150
#define MC_DA_ALARM			151

#define MC_POS_SENS_POTENTIO	0
#define MC_POS_SENS_ENCODER		1

#include "ControlModule.h"
#include"MotorPWM.h"
#include"MotorDA.h"
#include"Encoder.h"
#include"Ad.h"

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

typedef struct _MC_MOTOR_PARAM
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
	float fPosition;
	float fVelocity;
	float fTorque;

} MC_REFERENCE, *PMC_REFERENCE;


#define M_PI 3.1415f
#define M_2PI 6.283f
#define DEGtoRAD 0.01745329252f 
#define RADtoDEG 57.29577951f
#define RADSECtoRPM 9.549296586 
#define RPMtoRADSEC 0.1047197551f

extern volatile char MC_ControlTrig;

void MC_Open( void );
void MC_Stop( void );
void MC_SetSimMode( float load );
BOOL MC_CommandProc( BYTE command );
void MC_PresetData( void );
BOOL MC_ControlCommandProc( BYTE command );
void MC_Control( void );

#endif
