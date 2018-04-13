#include "speed_control.h"
#include"include.h"
/**
* @file		�����ٶ�ƫ��
* @note      	����ȫ�ֱ���g_fSpeedError
* @author     jassy
* @date		2017
*/
float RealSpeed = 0;
float ExpectSpeed = 11;
float SpeedErr = 0;
float SpeedErrorTemp[5];
float SpeedOut = 0;
float SpeedOutNew = 0; 
float SpeedOutOld = 0;
float Speed_Kp = 20;
int Flag_SpeedControl = 0;
void CalSpeedError(void)
{
	static float fSpeedOld = 0, SpeedFilter = 0;
	const float fSpeedNew = (motorEncorderL + motorEncorderR)*0.5;				//��ʵ�ٶ�
	RealSpeed = fSpeedNew;														//�ٶȲɼ��ݶ�ƽ����ÿ�βɼ����仯200
	fSpeedOld = SpeedFilter;

	if (fSpeedNew >= fSpeedOld)
		SpeedFilter = ((fSpeedNew - fSpeedOld) > 3 ? (fSpeedOld + 3) : fSpeedNew);
	else
		SpeedFilter = ((fSpeedNew - fSpeedOld) < -3 ? (fSpeedOld - 3) : fSpeedNew);

	SpeedErr = ExpectSpeed - SpeedFilter;

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
	SpeedErr = (SpeedErr > 10 ? 10 : SpeedErr);//�ٶ�ƫ���޷�
	vcan_send_buff[3] = SpeedErr;  //�ٶ�ƫ��
	SpeedOutOld = SpeedOutNew;
	SpeedOutNew = Speed_Kp * SpeedErr;
}

void SpeedControlOut(void)
{
	SpeedOut = (SpeedOutNew - SpeedOutOld)*Flag_SpeedControl / 10 +
		SpeedOutOld;
}