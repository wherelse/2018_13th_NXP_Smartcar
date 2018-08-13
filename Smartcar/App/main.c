/*!
 *     COPYRIGHT NOTICE
 *     Copyright (c) 2017,ɽ��Ƽ�
 *     All rights reserved.
 *     �������ۣ�ɽ����̳ http://www.vcan123.com
 *
 *     ��ע�������⣬�����������ݰ�Ȩ����ɽ��Ƽ����У�δ����������������ҵ��;��
 *     �޸�����ʱ���뱣��ɽ��Ƽ��İ�Ȩ������
 *
 * @file       main.c
 * @brief      ɽ��KEA ƽ̨������
 * @author     ɽ��Ƽ�
 * @version    v6.0
 * @date       2017-12-10
 */

#include "common.h"
#include "include.h"


 //�Զ�����Ҫ��������ݣ����ȷ������ȫΪ 0XFF �������ʶ���쳣
 //���������ܷ������Ƕ��һ��������ֵ��ֵΪ0����
typedef struct
{
	float speedkp;
	float speedki;
	float dirkp;
	float dirkd;
	float balancekp;
	float balancekd;
	int speedmax;
	int circlevalue;
	int circlelow;
	int cirlcehigh;
	int speedadj;
} my_data_t;

flash_data_t data;

int adc_value[4];
int key_value = 0;
char KEY_NUM;
float vcan_send_buff[4]; //ɽ����λ������ʾ����
int ErrLoop = 0;
int flag_run = 2;

int flashsaveflag = 0;
int stoptimes = 0;
int stopflag = 0;
int beepflag = 0;
int stopen = 0;
/*
  �������õ��жϣ���Ҫ�� MKEA128_it.h �ļ�����ӣ�
  #define PIT_CH0_IRQHandler  pit0_irq
  ��ʾpitͨ��0���жϺ���Ϊpit0_irq
*/
void irq_irq(void)
{
	if (IS_IRQ_FLAG() && stopen == 0)
	{
		stopflag++;
		if (stopflag >= 2 && stoptimes==0)
		{
			flag_run = 0;
			stopflag = 0;
		}
		stoptimes = 200;
		beepflag = 1;
	}
	IRQ_CLEAN_FLAG();
}
#ifdef BALANCE 
void pit0_irq(void)
{
	static int index = 0;//ʱ����Ʊ�־λ
	GetAngle();//��ȡ�����ǽǶ�
	Dir_Control();//�������
	if (++index > 5)
	{
		led_turn(LED0); //��˸ LED3
		if (++Flag_SpeedControl > 10)
		{
			Flag_SpeedControl = 0;
			Speed_calulate();
			SpeedControl();
		}
		if (flag_run != 0)
		{

			Right_Motor_Control(Balance_Out - SpeedOut - DirOut);
			Left_Motor_Control(Balance_Out - SpeedOut + DirOut);
		}
		else//ͣ��
		{
			Right_Motor_Control(0);
			Left_Motor_Control(0);
		}
		if (Speed_Kp > 1)//�ٶȻ�����Ϊ0ʱ�ٶȿ���
			SpeedControlOut();

	}
	BalanceControl();//ƽ�����
	PIT_Flag_Clear(PIT0); //���жϱ�־λ
}
#else
void pit0_irq(void)
{	
	static int index = 0;//ʱ����Ʊ�־λ
	//GetAngle();//��ȡ�����ǽǶ�
	Dir_Control();//�������
	Speed_calulate();
	if (++index > 5)
	{
		led_turn(LED0); //��˸ LED3
		if (++Flag_SpeedControl > 10)
		{
			Flag_SpeedControl = 0;
			SpeedControl();
		}
		if (flag_run != 0)
		{

			Right_Motor_Control(Balance_Out - SpeedOut + DirOut);
			Left_Motor_Control(Balance_Out - SpeedOut - DirOut);
		}
		else//ͣ��
		{
			Right_Motor_Control(0);
			Left_Motor_Control(0);
		}
		if (Speed_Kp > 1)//�ٶȻ�����Ϊ0ʱ�ٶȿ���
			SpeedControlOut();

	}

	circletime--;
	stoptimes--;
	if (circletime < 0)circletime = 0;
	if (stoptimes < 0)stoptimes = 0;
	//if (bInCircle != 0)circletime = 30;
	PIT_Flag_Clear(PIT0); //���жϱ�־λ
}

