/****************************************************************************************

		Measurement.h Fuctions for measurement
		
													2010.4.19 M.Utsumi@ArcDevice 

*****************************************************************************************/

#ifndef _MEASUREMENT_H
#define _MEASUREMENT_H

//#define  DEBUG

#ifdef DEBUG
#define dbg_printf printf
#else
#define dbg_printf dumy_printf
 void dumy_printf( const char*, ... );
#endif


extern double MES_Time_ini;		/* Initial  time [s]*/
extern double MES_Time_prv;		/* Previous time [s]*/
extern double MES_Time_cur;		/* Current  time [s]*/
extern double MES_Time_int;		/* Interval time [s]*/

void MES_StartTimeCount( void );
void MES_GetTimeCount( void  );
void MES_WaitNextTime( double time_n );
BOOL MES_Save( char* pFileName, char* pCaption, void* pData, int item, int size );
void MES_PrintData( void* data, int size );

#endif

