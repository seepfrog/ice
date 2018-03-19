/*
	Motor Control Functions
						
					2009.2.19 M.Ustumi@EasLogic

*/

#include"stdio.h"
#include"math.h"
#include"timer.h"

#include"Hardware.h"
#include"MotorController.h"

extern float dt;
extern float CtrlTimeout;
extern float ComTimeout;


float kp;
float kd;
float kv;
float ki_dt;

float PosLimit_h;
float PosLimit_l;
float MinEn = 0;
float PotOffset = 0;

float ke;
float kt;
float rm;
float vcc;
float kg;
float ikg;
float ken;	// encoder
float kpt;	// potentiometor
float kave;
float ikave;
float ktd;		// trq to duty
float kdt;		// duty to trq
short sDutyLimit;
short sMaxDuty;

float kw;		// rev/pulse
float kdv;		// vcc/MOTOR_DA_MAX_VALUE
float ken_pos;	// encoder to position

float kdidt = 0;	// kd / dt
float ka;		// rad/pulse

float ei = 0;
float eo = 0;

MC_DATA Data;
MC_REFERENCE Ref;
char cPotMode;
short sDuty;
float fPosition, fVelocity, fRefVelocity;
float fTorque, fRefTorque;
BYTE bCtrlCommand;
UINT uiWatchdog;
BOOL TestMode;
BOOL Emergency = FALSE;

/*
MC_CTRL_PARAM CtrlParam = {
	//   kp, kd, kv, ki,
		 10.0f, 0.01f, 1.0f, 1.0f,
	//  fMinEn, fPosLimit_h, fPosLimit_l
		0.2f*DEGtoRAD, 300.0f*DEGtoRAD, -300.0f*DEGtoRAD, 
	//  fssDutyLimit 
		0
		};

MC_MOTOR_PARAM MotParam ={	// RE25
	// ke[V/(rad/sec)], kt[mNm/A], rm[ohm], vcc[V]
		1.0f/(RPMtoRADSEC*407.0f), 23.4f, 2.32f, 24.0f,
	// kg[1:N], ken[pulse/rev],
		374.0f, 1000.0f, 
	// kpt[rad/ad], fPotOffset
		4.25f/5.0f*M_2PI/1023.0f, 180.0f*DEGtoRAD
		};
*/

MC_CTRL_PARAM CtrlParam = {
	//   kp, kd, kv, ki,
		 10.0f, 0.01f, 1.0f, 1.0f,
	//  fMinEn, fPosLimit_h, fPosLimit_l
		0.2f*DEGtoRAD, 300.0f*DEGtoRAD, -300.0f*DEGtoRAD, 
	//  fssDutyLimit 
		0
		};

MC_MOTOR_PARAM MotParam ={	// RE25
	// ke[V/(rad/sec)], kt[mNm/A], rm[ohm], vcc[V]
		1.0f/(RPMtoRADSEC*407.0f), 23.4f, 2.32f, 24.0f,
	// kg[1:N], ken[pulse/rev],
		4.0f, 500.0f, 
	// kpt[rad/ad], fPotOffset
		4.25f/5.0f*M_2PI/1023.0f, 180.0f*DEGtoRAD
		};


int fTorqueCtrl( float td )
{
	int duty;
	
	duty =(int)( ktd*td );

	if(duty>sDutyLimit) duty = sDutyLimit;
	else if(duty<-sDutyLimit) duty =-sDutyLimit;
	
	return( duty );
}

int PI( float wd, float w, float td, float trq )
{
	float en;
	float duty, max_duty;
/*
	if( td < trq  ){
		max_duty = ktd*td;
		if(max_duty > dtmax ) max_duty = dtmax;
	}else{
		max_duty = dtmax;
	}
*/
	max_duty = ktd*td;
	if(max_duty > sDutyLimit ) max_duty = sDutyLimit;

	en = wd - w;
	ei+=ki_dt*en;
	if( ei>max_duty) ei = max_duty;
	else if( ei<-max_duty) ei =-max_duty;
	
	duty = kv*en+ei;
	
	if(duty>max_duty) duty = max_duty;
	else if(duty<-max_duty) duty =-max_duty;
	
	return( (int)duty );
}


