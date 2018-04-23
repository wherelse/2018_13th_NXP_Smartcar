#include "include.h"
float DirErr, DirErrLast;
float DirKp = 130, DirKd = 160;
float DirOut;
float DirErrList[10];
int bInCircle = 0.5;//圆环标志位，0：无圆环；1：向右进；2：向左进；
/*
 @ 作者:邓
 @ 功能介绍 俊奇的获取传感器偏差值
 @ 参数:
 @ 传感器数据数组
*/
float Get_ADC_Err(int *ADC_value)
{
	float g_fDirectionError; 
	static float g_fDirectionErrorTemp[5];
	float value[4];
	//传感器对应

	//ADC_value[0] ADC_value[1] ADC_value[2] ADC_value[3]
	//    value[0]    value[2]    value[3]    value[1]
	//       o-----------o-----------o-----------o     
	//                    \         /
	//                     \       /
	//                      \     /

	value[0] = ADC_value[0];
	value[1] = ADC_value[3];
	value[2] = ADC_value[1];
	value[3] = ADC_value[2];

	//--------------------圆环(beta)-----↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓-----圆环(beta)--------------------//
	if (value[2] > 600 || value[3] > 600)
	{
		//value[0] = value[1];
		if (value[2] > 600 && bInCircle == 0)bInCircle = 1;
		else if (value[3] > 600 && bInCircle == 0)bInCircle = 2;
	}
	if (bInCircle != 0)
	{
		if (value[0] < 100 && value[1] < 100)
		{
			bInCircle = 0;
		}
		if (bInCircle == 1)value[1] = 50;
		else if (bInCircle == 2)value[0] = 50;
	}
	//--------------------圆环(beta)-----↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑-----圆环(beta)--------------------//

	//感觉没啥用
	/*value[0] = (value[0] < 2 ? 2 : value[0]);
	value[1] = (value[1] < 2 ? 2 : value[1]);
	value[2] = (value[2] < 2 ? 2 : value[2]);
	value[3] = (value[3] < 2 ? 2 : value[3]);*/

	g_fDirectionError = (float)(value[0] - value[1]) / (value[0] + value[1]);//差比和

	//感觉也没啥用
	//g_fDirectionError = (g_fDirectionError >= 1 ? 1 : g_fDirectionError);//差比和限幅
	//g_fDirectionError = (g_fDirectionError <= -1 ? -1 : g_fDirectionError);

	g_fDirectionErrorTemp[4] = g_fDirectionErrorTemp[3];
	g_fDirectionErrorTemp[3] = g_fDirectionErrorTemp[2];
	g_fDirectionErrorTemp[2] = g_fDirectionErrorTemp[1];
	g_fDirectionErrorTemp[1] = g_fDirectionErrorTemp[0];
	g_fDirectionErrorTemp[0] = g_fDirectionError;
	return g_fDirectionError;
}
void Dir_Control(void)
{
	float DirectionError_dot;
	DirErr = Get_ADC_Err(adc_value);
	DirErrList[0] = DirErrList[1];
	DirErrList[1] = DirErrList[2];
	DirErrList[2] = DirErrList[3];
	DirErrList[3] = DirErrList[4];
	DirErrList[4] = DirErrList[5];
	DirErrList[5] = DirErrList[6];
	DirErrList[6] = DirErrList[7];
	DirErrList[7] = DirErrList[8];
	DirErrList[8] = DirErrList[9];
	DirErrList[9] = DirErr;
	DirectionError_dot = DirErr - DirErrList[0];
	//微分项限幅，按需加
	//DirectionError_dot = (DirectionError_dot>0.05 ? 0.05 : DirectionError_dot);
	//DirectionError_dot = (DirectionError_dot<-0.05 ? -0.05 : DirectionError_dot);
	DirOut = DirKp * DirErr + DirKd * DirectionError_dot;
}