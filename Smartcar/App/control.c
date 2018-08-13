//---------------------------//
//                           //
//------���ļ�Ϊ����汾-------//
//                           //
//---------------------------//
#include "control.h"
#include "include.h"


#define ADC_CH0 ADC0_SE12
#define ADC_CH1 ADC0_SE13
#define ADC_CH2 ADC0_SE14
#define ADC_CH3 ADC0_SE15

int motorEncorderL=0,motorEncorderR=0;
int g_base_speed=20,g_real_speed=0,g_set_speed=120,speedMax=600,dutyMax=600;
int g_leftThrottle=0,g_rightThrottle=0,loss_line=0;
extern char KEY_NUM;


struct pid speed = {5,0.1,0,0,0,0,0};
struct pid dir = {5,0,50,0,0,0,0};

unsigned char Key_Scan(void)
{
	uint8 num = 0x00;
	if (key_get(KEY_A) == KEY_DOWN)
		KEY_NUM = 1;
	else if (key_get(KEY_B) == KEY_DOWN)
		KEY_NUM = 2;
	else if (key_get(KEY_D) == KEY_DOWN)
		KEY_NUM = 3;
	else if (key_get(KEY_U) == KEY_DOWN)
		KEY_NUM = 4;
	else if (key_get(KEY_L) == KEY_DOWN)
		KEY_NUM = 5;
	else if (key_get(KEY_R) == KEY_DOWN)
		KEY_NUM = 6;

	if (key_get(KEY_A) == KEY_DOWN) { num = SOLGUI_KEY_BACK; }
	if (key_get(KEY_B) == KEY_DOWN) { num = SOLGUI_KEY_OK; }
	if (key_get(KEY_D) == KEY_DOWN) { num = SOLGUI_KEY_DOWN; }
	if (key_get(KEY_U) == KEY_DOWN) { num = SOLGUI_KEY_UP; }
	if (key_get(KEY_L) == KEY_DOWN) { num = SOLGUI_KEY_LEFT; }
	if (key_get(KEY_R) == KEY_DOWN) { num = SOLGUI_KEY_RIGHT; }
	
	return num;
}


/*
*  @brief      �ٶȼ���
*  @since      v1.0
*	g_real_speed �����ٶ�
*/
void Speed_calulate(void)
{
	motorEncorderL = ftm_pulse_get(FTM1);
	ftm_clean(FTM1);
	if (gpio_get(PTD2) == 0)
		motorEncorderL = - motorEncorderL;
		
	motorEncorderR = ftm_pulse_get(FTM0);
	ftm_clean(FTM0);
	if (gpio_get(PTD3) == 1)
		motorEncorderR = - motorEncorderR;
		
	g_real_speed=(motorEncorderL+motorEncorderR)/2.0;
}

/*
*  @brief      ����������
*  @since      v1.0
*
*/
void beep(unsigned char clk)
{
	for (int i = 0; i < clk; i++)
	{
		gpio_turn(PTD4);
		systick_delay_ms(50);
		gpio_turn(PTD4);
                systick_delay_ms(50);
	}

}

/*
*  @brief      ��������ʼ������
*  @since      v1.0
*	FTM0,FTM1���ⲿ����
*/
void Encoder_init(void)
{
	gpio_init(PTD2, GPI, 0);
	gpio_init(PTD3, GPI, 0);
	ftm_pulse_init(FTM0,FTM_PS_1,PTE7);
	ftm_pulse_init(FTM1,FTM_PS_1,PTE0);
}

void Motor_control(void)
{
	float dutyL = 0, dutyR = 0;
	if (dir.pidout > 0)
	{
		if ((g_base_speed + speed.pidout - dir.pidout) <= 0)
		{
			dutyR = 0;
			dutyL = g_base_speed + speed.pidout + dir.pidout;
		}
		else
		{
			dutyR = g_base_speed + speed.pidout - dir.pidout;
			dutyL = g_base_speed + speed.pidout + dir.pidout;
		}
	}
	else
	{
		if ((g_base_speed + speed.pidout + dir.pidout) <= 0)
		{
			dutyL = 0;
			dutyR = g_base_speed + speed.pidout - dir.pidout;
		}
		else
		{
			dutyL = g_base_speed + speed.pidout + dir.pidout;
			dutyR = g_base_speed + speed.pidout - dir.pidout;
		}
	}
	if (dutyL > dutyMax)
		dutyL = dutyMax;
	if (dutyR > dutyMax)
		dutyR = dutyMax;

	if (loss_line == 1) { dutyL = 0; dutyR = dutyMax-200; }
	if (loss_line == 2) { dutyL = dutyMax-200;dutyR = 0; }
	ftm_pwm_duty(FTM2, FTM_CH1, dutyL);
	ftm_pwm_duty(FTM2, FTM_CH2, dutyR);
}