float PD( float ref, float pos, float wd )
{
	float w, en, ed;

	if( ref > PosLimit_h ) ref = PosLimit_h;
	else if( ref < PosLimit_l ) ref = PosLimit_l;
	
	en = ref - pos;
	if( fabs(en)<=MinEn ) en = 0;
	else if( en > MinEn ) en-= MinEn;
	else en+= MinEn;

	ed = en - eo;
	w = kp * en - kdidt * ed;

	if( w > wd ) w = wd;
	else if( w < -wd ) w = -wd;
	
	eo = en;

	return( w );
}
void SetCtrlParam( PMC_CTRL_PARAM p )
{
	kp = p->kp;
	kdidt = p->kd/dt;
	kv = p->kv;
	ki_dt = p->ki*dt;
	PosLimit_h = p->fPosLimit_h;
	PosLimit_l = p->fPosLimit_l;
	MinEn = p->fMinEn;
	if( p->fDutyLimit == 0 ) sDutyLimit = sMaxDuty;
	else sDutyLimit = p->fDutyLimit;

}

void SetMotorParam( PMC_MOTOR_PARAM p )
{
	ke = p->ke;
	kt = p->kt/1000.0f;	// mNm/A -> Nm/A  
	rm = p->rm;
	vcc = p->vcc;
	kg = p->kg;
	ken = 4.0f*p->ken;		// 4x by QEC
	kpt = p->kpt;
	PotOffset = p->fPotOffset;
		
	kdt =  kt*vcc/rm / sMaxDuty;
	ktd = 1.0f/kdt;
	
	kw = 6.283f/dt/ken;
	ken_pos = 6.283f/ken/kg;	
	
	kdv = vcc / sMaxDuty;
	ikg = 1.0f/kg;
}

void MC_Stop( void )
{
	if( bBoardID_PB == DA_BOARD ) MOTOR_DA_Brake();
	else MOTOR_Brake();

	ei=0;			
}

void MC_Open( void )
{
	fPosition = 0;
	fVelocity = 0;
	fRefVelocity = 0;

	sDuty = 0;
	uiWatchdog = 0;

	if( bBoardID_PA == ENC_BOARD )	cPotMode = FALSE;
	else cPotMode = TRUE;
	
	if( bBoardID_PB == DA_BOARD ) sMaxDuty = MOTOR_DA_MAX_VALUE;
	else sMaxDuty = MOTOR_MAX_VALUE;

	ZeroMemory( (unsigned char*)&Ref, sizeof( MC_REFERENCE ));
	ZeroMemory( (unsigned char*)&Data, sizeof( MC_DATA ));

	SetCtrlParam( &CtrlParam );
	SetMotorParam( &MotParam );

	bCtrlCommand = MC_STOP;
	
	TestMode = FALSE;

	MC_PresetData();


}

void MC_SetSimMode( float load  )
{
	if( load >=0 )	ENCODER_SetSimMode( (1.0f-load)*vcc/ke/M_2PI*ken*dt/(float)sMaxDuty ); // max pulse/sec
	else ENCODER_ResetSimMode();
}

