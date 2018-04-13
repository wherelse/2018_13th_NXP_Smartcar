#include "speed_control.h"
#include"include.h"
/**
* @file		计算速度偏差
* @note      	产生全局变量g_fSpeedError
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
	const float fSpeedNew = (motorEncorderL + motorEncorderR)*0.5;				//真实速度
	RealSpeed = fSpeedNew;														//速度采集梯度平滑，每次采集最大变化200
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
* @file		速度控制
* @note      	速度梯度平滑
* @author     jassy
* @date		2017
*/
void SpeedControl(void)
{
	//	static float fSpeedErrorDot = 0;
	CalSpeedError();
	SpeedErr = (SpeedErr > 10 ? 10 : SpeedErr);//速度偏差限幅
	vcan_send_buff[3] = SpeedErr;  //速度偏差
	SpeedOutOld = SpeedOutNew;
	SpeedOutNew = Speed_Kp * SpeedErr;
}

void SpeedControlOut(void)
{
	SpeedOut = (SpeedOutNew - SpeedOutOld)*Flag_SpeedControl / 10 +
		SpeedOutOld;
}