#endif // BALANCE 

void pit1_irq(void)
{
	key_value = Key_Scan();
	SOLGUI_InputKey(key_value);
	SOLGUI_Menu_PageStage(); //
	SOLGUI_Refresh();        //OLEDˢ��
	if (key_get(Switch4) == 0)
		OLED_Fill(0x00);       
	PIT_Flag_Clear(PIT1);

}
void main(void)
{
	gpio_init(PTD4, GPO, 0); //��ʼ��������
	gpio_init(PTH6, GPO, 0); //��ʼ��LED0
	gpio_init(PTH7, GPO, 0); //��ʼ��LED1
	key_init(KEY_MAX);

	OLED_Init();
	OLED_Fill(0x00);
	draw_cauc_logo();

	gpio_turn(PTD4);
	systick_delay_ms(50);
	gpio_turn(PTD4);
	systick_delay_ms(50);
	gpio_turn(PTD4);
	systick_delay_ms(50);
	gpio_turn(PTD4);

	DELAY_MS(500);
        
        flash_saveinit();
        flash_loaddata();
        
	/*PWM��ʼ����FTM2,CH0-CH3,Ƶ��2K,����1000u*/
	ftm_pwm_init(FTM2, FTM_CH0, 2000, 0);
	ftm_pwm_init(FTM2, FTM_CH1, 2000, 0); //ǰ��ͨ��

	ftm_pwm_init(FTM2, FTM_CH2, 2000, 0); //ǰ��ͨ��
	ftm_pwm_init(FTM2, FTM_CH3, 2000, 0);


	/*oled��ʼ��*/
	SOLGUI_Init(&UI_MENU);
	SOLGUI_Refresh();

	led_init(LED0);
	Encoder_init();
	//InitMPU6050();
#ifdef BALANCE
	IIC_init_BMX();
	while (BMX055_init() == 0)
	{
		ErrLoop++;
		if (ErrLoop >= 20)
			while (1);
	};
#endif // BALANCE



	pit_init_ms(PIT0, 5);
	pit_init_ms(PIT1, 100);
	enable_irq(PIT_CH0_IRQn);
	enable_irq(PIT_CH1_IRQn);
	irq_init(IRQ_PIN, IRQ_PULLUP_EN | IRQ_FALLING);          //����Ϊ�½����ж�
	enable_irq(IRQ_IRQn);
	EnableInterrupts; //���ܵ��жϿ���

	adc_init(ADC0_SE12);
	adc_init(ADC0_SE13);
	adc_init(ADC0_SE14);
	adc_init(ADC0_SE15);

	for (;;)
	{
//		adc_value[0] = ad_ave(ADC0_SE12, ADC_10bit,10);
//		adc_value[1] = ad_ave(ADC0_SE13, ADC_10bit,10);
//		adc_value[2] = ad_ave(ADC0_SE14, ADC_10bit,10);
//		adc_value[3] = ad_ave(ADC0_SE15, ADC_10bit,10);
                
		if (adc_value[0] < 10 && adc_value[1] < 10 && adc_value[2] < 10 && adc_value[3] < 10  )
		{
			flag_run = 0;//������ͣ���ж�
		}

		if (bInCircle == 1 || bInCircle == 2 || flashsaveflag == 1|| beepflag==1)
		{
			beep(1);
			flashsaveflag = 0;
			beepflag = 0;
		}
		//if(g_AngleOfCar>300|| g_AngleOfCar<-800 )flag_run = 0;
		//vcan_send_buff[0] = g_AngleOfCar;
		//vcan_send_buff[0] = AccZAngle;
		//vcan_send_buff[1] = SystemAttitude.Pitch;
		//vcan_send_buff[2] = SystemAttitudeRate.Pitch;
		//vcan_sendware((uint8_t *)vcan_send_buff, sizeof(vcan_send_buff));
	}
}


