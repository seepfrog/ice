/*****************************************************************************************************

	Ex.cpp

	Control Module SC02 & CM001 Program
	 
	
													2010.4.23 M.Utsumi@Arcdevice

******************************************************************************************************/

#include<windows.h>
#include<stdio.h>
#include<conio.h>
#include<string.h>
#include<math.h>
#include"SC02.h"
#include"CM001.h"
#include"Measurement.h"
#include <GL/freeglut.h>
#include <vector>
#include <random>


CM_PARAM Param ={
	//   dt[s], fCtrlTimeout fComTimeout 
		0.001f, 1.00f, 1.000f,
};


SC02 ScModule;
CM001 Module00, Module01, Module02;

int iVal = 0; //モータの回転
int solVal = 0; //ソレノイド
int NewEnc = 0;
int OldEnc = 0;


#define WIDTH 640
#define HEIGHT 480

//氷の大きさ
float scale = 0.0f;
//色
GLfloat Blue[] = { 0.0, 0.0, 1.0, 1.0 };  //青
GLfloat Green[] = { 0.0, 1.0, 0.0, 1.0 }; //緑
GLfloat Red[] = { 1.0, 0.0, 0.0, 1.0 };   //赤
GLfloat White[] = { 1.0, 1.0, 1.0, 1.0 }; //白
GLfloat vividBlue[] = { 0.8, 1.0, 1.0, 1.0 };  //青

//ライトの位置
GLfloat lightpos[] = { 200.0, 150.0, -200.0, 0.0 };
//ライトの色
GLfloat ambient[] = { 0.2, 0.2, 0.2, 1.0 };

static GLfloat lightDiffuse[3] = { 1.0, 1.0, 1.0 };

GLUquadricObj* cylinder = gluNewQuadric();
GLUquadricObj* disk = gluNewQuadric();



BOOL TestSC( void )
{

//	if( !ScModule.SetID( 1 ) )	return( FALSE );
	if( !ScModule.GetID() ) return( FALSE );
	printf("ID = %d\r\n", ScModule.bID );
/*
	if( !ScModule.GetVersion() ) return( FALSE );
//	printf("%x\r\n", ScModule.bVersion );
//	printf("Status %d \r\n", ScModule.bStatus );
/**/
/*
	printf("Hit any key\r\n");
	getch();
	if( !ScModule.Blink(3) ) return( FALSE );

	printf("Status %d \r\n", ScModule.bStatus );
/**/
	return( TRUE );
}

BOOL TestCM( void )
{
	Module00.GetVersion();
	printf( "CM001 Ver.%d.%d\r\n", 0x0F & Module00.bVersion >>4, 0x0F & Module00.bVersion );

	if( !Module00.Ping() ) return( FALSE );
	printf("Ping OK\r\n", ScModule.bStatus, Module00.bStatus );

	_getch();
	Module00.Blink( 2 );
	printf("status SC02 %d CM001 %d\r\n", ScModule.bStatus, Module00.bStatus );

	return( TRUE );
}

void SetData( CM001 *pModule, int iVal , int solVal)
{

	// ポートAがPIOの場合のデータ設定
	if( pModule->bBoardID_PA == CM001_PIO_BOARD ){
		// バイト単位で設定する場合（PIOAは8bitまで有効）
		//pModule->PioBoardA.Word.wData = ~pModule->PioBoardA.Word.wData;
		pModule->PioBoardA.Word.wDir = 0xFFFF;
		// ビット単位で設定する場合
		pModule->PioBoardA.Bit.P1 = ~pModule->PioBoardA.Bit.P1;
		pModule->PioBoardA.Bit.DIR1 = 0;
	}

	// ポートBのデータ設定
	switch(  pModule->bBoardID_PB ){
	case CM001_PWM_BOARD:
		pModule->PwmBoard.sPwm1 = iVal;
		pModule->PwmBoard.sPwm2 = solVal;
		printf("sPwm %d %d ", pModule->PwmBoard.sPwm1, pModule->PwmBoard.sPwm2 );
		break; 
	case CM001_DA_BOARD:
		pModule->DaBoard.sDA = iVal;
		printf("da %d ", pModule->DaBoard.sDA );
		break;
	case CM001_PIO_BOARD:
		// バイト単位で設定する場合（PIOBは10bitまで有効）
		//Module00.PioBoardB.Word.wData = ~Module00.PioBoardB.Word.wData;
		pModule->PioBoardB.Word.wDir = 0xFFFF;
		// ビット単位で設定する場合
		pModule->PioBoardB.Bit.P9 = ~pModule->PioBoardB.Bit.P9;
		pModule->PioBoardB.Bit.DIR9 = 0;
		break;	
	}
}