BOOL MC_CommandProc( BYTE command )
{
	float load;
	char ch;
	WORD wid;
	BOOL ret = TRUE;

	uiWatchdog = 0;

	switch( command ){
	case MC_STOP:
		CM_Reply();

		MC_Stop();		
		bCtrlCommand = MC_STOP;
		Emergency = FALSE;
		CM_bCtrlStatus = CM_NORMAL;
		CM_bStatus = CM_bCtrlStatus;
		 
		break;
	case MC_SET_CTRL_PARAM:
		CM_ReadBuf( (BYTE*)&CtrlParam, sizeof(MC_CTRL_PARAM) );
		CM_Reply();
	
		SetCtrlParam( &CtrlParam );
		bCtrlCommand = MC_STOP;
		break;
	case MC_GET_CTRL_PARAM:
		CM_WriteBuf( (BYTE*)&CtrlParam, sizeof(MC_CTRL_PARAM) );
		CM_Reply();
		
		bCtrlCommand = MC_STOP;
		break;
	case MC_SET_MOTOR_PARAM:
		CM_ReadBuf( (BYTE*)&MotParam, sizeof(MC_MOTOR_PARAM) );
		CM_Reply();

		SetMotorParam( &MotParam );
		bCtrlCommand = MC_STOP;
		break;
	case MC_GET_MOTOR_PARAM:
		CM_WriteBuf( (BYTE*)&MotParam, sizeof(MC_MOTOR_PARAM) );
		CM_Reply();

		bCtrlCommand = MC_STOP;
		break;
	case MC_CLEAR_DATA:
		CM_Reply();
	
		ENCODER_WriteCounter( 0 );
		bCtrlCommand = MC_STOP;
		break;
	case MC_SET_POS_SENSOR:
		CM_ReadBuf( &ch, 1 );
		switch( ch ){
		case MC_POS_SENS_POTENTIO: cPotMode = TRUE; break;
		case MC_POS_SENS_ENCODER:  cPotMode = FALSE; break;
		}
		CM_Reply();
	
		bCtrlCommand = MC_STOP;
		break;
	case MC_ADJUST_ENC:
		CM_Reply();

		fPosition = kpt* AD_GetData( 0 ) - PotOffset;
		ENCODER_WriteCounter( fPosition / ken_pos);
		bCtrlCommand = MC_STOP;
		break;
	case MC_SIM_MOTOR:
		CM_ReadBuf( (BYTE*)&load, sizeof(float) );
		CM_Reply();

		MC_SetSimMode( load ); // max pulse/sec
		bCtrlCommand = MC_STOP;
		break;
	case MC_TEST_MODE:
		CM_ReadBuf( (BYTE*)&TestMode, 1 );
		CM_Reply();

		bCtrlCommand = MC_STOP;
		break;
	case MC_ECHO:
		CM_ReadBuf( &ch, 1 );
		CM_WriteBuf( &ch, 1 ); 
		CM_Reply();

		bCtrlCommand = MC_STOP;
		break;
	case MC_SET_POSITION:
		CM_ReadBuf( (BYTE*)&Ref, sizeof(MC_REFERENCE) );
		
		fRefVelocity = fabs(kg*Ref.fVelocity);
		fRefTorque   = fabs(ikg*Ref.fTorque);
		bCtrlCommand = MC_SET_POSITION;
		break;
	case MC_SET_VELOCITY:
		CM_ReadBuf( (BYTE*)&Ref, sizeof(MC_REFERENCE) );

		fRefVelocity = kg*Ref.fVelocity;
		fRefTorque = fabs(ikg*Ref.fTorque);
		bCtrlCommand = MC_SET_VELOCITY;
		break;
	case MC_SET_TORQUE:
		CM_ReadBuf( (BYTE*)&Ref.fTorque, sizeof(float) );

		fRefTorque = ikg*Ref.fTorque;
		bCtrlCommand = MC_SET_TORQUE;
		break;
	case MC_SET_DUTY:
		CM_ReadBuf( (BYTE*)&sDuty, sizeof(short) );

		bCtrlCommand = MC_SET_DUTY;
		break;
	case MC_GET_DATA:
		bCtrlCommand = MC_STOP;
		break;

	default :
		bCtrlCommand = MC_STOP;
		ret = FALSE;
		break;
	}
	return( ret );
}

