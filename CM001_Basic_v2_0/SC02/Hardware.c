/*
	Hardware.c

	ハード固有のセッティング等
						2008.11.20 M.Ustumi@Easlogic

*/

#include"Hardware.h"

_FOSC(CSW_FSCM_OFF&XT_PLL8);	// Fosc x 8
_FWDT(WDT_OFF);
_FBORPOR(PBOR_ON & BORV_20 & PWRT_64 & MCLR_EN );
_FGS( CODE_PROT_OFF );


void wait_us( int us  )
{
	while( --us ){
		Nop();
		Nop();
		Nop();
		Nop();
		Nop();
		Nop();
		Nop();
		Nop();
		Nop();
		Nop();
		Nop();
		Nop();
		Nop();
		Nop();
		Nop();
	};
}

void wait_ms( int ms )
{
	while(ms--) wait_us( 1000 );
}

void blink( int t )
{
	while(t--){
		LED=~LED;
		wait_ms(250);
		LED=~LED;
		wait_ms(250);
	}
	wait_ms(250);
}

void ZeroMemory( void* buf, int size )
{
	unsigned char* ptr;
	ptr = (unsigned char*)buf;
	while( size-- ) *ptr++=0;
}

int _EEDATA(2) EEP_Data[32];
	
void EEP_ReadWord( WORD page, WORD offset, WORD* data )
{
	asm( "PUSH	TBLPAG");
	asm( "MOV		W0, TBLPAG ");
	asm( "TBLRDL	[W1], [W2]");
	asm( "POP  	TBLPAG");
}

void EEP_EraceWord( WORD page, WORD offset )
{
	asm( "PUSH	TBLPAG");
	asm( "MOV		W0, NVMADRU ");
	asm( "MOV		W1, NVMADR  ");
	NVMCON  = 0x4044;
	asm( "PUSH	SR");
	asm( "MOV		#0x00E0, W0" );
	asm( "IOR		SR" );
	asm( "MOV		#0x55, W0 " );
	asm( "MOV		W0, NVMKEY " );
	asm( "MOV		#0xAA, W0 " );
	asm( "MOV		W0, NVMKEY " );
	asm( "BSET	NVMCON, #15" );
	asm( "NOP" );
	asm( "NOP" );
	while( NVMCONbits.WR );
	asm( "POP  SR");
	asm( "POP  TBLPAG");
}

void EEP_WriteWord( WORD data, WORD page, WORD offset  )
{
	asm( "PUSH	TBLPAG");
	asm( "MOV		W1, TBLPAG");
	asm( "TBLWTL	W0, [W2]");
	asm( "MOV		W1, NVMADRU ");
	asm( "MOV		W2, NVMADR  ");
	NVMCON = 0x4004;
	asm( "PUSH	SR");
	asm( "MOV		#0x00E0, W0" );
	asm( "IOR		SR" );
	asm( "MOV		#0x55, W0 " );
	asm( "MOV		W0, NVMKEY " );
	asm( "MOV		#0xAA, W0 " );
	asm( "MOV		W0, NVMKEY " );
	asm( "BSET	NVMCON, #15" );
	asm( "NOP" );
	asm( "NOP" );
	while( NVMCONbits.WR );
	asm( "POP  SR");
	asm( "POP  TBLPAG");

}

void EEP_Read( WORD* buf, int size )
{
	int i;

	for(i=0;i<size*2;i+=2 ){	// WORD access
		EEP_ReadWord( __builtin_tblpage(&EEP_Data[0]), 
					__builtin_tbloffset(&EEP_Data[0])+i, buf++ );
	}
}

void EEP_Write( WORD* buf, int size )
{
	int i;
	for(i=0;i<size*2;i+=2){
		EEP_EraceWord( __builtin_tblpage(&EEP_Data[0]), 
					__builtin_tbloffset(&EEP_Data[0])+i );
	}
	for(i=0;i<size*2;i+=2){
		EEP_WriteWord( *buf++,__builtin_tblpage(&EEP_Data[0]), 
					__builtin_tbloffset(&EEP_Data[0])+i );
	}
}