void IndecateData( CM001 *pModule )
{
	switch( pModule->bBoardID_PA){
	case CM001_ENC_BOARD:
		printf("enc %x ad %d ", pModule->EncBoard.lEncoder, pModule->EncBoard.wSensor[0] );
		OldEnc = NewEnc;
		NewEnc = pModule->EncBoard.lEncoder;
		
		break;
	case CM001_SENSOR_BOARD:
		printf("\r\nsens %d  %d  %d  %d  %d  %d  %d  %d",
			pModule->SensorBoard.wSensor[0], pModule->SensorBoard.wSensor[1],
			pModule->SensorBoard.wSensor[2], pModule->SensorBoard.wSensor[3],
			pModule->SensorBoard.wSensor[4], pModule->SensorBoard.wSensor[5],
			pModule->SensorBoard.wSensor[6], pModule->SensorBoard.wSensor[7]
		);
		break;
	case CM001_PIO_BOARD:
		printf("pioA %04x ", pModule->PioBoardA.Word.wData );
		break;
	}
	if( pModule->bBoardID_PB == CM001_PIO_BOARD ){
		printf("pioB %04x ", pModule->PioBoardB.Word.wData );
	}
	printf("\r\n");

}


std::random_device rnd;
std::mt19937 mt(rnd());
std::uniform_int_distribution<> rand50(0, 20);

int chage = 0;

// 基本的なデータの送受信．入出力データはクラスCM001のメンバ変数を使用
void Exchange( void )
{
	
	BOOL loop=TRUE;
	
	MES_StartTimeCount();

	// データの初期値
	//iVal= 0;

	//while(loop){
		printf("Time %2d:%02d:%02d dt %5.3fms ",(int)(MES_Time_cur/60.0f),(int)MES_Time_cur%60,(int)(100.0f*MES_Time_cur)%100, MES_Time_int * 1000.0f);

		/*

		if(_kbhit()){


			switch(_getch()){
			case '8': iVal += 100;  if (iVal > 1023){ iVal = 1023; break; }
					   //if (solVal > 1023){ solVal = 1023; break; }
			case '2': iVal -= 50, solVal = -100; break;
			case '5': iVal =  0 , solVal = 0; break;
			case 27: loop = FALSE; break;
			}
		}/**/

		// モータの回転制御
		if (NewEnc > OldEnc + 30 ){
			//chage = iVal;
			iVal += - 1 * (NewEnc - OldEnc) * 0.5;
			chage += 1;
			if (chage >= rand50(mt) || chage > 20){
				iVal = 0;
				chage = 0;
			}
			/*if (NewEnc % 31 == 0){
				solVal = 500;
			}*/
		}
		if (OldEnc >= NewEnc){
			iVal = 0;
			//solVal = -10;
		}
		if(scale +1> 40){  //氷を削りきったのでモーターを停止
			iVal = 0;
			//solVal = 0;
		}

		//モータの回転数制限
		if (iVal > 512){ 
			iVal = 0; 
		}/*
		if (solVal > 1023){
			solVal = 1023;
		}*/


		SetData( &Module02, iVal , solVal);
		//SetData( &Module01, iVal );

		// データ送受信
		MES_GetTimeCount();
		//if(!Module00.Exchange(TRUE) ) loop =FALSE;
		//if(!Module01.Exchange(TRUE) ) loop =FALSE;
		if(!Module02.Exchange(TRUE) ) loop =FALSE;
		MES_GetTimeCount();

		//IndecateData( &Module00 );
		//IndecateData( &Module01 );
		IndecateData( &Module02 );

	    Sleep(10);
	//}
	
	SetData( &Module02, 0 , 0);
	//SetData( &Module01, 0 );
	

	//if(!Module00.Exchange(TRUE) ) loop =FALSE;
	//if(!Module01.Exchange(TRUE) ) loop =FALSE;
	if(!Module02.Exchange(TRUE) ) loop =FALSE;
}

