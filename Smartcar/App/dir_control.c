#include "include.h"
float DirErr, DirErrLast;
float DirKp = 400, DirKd = 200;
float DirOut;
float DirErrList[10];
int bInCircle = 0;//圆环标志位，0：无圆环；1：向右进；2：向左进；
int enCircle = 1;
int cicle_value = 100;
int ciclelow = 45;
int ciclehigh = 200;
int circletime = 0;

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

	//ADC_value[3] ADC_value[2] ADC_value[1] ADC_value[0]
//	//    value[3]    value[2]    value[1]    value[0]
	//       o-----------o-----------o-----------o     
	//                    \         /
	//                     \       /
	//                      \     /
	adc_value[0] = ad_ave(ADC0_SE12, ADC_10bit, 5);
	adc_value[1] = ad_ave(ADC0_SE13, ADC_10bit, 5);
	adc_value[2] = ad_ave(ADC0_SE14, ADC_10bit, 5);
	adc_value[3] = ad_ave(ADC0_SE15, ADC_10bit, 5);
	value[0] = ADC_value[0];
	value[1] = ADC_value[1];
	value[2] = ADC_value[2];
	value[3] = ADC_value[3];

	//--------------------圆环(beta)-----↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓-----圆环(beta)--------------------//
	if (enCircle && circletime == 0)
	{
		//if (value[2] > 400 || value[1] > 400)
		if((value[0]>ciclehigh && (value[1]<300 && value[2]<300 && value[3]<300)) || (value[3]>ciclehigh && (value[1]<300 && value[2]<300 && value[0]<300)))
		{
			if (value[1] < ciclelow && (value[0]>50 && value[2]>50 && value[3]>50) && bInCircle == 0)
			{
				bInCircle = 2; //2：向左进
			}
			else if (value[2] < ciclelow && (value[0]>50 && value[1]>50 && value[3]>50) && bInCircle == 0)
			{
				bInCircle = 1;//1：向右进
			}
		}
		if (bInCircle != 0)
		{
			if (value[0] < 150 && value[3] < 150)
			{
				bInCircle = 0;
				circletime = 400;
			}
			//if (bInCircle == 1 && value[1] < 30)value[0] += cicle_value;//右进
			//else if (bInCircle == 2 && value[2] < 30)value[3] += cicle_value;//左进
			if (bInCircle == 1 && value[1] < 30)value[3] = 0;//右进
			else if (bInCircle == 2 && value[2] < 30)value[0] = 0;//左进
		}
	}
	//--------------------圆环(beta)-----↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑-----圆环(beta)--------------------//

	//感觉没啥用
	value[0] = (value[0] < 5 ? 5 : value[0]);
	value[1] = (value[1] < 5 ? 5 : value[1]);
	value[2] = (value[2] < 5 ? 5 : value[2]);
	value[3] = (value[3] < 5 ? 5 : value[3]);

	g_fDirectionError = (float)(value[0] - value[3]) / (value[0] + value[3]);//差比和

	//感觉也没啥用
	g_fDirectionError = (g_fDirectionError >= 1 ? 1 : g_fDirectionError);//差比和限幅
	g_fDirectionError = (g_fDirectionError <= -1 ? -1 : g_fDirectionError);

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
	DirErrList[4] = DirErr;
	//DirErrList[4] = DirErrList[5];
	//DirErrList[5] = DirErrList[6];
	//DirErrList[6] = DirErrList[7];
	//DirErrList[7] = DirErrList[8];
	//DirErrList[8] = DirErrList[9];
	//DirErrList[9] = DirErr;
	DirectionError_dot = DirErr - DirErrList[0];
	//微分项限幅，按需加
	//DirectionError_dot = (DirectionError_dot>0.05 ? 0.05 : DirectionError_dot);
	//DirectionError_dot = (DirectionError_dot<-0.05 ? -0.05 : DirectionError_dot);
	DirOut = DirKp * DirErr + DirKd * DirectionError_dot;
}