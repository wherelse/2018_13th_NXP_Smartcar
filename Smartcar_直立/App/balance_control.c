#include "include.h"
float Balance_Kp = 0.8;
float Balance_Kd = -0.51;
float Balance_Out = 0;
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

	//fPWM = ((-g_AngleOfCar * Balance_Kp) + (-AngleSpeed * Balance_Kd));
	//	fPWM = (( - g_AngleOfCar*float01)+( - g_fAngleSpeed[0]*float02));
	//g_upstandControl_PWMTemp[4] = g_upstandControl_PWMTemp[3];
	//g_upstandControl_PWMTemp[3] = g_upstandControl_PWMTemp[2];
	//g_upstandControl_PWMTemp[2] = g_upstandControl_PWMTemp[1];
	//g_upstandControl_PWMTemp[1] = g_upstandControl_PWMTemp[0];
	//g_upstandControl_PWMTemp[0] = fPWM;
	//Balance_Out =fPWM ;
       Balance_Out= ((-g_AngleOfCar * Balance_Kp) + (-AngleSpeed * Balance_Kd));
}
