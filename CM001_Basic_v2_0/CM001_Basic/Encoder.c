/***************************************************************
	Encoder.c 
	
								2010.4.19　M.Utsumi@ArcDevice

****************************************************************/

#include"Hardware.h"
#include"qei.h"
#include"encoder.h"

#define TRUE 1
#define FALSE 0


UN_COUNTER ENCODER_Counter;
volatile WORD EncCount;// エンコーダのカウント値：中間受け渡し用
WORD count_o;

float k_pulse;	// pulse/duty
char ENCODER_SimMode = FALSE;


void ENCODER_Open( void )
{
	POSCNT = 0;
	MAXCNT = 0xFFFF;
	k_pulse=1.0f;

	ADPCFGbits.PCFG4 = 1;	// Enanle QEA
	ADPCFGbits.PCFG5 = 1;	// Enanle QEB
	
	OpenQEI(
		//QEI_EXT_CLK & 
		QEI_INDEX_RESET_DISABLE &
		QEI_CLK_PRESCALE_1 &
		QEI_NORMAL_IO &
		QEI_GATED_ACC_DISABLE &
		QEI_INPUTS_NOSWAP &
		QEI_MODE_x4_MATCH &
		//QEI_DIR_SEL_QEB &
		QEI_IDLE_CON,
		POS_CNT_ERR_INT_DISABLE&
		QEI_QE_OUT_DISABLE );

	ENCODER_ClearCounter();

}

int ENCODER_GetRelativeCount(void)
{
	unsigned short count;
	int dif;

	count = EncCount;
				
	if( count_o > 0xEFFF && count <  0x0FFF ){ // Overflow
		ENCODER_Counter._16.count_L = count;
		ENCODER_Counter._16.count_H++;
		dif = (int)(count + (0xFFFF - count_o + 1));
	}else if( count_o < 0x0FFF && count > 0xEFFF   ){ //Underflow
		ENCODER_Counter._16.count_L = count;
		ENCODER_Counter._16.count_H--;
		dif = -(int)(( 0xFFFF - count + 1 ) + count_o );
	}else{
		ENCODER_Counter._16.count_L = count;
		dif = (int)(count - count_o);
	}

	count_o = count;

	return( dif );

}

void ENCODER_WriteCounter( long count )
{
	ENCODER_Counter.count_32 = count;
	POSCNT = ENCODER_Counter._16.count_L; 
	count_o = ENCODER_Counter._16.count_L;
	EncCount = POSCNT;
}

void ENCODER_ClearCounter( void )
{
	ENCODER_WriteCounter( 0 );
}

void ENCODER_SetSimMode( float rate )
{
	k_pulse = rate;
	ENCODER_SimMode = TRUE;
}

void ENCODER_ResetSimMode( void )
{
	ENCODER_SimMode = FALSE;
}

void ENCODER_SimCount( short duty )
{
	POSCNT += (short)(k_pulse * duty );
}
