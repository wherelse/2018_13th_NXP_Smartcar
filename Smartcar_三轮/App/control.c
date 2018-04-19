#include "control.h"
#include "include.h"


#define ADC_CH0 ADC0_SE12
#define ADC_CH1 ADC0_SE13
#define ADC_CH2 ADC0_SE14
#define ADC_CH3 ADC0_SE15

int motorEncorderL=0,motorEncorderR=0;
int g_base_speed=40,g_real_speed=0,g_set_speed=40,speedMax=200,dutyMax=600,loselinespeed=300;
int loss_line=0;
extern char KEY_NUM;
extern int adc_value[4];

struct pid speed = {-4.5,-0.1,0,0,0,0,0};
struct pid dir = {2,2,0,0,0,0,0};

int normalization_threshold[4]={100,100,100,100};

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
        
	g_real_speed=(motorEncorderL+motorEncorderR)*0.5;
}



void Encoder_init(void)
{
    gpio_init(PTD2, GPI, 0);
    gpio_init(PTD3, GPI, 0);
    ftm_pulse_init(FTM0,FTM_PS_4,PTE7);
    ftm_pulse_init(FTM1,FTM_PS_4,PTE0);
}

/*
*  @brief      ������ƺ���
*  @since      v1.0
*
*/
void Motor_control(void)
{
	float dutyL = 0, dutyR = 0;

	dutyR = g_base_speed + speed.pidout + dir.pidout;
	dutyL = g_base_speed + speed.pidout - dir.pidout;


	//if (loss_line == 2)
	//{
	//	dutyL = -50;
	//	dutyR = dutyMax - 100;
	//}
	//if (loss_line == 1)
	//{
	//	dutyL = dutyMax - 50;
	//	dutyR = -50;
	//}

	static int change = 50;
	if (loss_line == 0)
	{
		change = 50;
	}
	if (loss_line == 1)
	{
		dutyL = loselinespeed;
		dutyR = -change;
		change += 5;
		if (change > dutyMax)change = dutyMax;
	}
	if (loss_line == 2)
	{
		dutyL = -change;
		dutyR = loselinespeed;
		change += 5;
		if (change > dutyMax)change = dutyMax;
	}

	if (dutyL > dutyMax)
		dutyL = dutyMax;
	if (dutyL < (-dutyMax))
		dutyL = -dutyMax;
        
	if (dutyR > dutyMax)
		dutyR = dutyMax;
	if (dutyR < (-dutyMax))
		dutyR = -dutyMax;

	if (dutyL < 0)
	{
		ftm_pwm_duty(FTM2, FTM_CH0, 0);
		ftm_pwm_duty(FTM2, FTM_CH1, (-dutyL));
	}
	else if (dutyL > 0)
	{
		ftm_pwm_duty(FTM2, FTM_CH0, dutyL);
		ftm_pwm_duty(FTM2, FTM_CH1, 0);
	}
        else
        {
          	ftm_pwm_duty(FTM2, FTM_CH0, 0);
		ftm_pwm_duty(FTM2, FTM_CH1, 0);
        }

	if (dutyR < 0)
	{
		ftm_pwm_duty(FTM2, FTM_CH2, (-dutyR));
		ftm_pwm_duty(FTM2, FTM_CH3, 0);
	}
	else if (dutyR > 0)
	{
		ftm_pwm_duty(FTM2, FTM_CH2, 0);
		ftm_pwm_duty(FTM2, FTM_CH3, dutyR);
	}
        else
        {
        ftm_pwm_duty(FTM2, FTM_CH2, 0);
		ftm_pwm_duty(FTM2, FTM_CH3, 0);
        }
}

/*
*  @brief      �ٶȻ�
*  @since      v1.0
*
*/
void Speed_PIControl(int set_speed,int real_speed)
{
	uint16 SpeedMax = 500;
	speed.err = set_speed - real_speed;
	//if (speed.err > 30)speed.err = 30;
	//if (speed.err < -30)speed.err = -30;
	speed.pidout = speed.Kp*speed.err + speed.Ki*speed.i;
	if (fabs(speed.err) > 50)speed.i = 0;
	speed.i += speed.err;
}

