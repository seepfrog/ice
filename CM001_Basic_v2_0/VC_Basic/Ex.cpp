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

int iVal = 0; //���[�^�̉�]
int solVal = 0; //�\���m�C�h
int NewEnc = 0;
int OldEnc = 0;


#define WIDTH 640
#define HEIGHT 480

//�X�̑傫��
float scale = 0.0f;
//�F
GLfloat Blue[] = { 0.0, 0.0, 1.0, 1.0 };  //��
GLfloat Green[] = { 0.0, 1.0, 0.0, 1.0 }; //��
GLfloat Red[] = { 1.0, 0.0, 0.0, 1.0 };   //��
GLfloat White[] = { 1.0, 1.0, 1.0, 1.0 }; //��
GLfloat vividBlue[] = { 0.8, 1.0, 1.0, 1.0 };  //��

//���C�g�̈ʒu
GLfloat lightpos[] = { 200.0, 150.0, -200.0, 0.0 };
//���C�g�̐F
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

	// �|�[�gA��PIO�̏ꍇ�̃f�[�^�ݒ�
	if( pModule->bBoardID_PA == CM001_PIO_BOARD ){
		// �o�C�g�P�ʂŐݒ肷��ꍇ�iPIOA��8bit�܂ŗL���j
		//pModule->PioBoardA.Word.wData = ~pModule->PioBoardA.Word.wData;
		pModule->PioBoardA.Word.wDir = 0xFFFF;
		// �r�b�g�P�ʂŐݒ肷��ꍇ
		pModule->PioBoardA.Bit.P1 = ~pModule->PioBoardA.Bit.P1;
		pModule->PioBoardA.Bit.DIR1 = 0;
	}

	// �|�[�gB�̃f�[�^�ݒ�
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
		// �o�C�g�P�ʂŐݒ肷��ꍇ�iPIOB��10bit�܂ŗL���j
		//Module00.PioBoardB.Word.wData = ~Module00.PioBoardB.Word.wData;
		pModule->PioBoardB.Word.wDir = 0xFFFF;
		// �r�b�g�P�ʂŐݒ肷��ꍇ
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

// ��{�I�ȃf�[�^�̑���M�D���o�̓f�[�^�̓N���XCM001�̃����o�ϐ����g�p
void Exchange( void )
{
	
	BOOL loop=TRUE;
	
	MES_StartTimeCount();

	// �f�[�^�̏����l
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

		// ���[�^�̉�]����
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
		if(scale +1> 40){  //�X����肫�����̂Ń��[�^�[���~
			iVal = 0;
			//solVal = 0;
		}

		//���[�^�̉�]������
		if (iVal > 512){ 
			iVal = 0; 
		}/*
		if (solVal > 1023){
			solVal = 1023;
		}*/


		SetData( &Module02, iVal , solVal);
		//SetData( &Module01, iVal );

		// �f�[�^����M
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

//������
void cuboid(float width, float height, float depth)
{
	glBegin(GL_QUADS);
	//�O
	glNormal3f(0.0, 0.0, -1.0);
	glVertex3f(width / 2, height / 2, depth / 2);
	glVertex3f(-width / 2, height / 2, depth / 2);
	glVertex3f(-width / 2, -height / 2, depth / 2);
	glVertex3f(width / 2, -height / 2, depth / 2);

	//��
	glNormal3f(1.0, 0.0, 0.0);
	glVertex3f(width / 2, height / 2, depth / 2);
	glVertex3f(width / 2, height / 2, -depth / 2);
	glVertex3f(width / 2, -height / 2, -depth / 2);
	glVertex3f(width / 2, -height / 2, depth / 2);

	//�E
	glNormal3f(-1.0, 0.0, 0.0);
	glVertex3f(-width / 2, height / 2, -depth / 2);
	glVertex3f(-width / 2, height / 2, depth / 2);
	glVertex3f(-width / 2, -height / 2, depth / 2);
	glVertex3f(-width / 2, -height / 2, -depth / 2);

	//��
	glNormal3f(0.0, 0.0, 1.0);
	glVertex3f(width / 2, height / 2, -depth / 2);
	glVertex3f(-width / 2, height / 2, -depth / 2);
	glVertex3f(-width / 2, -height / 2, -depth / 2);
	glVertex3f(width / 2, -height / 2, -depth / 2);

	//��
	glNormal3f(0.0, 1.0, 0.0);
	glVertex3f(width / 2, height / 2, depth / 2);
	glVertex3f(-width / 2, height / 2, depth / 2);
	glVertex3f(-width / 2, height / 2, -depth / 2);
	glVertex3f(width / 2, height / 2, -depth / 2);

	//��
	glNormal3f(0.0, -1.0, 0.0);
	glVertex3f(width / 2, -height / 2, depth / 2);
	glVertex3f(-width / 2, -height / 2, depth / 2);
	glVertex3f(-width / 2, -height / 2, -depth / 2);
	glVertex3f(width / 2, -height / 2, -depth / 2);
	glEnd();
}

//��
void bowl(){

	//�}�e���A���̐ݒ�
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Blue);

	glTranslatef(5.0f, 20.0f, 0.0f);

	//��O����
	glPolygonMode(GL_BACK, GL_FILL);
	glTranslatef(0.0f, -60.0f, 0.0f);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluCylinder(cylinder, 50, 40, 40, 100, 100);
	//glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
	glFrontFace(GL_CW);

	gluCylinder(cylinder, 50, 40, 40, 100, 100);
	//glRotatef(-180.0f, 0.0f, 0.0f, 1.0f);

	//��̒�
	glTranslatef(0.0f, 0.0f, 30.0f);
	glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
    gluDisk(disk, 0, 40, 100, 100);
	glRotatef(-180.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, 0.0f, -30.0f);

	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, 60.0f, 0.0f);

	glTranslatef(-5.0f, -20.0f, 0.0f);

}

