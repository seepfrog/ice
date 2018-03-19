/**************************************************************
	Hardware.c Hardware settings and utilitys

						2010.4.19 M.Ustumi@Arcdevice

***************************************************************/

#include"Hardware.h"

_FOSC(CSW_FSCM_OFF&XT_PLL8);	// Fosc x 8
_FWDT(WDT_OFF);
_FBORPOR(PBOR_ON & BORV_20 & PWRT_64 & MCLR_EN );
_FGS( CODE_PROT_OFF );

BYTE bBoardID_PA=0;
BYTE bBoardID_PB=0;


void HW_Wait_us( WORD us  )
{
	while( --us ){
		Nop();	Nop();	Nop();
		Nop();	Nop();	Nop();
		Nop();	Nop();	Nop();
		Nop();	Nop();	Nop();
		Nop();	Nop();	Nop();
	};
}

void HW_Wait_ms( WORD ms )
{
	while(ms--) HW_Wait_us( 1000 );
}

void HW_Blink( WORD t )
{
	while(t--){
		LED=~LED;
		HW_Wait_ms(125);
		LED=~LED;
		HW_Wait_ms(125);
	}
	HW_Wait_ms(250);
}

void HW_ZeroMemory( void* pBuf, WORD wSize )
{
	BYTE* ptr;
	ptr = (BYTE*)pBuf;
	while( wSize-- ) *ptr++=0;
}

void NVM_Update( void )
{
	asm("push SR ");
	asm("mov #0x00E0, W0 ");
	asm("ior	SR ");
	asm("mov #0x55, W0 ");
	asm("mov W0, NVMKEY ");
	asm("mov #0xAA, W0 ");
	asm("mov W0, NVMKEY	");
	NVMCONbits.WR = 1;
	asm("nop");
	asm("nop");
	while( NVMCONbits.WR );
	asm("pop SR");
}

void NVM_Write_w( WORD wPage, WORD wOffset, WORD* pwBuf )
{
	asm( "mov W0,TBLPAG " );
	asm( "tblwtl [W2], [W1] " );	
}

void NVM_Read_w( WORD wPage, WORD wOffset, WORD* pwBuf )
{ 
	asm( "mov W0,TBLPAG " );
	asm( "tblrdl [W1], [W2] " );
}


// 32命令（64words）単位で消去
void FLASH_Erace_32dw( WORD wAddr )
{
	NVMADR  = wAddr;	// High & Low
	NVMADRU = 0;		// Upper
	NVMCON  = 0x4041;	// Set erace command
	NVM_Update();	
}

// 4命令（8words）単位で書き込み
void FLASH_Write_4dw( WORD wAddr, WORD* pwCode )
{
	asm("clr w2");
	asm("mov w2, TBLPAG ");
	asm("tblwtl [W1++], [W0] ");	
	asm("tblwth [W1++], [W0++] ");
	asm("tblwtl [W1++], [W0] ");	
	asm("tblwth [W1++], [W0++]");
	asm("tblwtl [W1++], [W0] ");	
	asm("tblwth [W1++], [W0++] ");
	asm("tblwtl [W1++], [W0] ");	
	asm("tblwth [W1],   [W0] ");
	NVMCON = 0x4001;
	NVM_Update();	
}

// 1命令（2words）単位で読み込み
void FLASH_Read_dw( WORD wAddr, WORD* pwCode )
{
	asm("clr w2");
	asm("mov w2, TBLPAG ");
	asm("tblrdl [W0], [W1++] ");
	asm("tblrdh [W0], [W1] ");
}


void EEP_Read( WORD wPage, WORD wOffset, WORD* pwBuf, WORD wSize )
{
	while(wSize--){	// WORD access
		NVM_Read_w( wPage, wOffset,	pwBuf++ );
		wOffset+=2;
	}
}

void EEP_Write( WORD wPage, WORD wOffset, WORD* pwBuf, WORD wSize )
{
	while(wSize--){
		// Erace EEP 1 word
		NVMADRU = wPage;
		NVMADR  = wOffset;
		NVMCON  = 0x4044;
		NVM_Update();

		// Write EEP 1 word
		NVM_Write_w( wPage, wOffset, pwBuf++);
		NVMCON = 0x4004;
		NVM_Update();	

		wOffset+=2;
	}

}
