#include "include.h"
float Balance_Kp = 5.4;
float Balance_Kd = -0.38;
float Balance_Out = 0;
float Balance_Inside_Kp = 0.1;
float Balance_Inside_Kd = 0;
float Balance_Inside_Out = 0;
float Balance_Err, Balance_LastErr, Balance_Inside_Err, Balance_Inside_LastErr;
float AngleSpeed_Old;
/**
* @file			平衡控制的PD计算
* @note      	得到全局变量g_BlanceControlOut
* @author		Jassy
* @date			2017
*/
void BalanceControl(void)
{
	//static float g_upstandControl_PWMTemp[5];
	//float fPWM;
	//车倒保护
	static int index = 0;
	//fPWM = ((-g_AngleOfCar * Balance_Kp) + (-AngleSpeed * Balance_Kd));
	//	fPWM = (( - g_AngleOfCar*float01)+( - g_fAngleSpeed[0]*float02));
	//g_upstandControl_PWMTemp[4] = g_upstandControl_PWMTemp[3];
	//g_upstandControl_PWMTemp[3] = g_upstandControl_PWMTemp[2];
	//g_upstandControl_PWMTemp[2] = g_upstandControl_PWMTemp[1];
	//g_upstandControl_PWMTemp[1] = g_upstandControl_PWMTemp[0];
	//g_upstandControl_PWMTemp[0] = fPWM;
	//Balance_Out =fPWM ;
	if(++index>5)
	{
		index = 0;
		Balance_Out = ((-g_AngleOfCar * Balance_Kp) + (AngleSpeed * Balance_Kd));
	}
	Balance_Inside_LastErr = Balance_Inside_Err;
	Balance_Inside_Err = 0.3*Balance_Inside_LastErr + 0.7*(Balance_Out-AngleSpeed);
	Balance_Inside_Out = Balance_Inside_Err * Balance_Inside_Kp + (Balance_Inside_Err- Balance_Inside_LastErr)* Balance_Inside_Kd;
	//static float errList[5];
	//Balance_Err = -g_AngleOfCar;
	//errList[4] = errList[3];
	//errList[3] = errList[2];
	//errList[2] = errList[1];
	//errList[1] = errList[0];
	//errList[0] = Balance_Err;
	//Balance_LastErr = errList[4];
	//Balance_Out = Balance_Err * Balance_Kp + (Balance_Err-Balance_LastErr)* Balance_Kd;
	//Balance_Inside_Err = Balance_Out - (Balance_Err - Balance_LastErr);
	//Balance_Inside_Out = Balance_Inside_Err*Balance_Inside_Kp + (Balance_Inside_Err-Balance_Inside_LastErr)* Balance_Inside_Kd;
	//Balance_Inside_LastErr = Balance_Inside_Err;
}