int HandleAngle = 0; //�n���h���̉�]�p�x
double HandleHeight = 0; //�n���h���̍���

void IceShaver(){
	//�}�e���A���ݒ�
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, vividBlue);
	
	glTranslatef(0.0f, 100.0f, 0.0f)
		;
	//�����X��{��
	//glutSolidCube(100.0f);
	glTranslatef(0.0f, 40.0f, 0.0f);
	cuboid(100, 10, 100);//���
	glTranslatef(0.0f, -40.0f, 0.0f);

	glTranslatef(0.0f, -20.0f, 0.0f);
	cuboid(100, 10, 100);//����
	glTranslatef(0.0f, 20.0f, 0.0f);

	glTranslatef(-50.0f, -75.0f, 0.0f);
	cuboid(10, 240, 100);//����
	glTranslatef(50.0f, 75.0f, 0.0f);

	glTranslatef(0.0f, -190.0f, 0.0f);
	cuboid(100, 10, 100);//��
	glTranslatef(0.0f, 190.0f, 0.0f);


	//�n���h���̉�]
	glRotatef(HandleAngle, 0.0f, 1.0f, 0.0f);
	//�n���h���̍���
	glTranslatef(0.0f, 70.0f - HandleHeight*1.5, 0.0f);
	//�n���h���̊O���̗ւ���
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	glutSolidTorus(3, 40, 10, 10);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	//�n���h���̃N���X�ɂȂ��Ă���
	glTranslatef(0.0f, 0.0f, -40.0f);
	glutSolidCylinder(2, 80, 30, 30); 
	glTranslatef(0.0f, 0.0f, 40.0f);
	//�n���h���̃N���X�ɂȂ��Ă���
	glTranslatef(-40.0f, 0.0f, 0.0f);
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	glutSolidCylinder(2, 80, 30, 30);
	glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
	glTranslatef(40.0f, 0.0f, 0.0f);
	//�n���h���̎�����
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, 38.0f, 0.0f);
	glutSolidCylinder(3, 30, 30, 30);
	glTranslatef(0.0f, -38.0f, 0.0f);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	//�^�񒆂̖_
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	glutSolidCylinder(3, 60, 30, 30);
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	//�X��}������
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

	//�X
	glRotatef(HandleAngle, 0.0f, 1.0f, 0.0f);
	glTranslatef(0.0f, 90.0f - HandleHeight * 1.5+ (scale*1)/2, 0.0f);
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	cuboid(50 , 50 , 41 - scale*1 ); //scale�ŕX�̑傫��
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, -90.0f + HandleHeight * 1.5 - (scale*1)/2, 0.0f);
	glRotatef(-HandleAngle, 0.0f, 1.0f, 0.0f);

	//���ꂽ�X
	glTranslatef(0.0f, -70.0f, 0.0f);
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	glutSolidCone(scale, scale, 100, 100); //scale�ŕX�̑傫��
	glTranslatef(0.0f, 70.0f, 0.0f);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	//glDisable(GL_BLEND);
	//glDepthMask(GL_TRUE);
}