void PrintCM001Status( CM001* cm )
{
	printf("status ScModule %d CM001 %d\r\n", cm->pParentModule->bStatus, cm->bStatus );
}

//直方体
void cuboid(float width, float height, float depth)
{
	glBegin(GL_QUADS);
	//前
	glNormal3f(0.0, 0.0, -1.0);
	glVertex3f(width / 2, height / 2, depth / 2);
	glVertex3f(-width / 2, height / 2, depth / 2);
	glVertex3f(-width / 2, -height / 2, depth / 2);
	glVertex3f(width / 2, -height / 2, depth / 2);

	//左
	glNormal3f(1.0, 0.0, 0.0);
	glVertex3f(width / 2, height / 2, depth / 2);
	glVertex3f(width / 2, height / 2, -depth / 2);
	glVertex3f(width / 2, -height / 2, -depth / 2);
	glVertex3f(width / 2, -height / 2, depth / 2);

	//右
	glNormal3f(-1.0, 0.0, 0.0);
	glVertex3f(-width / 2, height / 2, -depth / 2);
	glVertex3f(-width / 2, height / 2, depth / 2);
	glVertex3f(-width / 2, -height / 2, depth / 2);
	glVertex3f(-width / 2, -height / 2, -depth / 2);

	//後
	glNormal3f(0.0, 0.0, 1.0);
	glVertex3f(width / 2, height / 2, -depth / 2);
	glVertex3f(-width / 2, height / 2, -depth / 2);
	glVertex3f(-width / 2, -height / 2, -depth / 2);
	glVertex3f(width / 2, -height / 2, -depth / 2);

	//上
	glNormal3f(0.0, 1.0, 0.0);
	glVertex3f(width / 2, height / 2, depth / 2);
	glVertex3f(-width / 2, height / 2, depth / 2);
	glVertex3f(-width / 2, height / 2, -depth / 2);
	glVertex3f(width / 2, height / 2, -depth / 2);

	//下
	glNormal3f(0.0, -1.0, 0.0);
	glVertex3f(width / 2, -height / 2, depth / 2);
	glVertex3f(-width / 2, -height / 2, depth / 2);
	glVertex3f(-width / 2, -height / 2, -depth / 2);
	glVertex3f(width / 2, -height / 2, -depth / 2);
	glEnd();
}

//器
void bowl(){

	//マテリアルの設定
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Blue);

	glTranslatef(5.0f, 20.0f, 0.0f);

	//器外周部
	glPolygonMode(GL_BACK, GL_FILL);
	glTranslatef(0.0f, -60.0f, 0.0f);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluCylinder(cylinder, 50, 40, 40, 100, 100);
	//glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
	glFrontFace(GL_CW);

	gluCylinder(cylinder, 50, 40, 40, 100, 100);
	//glRotatef(-180.0f, 0.0f, 0.0f, 1.0f);

	//器の底
	glTranslatef(0.0f, 0.0f, 30.0f);
	glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
    gluDisk(disk, 0, 40, 100, 100);
	glRotatef(-180.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, 0.0f, -30.0f);

	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, 60.0f, 0.0f);

	glTranslatef(-5.0f, -20.0f, 0.0f);

}

int HandleAngle = 0; //ハンドルの回転角度
double HandleHeight = 0; //ハンドルの高さ

