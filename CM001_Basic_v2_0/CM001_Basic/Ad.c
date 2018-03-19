/*************************************************************
	AD.c 
	
						2010.4.19	 M.Utsumi@ArcDevice

**************************************************************/

#include"Hardware.h"
#include"ADC10.h"

void AD_Open( void )
{		
	ADCON1 = 0;
	ADCON1bits.ADON   = 0;
	ADCON1bits.ADSIDL = 0;		// Continue when Device is idle
	ADCON1bits.FORM	= 0;		// Data form is integer
	ADCON1bits.SSRC	= 0x07; 	// Auto convert (use internal counter)
	ADCON1bits.SIMSAM	= 0;		// Not use
	ADCON1bits.ASAM	= 0;		// Start sampling when SAMP bit is set
	
	ADCON2 = 0;
	ADCON2bits.VCFG	= 0;		// AVdd- AVss
	ADCON2bits.CSCNA	= 0;		// Not scan
	ADCON2bits.CHPS	= 0;		// Convert CH0
	ADCON2bits.SMPI	= 0;		// Interruput when conversion is complete
	ADCON2bits.BUFM 	= 0;		// 16WORD buffer
	ADCON2bits.ALTS	= 0;		// Allways use MUXA

	ADCON3 = 0;
	ADCON3bits.SAMC	= 31;	// Sampletime 31 Tad	
	ADCON3bits.ADRC	= 0;		// Clock source is 
	ADCON3bits.ADCS	= 15;	// Conversion time 15+1=16 Tcy
	
	ADCHS = 0;
	ADCHSbits.CH123NB = 0; 	// MUXB CH123 -input is VREF-
	ADCHSbits.CH123SB = 0; 	// MUXB CH123 +input is AN0,AN1,AN2
	ADCHSbits.CH0NB	= 0;		// MUXB CH0   -input is VREF-
	ADCHSbits.CH0SB 	= 0; 	// MUXB CH0   +input is AN02
	ADCHSbits.CH123NA = 0; 	// MUXA CH123 -input is VREF-
	ADCHSbits.CH123SA = 0; 	// MUXA CH123 +input is AN0,AN1,AN2
	ADCHSbits.CH0NA	= 0;		// MUXA CH0   -input is VREF-
	ADCHSbits.CH0SA 	= 0; 	// MUXA CH0   +input is AN02

	ADPCFG = 0xFFFF;			// Set all ports digital
	ADPCFGbits.PCFG0 = 0;		// Set AN0 as analog input
	ADPCFGbits.PCFG1 = 0;		// Set AN1 as analog input
	ADPCFGbits.PCFG3 = 0;		// Set AN3 as analog input
	ADPCFGbits.PCFG4 = 0;		// Set AN4 as analog input
	ADPCFGbits.PCFG5 = 0;		// Set AN5 as analog input
	ADPCFGbits.PCFG6 = 0;		// Set AN6 as analog input
	ADPCFGbits.PCFG7 = 0;		// Set AN7 as analog input
	ADPCFGbits.PCFG8 = 0;		// Set AN8 as analog input
	
	ADCSSL = 0;

	ADCON1bits.ADON = 1;
}

unsigned short AD_GetData( char ch )
{
	ADCON1bits.SAMP = 1;

	if(ch>1) ch++;	//AN2 is not use
	
	ADCHSbits.CH0SA 	= ch; 	// Set input ch

	while(!ADCON1bits.DONE);
	return( ADCBUF0 );
}
