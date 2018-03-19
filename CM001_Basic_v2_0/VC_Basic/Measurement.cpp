/****************************************************************************************

		Measurement.cpp Fuctions for measurement
		
													2010.4.19 M.Utsumi@ArcDevice 

*****************************************************************************************/


#include<windows.h>
#include<stdio.h>
#include<conio.h>

void dumy_printf( const char*, ... )
{

}

double MES_TimeScale;
double MES_Time_ini;		/* Initial  time */
double MES_Time_prv;		/* Previous time */
double MES_Time_cur;		/* Current  time */
double MES_Time_int;		/* Interval time */

void MES_StartTimeCount( void )
{
	LONGLONG freq;
	LONGLONG time_count;
	
	// 起動時からのＣＰＵクロックのカウント値から現在の時刻を計算する。
	 
	// ＣＰＵのクロック周波数を取得する。
	if( !QueryPerformanceFrequency( (LARGE_INTEGER*)&freq ) ){
		printf("err");
		return;
	}
	MES_TimeScale = 1.0 / freq;	// 周波数から1クロックの周期を計算する。
	
	QueryPerformanceCounter( (LARGE_INTEGER*)&time_count ); // カウント値を取得する。
	MES_Time_ini = time_count * MES_TimeScale;						// 開始時間として記録する。
	MES_Time_cur = 0;
	MES_Time_prv = 0;
}

void MES_GetTimeCount( void  )
{
	LONGLONG time_count;

	QueryPerformanceCounter( (LARGE_INTEGER*)&time_count ); // カウント値を取得する。
	
	MES_Time_cur = time_count * MES_TimeScale - MES_Time_ini; //　StartTimeCountからの現在の時間（経過時間）を計算する
	MES_Time_int = MES_Time_cur - MES_Time_prv;	// 	前回からの経過時間を計測する。	
	MES_Time_prv = MES_Time_cur;			// 	現在の時間を保存する。
}

void MES_WaitNextTime( double time_n )
{
	LONGLONG time_count;

	do{ // 	time_n秒になるまで待つ。
		QueryPerformanceCounter( (LARGE_INTEGER*)&time_count );
		
		MES_Time_cur = time_count * MES_TimeScale - MES_Time_ini;
	}while( MES_Time_cur < time_n );
	
	MES_Time_int = MES_Time_cur - MES_Time_prv;
	MES_Time_prv = MES_Time_cur;
}


BOOL MES_Save( char* pFileName, char* pCaption, void* pData, int item, int size )
{
	int i;
	FILE* fp;
	float* pfData;

	if( (fp = fopen( pFileName, "w+")) == NULL ){
		printf( "ファイルが開けません");
		return( FALSE );
	}

	fprintf( fp, "%s\n",pCaption );

	pfData = (float*)pData;

	for(i=0;i<size*item;i++){
		fprintf( fp,"%f",*pfData++ );
		if( (i+1)%item==0 ) fprintf( fp,"\n");
		else fprintf( fp,"," );
	}

	fclose( fp );
	return( TRUE );
};


void MES_PrintData( void* data, int size )
{
	int i;
	unsigned char* buf;

	buf = (unsigned char*)data;

	i=0;
	while(size--){
        printf( "%02x ", *buf++ );
		if(++i==16){
			printf("\r\n");
			i=0;
		}
	}
	printf("\r\n");
}
