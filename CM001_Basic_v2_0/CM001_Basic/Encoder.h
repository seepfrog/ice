/*
	Encoder.h 
	
	Rotary encoder counter

							2008.11.20　M.Utsumi@EasLogic
*/

#ifndef ENCODER_H
#define ENCODER_H

typedef union _UN_COUNTER
{
	long count_32;
	struct {
    	unsigned short count_L;
		unsigned short count_H;
	} _16;
}UN_COUNTER;

extern UN_COUNTER ENCODER_Counter;
extern char ENCODER_SimMode;
extern volatile unsigned short EncCount;// エンコーダのカウント値：中間受け渡し用

void ENCODER_Open( void );
int  ENCODER_GetRelativeCount( void );
void ENCODER_WriteCounter( long count );
void ENCODER_ClearCounter( void );
void ENCODER_SetSimMode( float rate );
void ENCODER_ResetSimMode( void );
void ENCODER_SimCount( short duty );

#define ENCODER_Count() 	EncCount = POSCNT		// エンコーダ値を一定時間で保存


#endif