void Speed_PidControl(int set_speed,int real_speed)
{
	uint16 SpeedMax = 500;
	static int real_speed_old = 0 ;
	int speed_stand = 0;
	real_speed_old = real_speed;
	speed_stand = real_speed * 0.7 + real_speed_old *0.3;
	speed.err = set_speed - speed_stand;
	speed.pidout = speed.Kp * speed.err + speed.Kd * (speed.err - speed.err_last) + speed.Ki * speed.i;
	speed.err_last = speed.err;
	speed.i += speed.err;
	if (speed.err > 50)
		speed.i = 0;
	if (speed.pidout > SpeedMax)
		speed.pidout = SpeedMax;
	if (speed.pidout < -150)
		speed.pidout = -150;
}
void Dir_PdControl(void)
{
	dir.err = ADC_deal(adc_value);
	dir.pidout = dir.Kp * dir.err + dir.Kd * (dir.err - dir.err_last);
	dir.err_last = dir.err;
}

void Sensor_init(void)
{
  adc_init(ADC_CH0);
  adc_init(ADC_CH1);
  adc_init(ADC_CH2);
  adc_init(ADC_CH3);
}

int ADC_deal(int * adcValue)
{
	int ValueCopy[4],adc_cal[4],position=0;
	u8 i;
	for (i = 0; i < 4; i++)
		ValueCopy[i] = adcValue[i];
	adc_cal[0]=ValueCopy[0]-ValueCopy[1];//�Ҽ�����
	adc_cal[1]=ValueCopy[1]-ValueCopy[2];//���м�����
	adc_cal[2]=ValueCopy[2]-ValueCopy[3];//���м���
	adc_cal[3]=ValueCopy[0]-ValueCopy[3];//�Ҽ���

	position= adc_cal[3] + adc_cal[1];
	return position;
}

void lose_line_deal(int *adc_Value, int position, int leftRange, int rightRange)
{
	static int error_add[3] = {0, 0, 0}, loss_line_inc;
	int loss_line_lock=0;
	int error_tatal = 0, sensor_value_all = 0;

	for (int i = 0; i < 2; i++) //�������ѭ���ƶ�
	{
		error_add[i + 1] = error_add[i];
	}
	error_add[0] = position;								  //������λΪ���������
	error_tatal = error_add[0] + error_add[1] + error_add[2]; //�������ֵ�ĺ�
	sensor_value_all = adc_value[0] + adc_value[1] + adc_value[2] + adc_value[3];
	if ((g_leftThrottle < sensor_value_all) || (g_rightThrottle < sensor_value_all))//�жϵ��ֵ�Ƿ�С�ڶ�����ֵ
	{
		if ((error_tatal > 0) && (loss_line == 0) && (g_leftThrottle < sensor_value_all))
			{
				loss_line = 1;
			}
		else if ((error_tatal < 0) && (loss_line == 1) && (g_rightThrottle < sensor_value_all))
			{
				loss_line = 2;
			}
		else
		{
			;;
		}
	}
	else
	{
		if (loss_line_lock == 1) //�˲�
		{
			loss_line_inc++;
			if (loss_line_inc > 5)
			{
				loss_line_lock = 0;
				loss_line_inc = 0;
			}
		}
		if (loss_line_lock == 0)
		{
			loss_line = 0;
		}
	}
}

int get_sensor_threshold_normalization(void)
{
	int sensor_value_now[4] = { 0,0,0,0 };
	int sensor_value_last[4] = { 0,0,0,0 };
	int sensor_value_max[4] = { 0,0,0,0 };
	sensorValue_get(sensor_value_now);
	for (int i = 0; i < 4; i++)
	{
		if (sensor_value_max[i] < MAX(sensor_value_now[i], sensor_value_last[i]))
		{
			sensor_value_max[i] = MAX(sensor_value_now[i], sensor_value_last[i]);
		}
	}
	for (int i = 0; i < 4; i++)
	{
		sensor_value_last[i] = sensor_value_now[i];
	}

}

void sensorValue_get(int * sensor_value)
{
	sensor_value[0] = adc_once(ADC_CH0, ADC_10bit);
	sensor_value[1] = adc_once(ADC_CH1, ADC_10bit);
	sensor_value[2] = adc_once(ADC_CH2, ADC_10bit);
	sensor_value[3] = adc_once(ADC_CH3, ADC_10bit);
}
//
////�Զ�����Ҫ��������ݣ����ȷ������ȫΪ 0XFF �������ʶ���쳣
////���������ܷ������Ƕ��һ��������ֵ��ֵΪ0����
//typedef struct
//{
//	uint16_t a;
//	uint16_t b;
//} my_data_t;
//
//flash_data_t data;
//
////�ⲿ�������� flash �������
//data.sectornum_start = FLASH_SECTOR_NUM - 3;     //��ʼ����      ������3����������Ϊ�������
//data.sectornum_end = FLASH_SECTOR_NUM - 1;       //��������
//
//data.data_addr = &md;                          //���ݵĵ�ַ
//data.data_size = sizeof(md);                  //���ݵĴ�С
//
//











































