void IceShaver(){
	//マテリアル設定
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, vividBlue);
	
	glTranslatef(0.0f, 100.0f, 0.0f)
		;
	//かき氷器本体
	//glutSolidCube(100.0f);
	glTranslatef(0.0f, 40.0f, 0.0f);
	cuboid(100, 10, 100);//上の
	glTranslatef(0.0f, -40.0f, 0.0f);

	glTranslatef(0.0f, -20.0f, 0.0f);
	cuboid(100, 10, 100);//下の
	glTranslatef(0.0f, 20.0f, 0.0f);

	glTranslatef(-50.0f, -75.0f, 0.0f);
	cuboid(10, 240, 100);//後ろの
	glTranslatef(50.0f, 75.0f, 0.0f);

	glTranslatef(0.0f, -190.0f, 0.0f);
	cuboid(100, 10, 100);//台
	glTranslatef(0.0f, 190.0f, 0.0f);


	//ハンドルの回転
	glRotatef(HandleAngle, 0.0f, 1.0f, 0.0f);
	//ハンドルの高さ
	glTranslatef(0.0f, 70.0f - HandleHeight*1.5, 0.0f);
	//ハンドルの外側の輪っか
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	glutSolidTorus(3, 40, 10, 10);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	//ハンドルのクロスになってるやつ
	glTranslatef(0.0f, 0.0f, -40.0f);
	glutSolidCylinder(2, 80, 30, 30); 
	glTranslatef(0.0f, 0.0f, 40.0f);
	//ハンドルのクロスになってるやつ
	glTranslatef(-40.0f, 0.0f, 0.0f);
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	glutSolidCylinder(2, 80, 30, 30);
	glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
	glTranslatef(40.0f, 0.0f, 0.0f);
	//ハンドルの持ち手
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, 38.0f, 0.0f);
	glutSolidCylinder(3, 30, 30, 30);
	glTranslatef(0.0f, -38.0f, 0.0f);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	//真ん中の棒
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	glutSolidCylinder(3, 60, 30, 30);
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	//氷を抑えるやつ
	glTranslatef(0.0f, -60.0f, 0.0f);
	cuboid(60, 5, 60);
	glTranslatef(0.0f, 60.0f, 0.0f);

	glTranslatef(0.0f, -70.0f + HandleHeight*1.5, 0.0f);
	glRotatef(-HandleAngle, 0.0f, 1.0f, 0.0f);

	glTranslatef(0.0f, -100.0f, 0.0f);
}

void Ice(){

	//glDepthMask(GL_FALSE);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, White);

	//氷
	glRotatef(HandleAngle, 0.0f, 1.0f, 0.0f);
	glTranslatef(0.0f, 90.0f - HandleHeight * 1.5+ (scale*1)/2, 0.0f);
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	cuboid(50 , 50 , 41 - scale*1 ); //scaleで氷の大きさ
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, -90.0f + HandleHeight * 1.5 - (scale*1)/2, 0.0f);
	glRotatef(-HandleAngle, 0.0f, 1.0f, 0.0f);

	//削られた氷
	glTranslatef(0.0f, -70.0f, 0.0f);
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	glutSolidCone(scale, scale, 100, 100); //scaleで氷の大きさ
	glTranslatef(0.0f, 70.0f, 0.0f);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	//glDisable(GL_BLEND);
	//glDepthMask(GL_TRUE);
}

//光と視点の設定
void myLight(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.6f, 0.6f, 0.6f, 0.6f);//背景色

	glViewport(0, 0, WIDTH, HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//視野角,アスペクト比(ウィンドウの幅/高さ),描画する範囲(最も近い距離,最も遠い距離)
	gluPerspective(30.0, (double)WIDTH / (double)HEIGHT, 1.0, 1000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//視点の設定
	gluLookAt(450.0, 280.0, -470.0, //カメラの座標
		0.0, 0.0, 0.0, // 注視点の座標
		0.0, 1.0, 0.0); // 画面の上方向を指すベクトル

	//ライトの設定
	glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);

	glEnable(GL_LIGHT0);
}

void display(void)
{
	//glDisable(GL_CULL_FACE);

	myLight();
	glTranslatef(0.0f, -30.0f, 0.0f);
	//かき氷器
	IceShaver();

	//器
	bowl();

	//氷
	Ice();
	
	/*
	//回転
	//glRotatef(anglex, 1.0f, 0.0f, 0.0f);//X軸を回転
	//台
	cylinder(30.0, 20.0, 10);

	//蓋
	glMaterialfv(GL_FRONT, GL_DIFFUSE, Red);
	glTranslatef(0.0f, 16.0f, 0.0f);
	glutSolidSphere(28.5, 16, 16);

	//軸
	glTranslatef(0.0f, 0.0f, 0.0f);
	cylinder(5.0, 42.0, 10);

	//取っ手
	glTranslatef(0.0f, 37.0f, -20.0f);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);//X軸を回転
	cylinder(4.6, 20.0, 10);

	//握り
	glTranslatef(0.0f, -18.0f, -7.0f);
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);//X軸を回転
	cylinder(4.6, 10.0, 10);

	*/

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	glutSwapBuffers();
}

