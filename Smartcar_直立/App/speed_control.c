#include "speed_control.h"
#include"include.h"
/**
* @file		�����ٶ�ƫ��
* @note      	����ȫ�ֱ���g_fSpeedError
* @author     jassy
* @date		2017
*/
float RealSpeed = 0;
float ExpectSpeed = 0;
float SpeedErr = 0;
float SpeedErrorTemp[5];
float SpeedOut = 0;
float SpeedOutNew = 0;
float SpeedOutOld = 0;
float Speed_Kp = 6.5;
float Speed_Ki = 0.15;
float Speed_i = 0;
int Flag_SpeedControl = 0;
/*
 @ ����:��
 @ ���ܽ��� �����ٶ�ƫ��
 @ ����:
 @ null
*/
void CalSpeedError(void)
{
	static float speed_old = 0, speed_filter = 0;
	const float speed_new = (motorEncorderL + motorEncorderR)*0.5;				
	RealSpeed = speed_new;														//��ʵ�ٶ�
	speed_old = speed_filter;

	//���Ʊ仯����
	if (speed_new >= speed_old)
		speed_filter = ((speed_new - speed_old) > 10 ? (speed_old + 10) : speed_new);
	else
		speed_filter = ((speed_new - speed_old) < -10 ? (speed_old - 10) : speed_new);

	SpeedErr = ExpectSpeed - RealSpeed; 

	SpeedErrorTemp[4] = SpeedErrorTemp[3];
	SpeedErrorTemp[3] = SpeedErrorTemp[2];
	SpeedErrorTemp[2] = SpeedErrorTemp[1];
	SpeedErrorTemp[1] = SpeedErrorTemp[0];
	SpeedErrorTemp[0] = SpeedErr;

}

/**
* @file		�ٶȿ���
* @note      	�ٶ��ݶ�ƽ��
* @author     jassy
* @date		2017
*/
void SpeedControl(void)
{
	//	static float fSpeedErrorDot = 0;
	CalSpeedError();
	if (SpeedErr > 0)
		SpeedErr = (SpeedErr > 10 ? 10 : SpeedErr);//�ٶ�ƫ���޷�
	else
		SpeedErr = (SpeedErr < -10 ? -10 : SpeedErr);//�ٶ�ƫ���޷�
	vcan_send_buff[3] = SpeedErr; 
	SpeedOutOld = SpeedOutNew;
	SpeedOutNew = Speed_Kp * SpeedErr + Speed_Ki*Speed_i;
	Speed_i += SpeedErr;
	if (SpeedOutNew > 600)SpeedOutNew = 600;
}
/*
 @ ����:��
 @ ���ܽ��� �ٶ�ƽ�����ƣ�һ���ٶ������Ϊ10�α仯
 @ ����:
 @ null
*/
void SpeedControlOut(void)
{
	SpeedOut = (SpeedOutNew - SpeedOutOld)*Flag_SpeedControl / 10 +
		SpeedOutOld;
}