/*!
 *  @brief      main����
 *  @since      v6.0
 *  @note       ɽ�� flash data

				��������Ҫ���ߵ��ԣ���������ֵ�ı仯��
				�� md ����watch���������ԣ��𲽷���md��Ԫ�����ݱ仯��
 */

void flash_saveinit(void)
{
	//�ⲿ�����û��Լ������ݣ���һ���ǽṹ�壬Ҳ���������飬�����ݱ����������Ҵ�Сȷ��
	my_data_t  mydata;
	mydata.speedkp = 2.5;
	mydata.speedki = 0.5;
	mydata.dirkp = 800;
	mydata.dirkd = 800;
	mydata.balancekp = 75;
	mydata.balancekd = 3;
	mydata.speedmax = 900;
	mydata.circlelow = 50;
	mydata.cirlcehigh = 200;
	mydata.speedadj = 10;

        
	//�ⲿ�������� flash �������
	data.sectornum_start = FLASH_SECTOR_NUM - 8;     //��ʼ����      ������3����������Ϊ�������
	data.sectornum_end = FLASH_SECTOR_NUM - 1;       //��������

	data.data_addr = &mydata;                          //���ݵĵ�ַ
	data.data_size = sizeof(mydata);                  //���ݵĴ�С

	//��ʼ��������
	flash_data_init(&data);

	//����flash(����������)
	//flash_data_reset(&data);

	//һ��ʼ����֪�������Ƿ���Ч��
//	if (flash_data_load(&data))
//	{
//		OLED_P8x16Str(100,0,0,"0"); //�������һ�δ洢�����ݳɹ�
//	}
}
	 //һ������£����ǲ���Ҫ���� flash_data_reset ����գ������㲻��Ҫ flash�����ݡ�

void flash_savedata(void)
{
	my_data_t  mydata;

	mydata.speedkp = Speed_Kp;
	mydata.speedki = Speed_Ki;
	mydata.dirkp = DirKp;
	mydata.dirkd = DirKd;
	mydata.balancekp = Balance_Kp;
	mydata.balancekd = Balance_Kd;
	mydata.speedmax = speedMax;
	mydata.circlevalue = cicle_value;
	mydata.circlelow = ciclelow;
	mydata.cirlcehigh = ciclehigh;
	mydata.speedadj = speed_adjust;
    //flash_data_reset(&data);
        
   // data.sectornum_start = FLASH_SECTOR_NUM - 4;     //��ʼ����      ������3����������Ϊ�������
	//data.sectornum_end = FLASH_SECTOR_NUM - 1;       //��������
	data.data_addr = &mydata;                          //���ݵĵ�ַ
	data.data_size = sizeof(mydata);                  //���ݵĴ�С
	flash_data_save(&data);
	flashsaveflag = 1;
}

void flash_loaddata(void)
{
	my_data_t  mydata;

	mydata.speedkp = 2.5;
	mydata.speedki = 0.5;
	mydata.dirkp = 880;
	mydata.dirkd = 850;
	mydata.balancekp = 75;
	mydata.balancekd = 3;
	mydata.speedmax = 900;
	mydata.circlevalue = 600;
	mydata.circlelow = 50;
	mydata.cirlcehigh = 200;
	mydata.speedadj = 10;
	flash_data_load(&data);

	Speed_Kp = mydata.speedkp;
	Speed_Ki = mydata.speedki;
	DirKp = mydata.dirkp;
	DirKd = mydata.dirkd;
	Balance_Kp = mydata.balancekp;
	Balance_Kd = mydata.balancekd;
    speedMax = mydata.speedmax;
	cicle_value = mydata.circlevalue;
	ciclelow = mydata.circlelow;
	ciclehigh = mydata.cirlcehigh;
	speed_adjust = mydata.speedadj;
}