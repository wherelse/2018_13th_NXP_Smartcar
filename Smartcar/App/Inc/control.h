#ifndef _control_h
#define _control_h
#include "include.h"

struct pid
{
	float Kp;
	float Ki;
	float Kd;
	float i;
	float err;
	float err_last;
	int pidout;
};
extern struct pid dir,speed;
extern int adc_value[4];
extern int motorEncorderL, motorEncorderR,g_base_speed,g_real_speed, g_set_speed, speedMax, dutyMax;
extern int g_leftThrottle , g_rightThrottle , loss_line ;
/*FTMÕ¼¿Õ±È*/
//#define CH0_Duty
unsigned char Key_check (unsigned char key);
unsigned char Key_Scan(void);
void Key_init(void);
void Speed_calulate(void);
void Encoder_init(void);
void Speed_PidControl(int set_speed,int real_speed);
void Dir_PdControl(void);
void Adc_Init(void);
void sensorValue_get(int * sensor_value);
void beep(unsigned char clk);
int ADC_deal(int * adcValue);
#endif