//���Ǝ��_�̐ݒ�
void myLight(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.6f, 0.6f, 0.6f, 0.6f);//�w�i�F

	glViewport(0, 0, WIDTH, HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//����p,�A�X�y�N�g��(�E�B���h�E�̕�/����),�`�悷��͈�(�ł��߂�����,�ł���������)
	gluPerspective(30.0, (double)WIDTH / (double)HEIGHT, 1.0, 1000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//���_�̐ݒ�
	gluLookAt(450.0, 280.0, -470.0, //�J�����̍��W
		0.0, 0.0, 0.0, // �����_�̍��W
		0.0, 1.0, 0.0); // ��ʂ̏�������w���x�N�g��

	//���C�g�̐ݒ�
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
	//�����X��
	IceShaver();

	//��
	bowl();

	//�X
	Ice();
	
	/*
	//��]
	//glRotatef(anglex, 1.0f, 0.0f, 0.0f);//X������]
	//��
	cylinder(30.0, 20.0, 10);

	//�W
	glMaterialfv(GL_FRONT, GL_DIFFUSE, Red);
	glTranslatef(0.0f, 16.0f, 0.0f);
	glutSolidSphere(28.5, 16, 16);

	//��
	glTranslatef(0.0f, 0.0f, 0.0f);
	cylinder(5.0, 42.0, 10);

	//�����
	glTranslatef(0.0f, 37.0f, -20.0f);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);//X������]
	cylinder(4.6, 20.0, 10);

	//����
	glTranslatef(0.0f, -18.0f, -7.0f);
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);//X������]
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
	HandleAngle = NewEnc * 5 * 0.01; //�n���h���̉�]�p�x

	//�n���h���̍���
	if (70 - HandleHeight > 60 && scale <= 40 && NewEnc > OldEnc){
		HandleHeight += 0.1;
	}
	else if (70 + HandleHeight < 0 && NewEnc < OldEnc){
		HandleHeight -= 5;
	}

	if (scale <= 40 && NewEnc > OldEnc){ //������X�̑傫������̑傫���ȉ��@���@�G���R�[�_�̒l�������Ă��鎞
		scale += (NewEnc - OldEnc) * 0.003f;                   //�X��傫������
	}
	/*else if (scale > 40){  //�X����肫�����̂Ń��[�^�[���~
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

	/*//�`�揉����
	glutInit(&argc, argv);//OpenGL�̏�����
	glutInitWindowPosition(100, 100);//�E�B���h�E�̕\���ʒu
	glutInitWindowSize(WIDTH, HEIGHT);//�E�B���h�E�̑傫��
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);//�f�B�X�v���C�ݒ�
	glutCreateWindow("�`��"); //�E�B���h�E����

	glutDisplayFunc(display);
	glutIdleFunc(idle);

	glutMainLoop();//*/

// ������
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
	
	//�`�揉����
	glutInit(&argc, argv);//OpenGL�̏�����
	glutInitWindowPosition(100, 100);//�E�B���h�E�̕\���ʒu
	glutInitWindowSize(WIDTH, HEIGHT);//�E�B���h�E�̑傫��
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);//�f�B�X�v���C�ݒ�
	glutCreateWindow("�`��"); //�E�B���h�E����

	glutDisplayFunc(display);
	glutIdleFunc(idle);

	glutMainLoop();
	
/**/
// ��{�f�[�^�̑���M�iBasic�j
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