BOOL MC_ControlCommandProc( BYTE command )
{
	BOOL ret = TRUE;
	uiWatchdog = 0;

	switch( command ){
	case MC_SET_POSITION:
		CM_ReadBuf( (BYTE*)&Ref, sizeof(MC_REFERENCE) );
		
		fRefVelocity = fabs(kg*Ref.fVelocity);
		fRefTorque   = fabs(ikg*Ref.fTorque);
		bCtrlCommand = MC_SET_POSITION;
		break;
	case MC_SET_VELOCITY:
		CM_ReadBuf( (BYTE*)&Ref, sizeof(MC_REFERENCE) );

		fRefVelocity = kg*Ref.fVelocity;
		fRefTorque = fabs(ikg*Ref.fTorque);
		bCtrlCommand = MC_SET_VELOCITY;
		break;
	case MC_SET_TORQUE:
		CM_ReadBuf( (BYTE*)&Ref.fTorque, sizeof(float) );

		fRefTorque = ikg*Ref.fTorque;
		bCtrlCommand = MC_SET_TORQUE;
		break;
	case MC_SET_DUTY:
		CM_ReadBuf( (BYTE*)&sDuty, sizeof(short) );

		bCtrlCommand = MC_SET_DUTY;
		break;
	case MC_GET_DATA:
		bCtrlCommand = MC_STOP;
		break;

	default :
		bCtrlCommand = MC_STOP;
		ret = FALSE;
		break;
	}
	return( ret );
}

void MC_PresetData( void )
{
	CM_WriteBuf( (BYTE*)&Data, sizeof( MC_DATA ));
	CM_ChangeTxBuffer();
	CM_WriteBuf( (BYTE*)&Data, sizeof( MC_DATA ));
	CM_ChangeTxBuffer();
}


void MC_Control( void )
{
	int dif;
	
	fTorque = kdt*sDuty;
			
	dif = ENCODER_GetRelativeCount();		
	fVelocity = (float)(kw*dif);// [rad/sec]
			
	if( cPotMode ){
		fPosition = kpt* AD_GetData(0) - PotOffset;
	}else{
		fPosition = ken_pos * ENCODER_Counter.count_32;
	}

	if( Emergency ) bCtrlCommand = MC_EMG_STOP;
			
	if( TestMode ){
		Data.Status.bControl = bCtrlCommand;
		Data.Status.bSensor1 = 0x00;
		Data.Status.bSensor2 = 0x00;
		Data.Status.bSensor3 = 0x00;
	}else{
		if( uiWatchdog > CM_wCtrlTimeout ){
			CM_bCtrlStatus = CM_CTRL_TIMEOUT;
			CM_bStatus = CM_bCtrlStatus;

			bCtrlCommand = MC_EMG_STOP;
			Emergency = TRUE;
		}
		if( (bBoardID_PB == DA_BOARD) && MOTOR_DA_Alarm() ){
			Data.Status.bControl = MC_DA_ALARM;
			Data.Status.bSensor1 = 0x01;
		}
		else{
			Data.Status.bControl = bCtrlCommand;
			Data.Status.bSensor1 = 0;
		}
		Data.Status.bSensor2 = 0x00;
		Data.Status.bSensor3 = 0x00;
	}

	Data.fPosition = fPosition;	
	Data.fVelocity = ikg*fVelocity;
	Data.fTorque = kg*fTorque;	
	Data.lCount = ENCODER_Counter.count_32; 

	CM_WriteBuf( (BYTE*)&Data, sizeof( MC_DATA ));
	CM_ChangeTxBuffer();
		
	switch( bCtrlCommand ){
	case MC_EMG_STOP:
	case MC_STOP:
		MC_Stop();

		sDuty=0;
		uiWatchdog = 0;
		break;
	case MC_SET_POSITION:
		fRefVelocity = kg*PD( Ref.fPosition, fPosition, Ref.fVelocity );
	case MC_SET_VELOCITY:
		sDuty = PI( fRefVelocity, fVelocity, fRefTorque, fTorque );
		if( bBoardID_PB == DA_BOARD ) MOTOR_DA_Drive( sDuty );
		else MOTOR_Drive( 0, sDuty );
		uiWatchdog++;
		break;
	case MC_SET_TORQUE:
		sDuty = fTorqueCtrl( fRefTorque );
		if( bBoardID_PB == DA_BOARD ) MOTOR_DA_Drive( sDuty );
		else MOTOR_Drive( 0, sDuty );
		uiWatchdog++;
		break;
	case MC_SET_DUTY:
		if( bBoardID_PB == DA_BOARD ) MOTOR_DA_Drive( sDuty );
		else MOTOR_Drive( 0, sDuty );
		uiWatchdog++;
		break;
	default:
		MOTOR_Drive( 0, 0 );
		break;
	}
			 
}