/*
*  @brief      �������
*  @since      v1.0
*  
*/
void Dir_PdControl(void)
{
	static float pid_out_last=0;
	dir.err = adc_value[0]-adc_value[3];//ADC_deal(adc_value);
	dir.pidout = dir.Kp * dir.err + dir.Kd * (dir.err - dir.err_last);
	dir.err_last = dir.err;
	//if (dir.pidout > 0 && pid_out_last < 0)dir.pidout = pid_out_last + 10;
	//else if (dir.pidout < 0 && pid_out_last>0)dir.pidout = pid_out_last - 10;
	//else pid_out_last = dir.pidout;
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
	error_add[0] = position;								  //������λΪ��������
	error_tatal = error_add[0] + error_add[1] + error_add[2]; //�������ֵ�ĺ�
	sensor_value_all = adc_value[0] + adc_value[1] + adc_value[2] + adc_value[3];
	if (((leftRange > sensor_value_all) || (rightRange < sensor_value_all) )&& (adc_value[0]<10||adc_value[3]<10))//�жϵ��ֵ�Ƿ�С�ڶ�����ֵ
	{
		if ((error_tatal > 0) && (loss_line == 0) && (leftRange > sensor_value_all))
			{
				loss_line = 2;
                loss_line_lock=1;
			}
		else if ((error_tatal < 0) && (loss_line == 0) && (rightRange > sensor_value_all ))
			{
				loss_line = 1;
                loss_line_lock=1;
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

void get_sensor_threshold_normalization(void)
{
	int sensor_value_now[4] = { 0,0,0,0 };
	static int sensor_value_last[4] = { 0,0,0,0 };
	static int sensor_value_max[4] = { 0,0,0,0 };
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
		normalization_threshold[i] = sensor_value_max[i];
	}
	
}

void sensorValue_get(int * sensor_value)
{
	sensor_value[0] = adc_once(ADC_CH0, ADC_10bit);
	sensor_value[1] = adc_once(ADC_CH1, ADC_10bit);
	sensor_value[2] = adc_once(ADC_CH2, ADC_10bit);
	sensor_value[3] = adc_once(ADC_CH3, ADC_10bit);
}

/*
*  @brief      ���������ݹ�һ��
*  @since      v1.0
*   sensor_value_normal   ��һ��������
*   sensor_value       ��һ��ǰ����
*/
void Senser_normalization(int * sensor_value)
{
	sensor_value[0] = (int)((sensor_value[0] / (float)(normalization_threshold[0])) * 100);
	sensor_value[1] = (int)((sensor_value[1] / (float)(normalization_threshold[1])) * 100);
	sensor_value[2] = (int)((sensor_value[2] / (float)(normalization_threshold[2])) * 100);
	sensor_value[3] = (int)((sensor_value[3] / (float)(normalization_threshold[3])) * 100);
}

/*
*  @brief      ƫ�����
*  @since      v1.0
*  ad_normal   ��һ��ǰ����
*/
float cal_deviation(uint16 * sensor_value)
{
	float amp = 10000.0f;		//�Ŵ���
	float pwr_total = 0.0f;
	float position = 0.0f;
	float ad_sum = 0.0f;
	float sensor_value_copy[6] = { 0.0f,0.0f,0.0f,0.0f,0.0f,0.0f };
	float ad_cal[4] = { 0.0f,0.0f,0.0f,0.0f };
	float minus[3] = { 0.0f,0.0f,0.0f };
	float multiply[3] = { 0.0f,0.0f,0.0f };
	float line[3] = { 0.0f,0.0f,0.0f };
	float pwr[3] = { 0.0f,0.0f,0.0f };
	float kp[3] = { 0.0f,0.0f,0.0f };

	for (uint8 i = 0; i<6; i++)
	{
		sensor_value_copy[i] = (float)(sensor_value[i]);		//��ԭ���鸴��һ�ݣ��Ա㴦�� 
	}

	sensor_value_copy[0] += 2.5f;		//������ֵΪ0ʱ�ᷢ�������������ÿ��ֵ������1 
	sensor_value_copy[1] += 2.5f;
	sensor_value_copy[2] += 2.5f;
	sensor_value_copy[3] += 2.5f;
	sensor_value_copy[4] += 2.5f;
	sensor_value_copy[5] += 2.5f;

	ad_cal[0] = sensor_value_copy[0];
	ad_cal[1] = sensor_value_copy[2];
	ad_cal[2] = sensor_value_copy[3];
	ad_cal[3] = sensor_value_copy[5];

	//        ad_sum = ad_cal[0] + ad_cal[1] + ad_cal[2] + ad_cal[3];

	//	ad_cal[0] = (ad_cal[0] / ad_sum) * 295.0f ;		//�������
	//	ad_cal[1] = (ad_cal[1] / ad_sum) * 295.0f ;
	//	ad_cal[2] = (ad_cal[2] / ad_sum) * 295.0f ;
	//	ad_cal[3] = (ad_cal[3] / ad_sum) * 295.0f ;

	minus[0] = (int)((ad_cal[1] - ad_cal[0]) * amp);		//���������������
	minus[1] = (int)((ad_cal[2] - ad_cal[1]) * amp);
	minus[2] = (int)((ad_cal[3] - ad_cal[2]) * amp);

	multiply[0] = ((int)(ad_cal[0] + 10.0f)) * ((int)(ad_cal[1] + 10.0f));			//��������������
	multiply[1] = ((int)(ad_cal[1] + 10.0f)) * ((int)(ad_cal[2] + 10.0f));
	multiply[2] = ((int)(ad_cal[2] + 10.0f)) * ((int)(ad_cal[3] + 10.0f));

	line[0] = minus[0] / multiply[0];			//��Ȼ������������
	line[1] = minus[1] / multiply[1];
	line[2] = minus[2] / multiply[2];

	//        printf("%5d %5d %5d\r\n",(int)(line[0]*100),(int)(line[1]*100),(int)(line[2]*100));

	line[0] = line[0] - 48.77;				//�Եڶ�������Ϊ��׼������Ư�ƣ����ߺ�һ
	line[1] = line[1];
	line[2] = line[2] + 45.60f;

	pwr[0] = (int)(ad_cal[0] + ad_cal[1]);			//���������������
	pwr[1] = (int)(ad_cal[1] + ad_cal[2]);
	pwr[2] = (int)(ad_cal[2] + ad_cal[3]);

	pwr[0] = pwr[0] * pwr[0];				//��һ�������ĺ�ֵ��ƽ��
	pwr[1] = pwr[1] * pwr[1];
	pwr[2] = pwr[2] * pwr[2];

	pwr_total = pwr[0] + pwr[1] + pwr[2];		//�����һ������ƽ���ĺ�

	kp[0] = ((float)(pwr[0])) / ((float)(pwr_total));			//������ֱ�����ܺ͵ó���������
	kp[1] = ((float)(pwr[1])) / ((float)(pwr_total));			//����������ڶ�̬������յ�����
	kp[2] = ((float)(pwr[2])) / ((float)(pwr_total));			//Ϊɶ��ô����Ҳ��֪��

	position = ((kp[0] * line[0]) + (kp[1] * line[1]) + (kp[2] * line[2])) * 1;

	return position;
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












































