void idle(void)
{
	 
	for (int i = 0; i < 5; i++){
		Exchange();
	}
	HandleAngle = NewEnc * 5 * 0.01; //ハンドルの回転角度

	//ハンドルの高さ
	if (70 - HandleHeight > 60 && scale <= 40 && NewEnc > OldEnc){
		HandleHeight += 0.1;
	}
	else if (70 + HandleHeight < 0 && NewEnc < OldEnc){
		HandleHeight -= 5;
	}

	if (scale <= 40 && NewEnc > OldEnc){ //削った氷の大きさが器の大きさ以下　かつ　エンコーダの値が増えている時
		scale += (NewEnc - OldEnc) * 0.003f;                   //氷を大きくする
	}
	/*else if (scale > 40){  //氷を削りきったのでモーターを停止
		iVal = 0;
		solVal = 0;
		SetData(&Module02, iVal, solVal); 
		//SetData(&Module01, iVal);
	}*/
	Sleep(10);
	glutPostRedisplay();
}

int main( int argc, char* argv[] )
{

	/*//描画初期化
	glutInit(&argc, argv);//OpenGLの初期化
	glutInitWindowPosition(100, 100);//ウィンドウの表示位置
	glutInitWindowSize(WIDTH, HEIGHT);//ウィンドウの大きさ
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);//ディスプレイ設定
	glutCreateWindow("描画"); //ウィンドウ生成

	glutDisplayFunc(display);
	glutIdleFunc(idle);

	glutMainLoop();//*/

// 初期化
	printf("Open ScModule ");
	if( ( ScModule.Open( CM_MASTER_ID ))==NULL ){
		printf( "Error %s\r\n", ScModule.Error );
		return( 0 );
	}
	printf("OK\r\n");
//	TestSC();
	/*
	printf("Open Module00 ");
	if( !Module00.Open( &ScModule, 0, 0 )){
		printf( "%s", Module00.Error );
		return( 0 );
	}
	printf("OK\r\n");
	PrintCM001Status( &Module00  );
	//TestCM();

	printf("Open Module01 ");
	if( !Module01.Open( &ScModule, 0, 1 )){
		printf( "%s", Module01.Error );
		return( 0 );
	}
	printf("OK\r\n");
	PrintCM001Status( &Module01  );
	*/
	printf("Open Module02 ");
	if( !Module02.Open( &ScModule, 0, 0xff )){
		printf( "%s", Module02.Error );
		return( 0 );
	}
	printf("OK\r\n");
	PrintCM001Status( &Module02  );
	
	//描画初期化
	glutInit(&argc, argv);//OpenGLの初期化
	glutInitWindowPosition(100, 100);//ウィンドウの表示位置
	glutInitWindowSize(WIDTH, HEIGHT);//ウィンドウの大きさ
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);//ディスプレイ設定
	glutCreateWindow("描画"); //ウィンドウ生成

	glutDisplayFunc(display);
	glutIdleFunc(idle);

	glutMainLoop();
	
/**/
// 基本データの送受信（Basic）
	//Exchange();
/*
	
	printf("Close Module00 ");
	if( !Module00.Close() ){
		printf("Error:%s",Module00.Error );
	}
	printf("OK\r\n");

	printf("Close Module01 ");
	if( !Module01.Close() ){
		printf("Error:%s",Module01.Error );
	}
	printf("OK\r\n");
*/
	printf("Close Module02 ");
	if( !Module02.Close() ){
		printf("Error:%s",Module02.Error );
	}
	printf("OK\r\n");


	printf("ScModule Close ");
	if(!ScModule.Close()){
		printf("Error:%s",ScModule.Error );
	}
	printf("OK\r\n");

	return(1);
}

