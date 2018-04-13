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
	uint16_t a;
	uint16_t b;
} my_data_t;

flash_data_t data;

int adc_value[4];
int key_value = 0;
char KEY_NUM;
float vcan_send_buff[4]; //ɽ����λ������ʾ����
/*
  �������õ��жϣ���Ҫ�� MKEA128_it.h �ļ�����ӣ�
  #define PIT_CH0_IRQHandler  pit0_irq
  ��ʾpitͨ��0���жϺ���Ϊpit0_irq
*/
void pit0_irq(void)
{
	led_turn(LED0); //��˸ LED3
	Speed_calulate();
	Get_AccData();
	Get_Gyro();
	KalmanFilter();
	BalanceControl();
	/*if(++Flag_SpeedControl>5)
	{
		Flag_SpeedControl = 0;
		SpeedControl();
	}
	SpeedControlOut();*/
	Right_Motor_Control(Balance_Out);
	Left_Motor_Control(Balance_Out);
	Dir_Control();
	PIT_Flag_Clear(PIT0); //���жϱ�־λ
}
void pit1_irq(void)
{
   key_value = Key_Scan();
   SOLGUI_InputKey(key_value);
   SOLGUI_Menu_PageStage(); //
   SOLGUI_Refresh();        //OLEDˢ��
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
	pit_init_ms(PIT0, 8);
	pit_init_ms(PIT1, 100);
	enable_irq(PIT_CH0_IRQn);  
	enable_irq(PIT_CH1_IRQn);
	EnableInterrupts; //���ܵ��жϿ���
	
	adc_init(ADC0_SE12);
	adc_init(ADC0_SE13);
	adc_init(ADC0_SE14);
	adc_init(ADC0_SE15);

	InitMPU6050();
	for (;;)
	{    
		adc_value[0] = adc_once(ADC0_SE12, ADC_10bit);
		adc_value[1] = adc_once(ADC0_SE13, ADC_10bit);
		adc_value[2] = adc_once(ADC0_SE14, ADC_10bit);
		adc_value[3] = adc_once(ADC0_SE15, ADC_10bit);
		vcan_send_buff[0] = g_AngleOfCar;
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
/*void  main(void)
{
	//�ⲿ�����û��Լ������ݣ���һ���ǽṹ�壬Ҳ���������飬�����ݱ����������Ҵ�Сȷ��
	my_data_t  md;
	md.a = 0;
	md.b = 0;

	//�ⲿ�������� flash �������
	data.sectornum_start    = FLASH_SECTOR_NUM - 3;     //��ʼ����      ������3����������Ϊ�������
	data.sectornum_end    = FLASH_SECTOR_NUM - 1;       //��������

	data.data_addr      = &md;                          //���ݵĵ�ַ
	data.data_size      = sizeof(md);                  //���ݵĴ�С

	//��ʼ��������
	flash_data_init(&data);

	//����flash(����������)
	//flash_data_reset(&data);

	//һ��ʼ����֪�������Ƿ���Ч��
	if(flash_data_load(&data))
	{
		//�������һ�δ洢�����ݳɹ�

	}
	else
	{
		//����������Ч��flash�����ǿհ׵�

		flash_data_reset(&data);        //����һ��flash����

		md.a = 0x1234;                  //�Ա�����ʼ��
		md.b = 0x5678;

		//д���ʼ��ֵ
		flash_data_save(&data) ;

		//����������
		md.a = 0;
		md.b = 0;
		flash_data_load(&data) ;
	}

	//����������ǿ�����˵㣬�� md �������Ƿ�ָ���

	//һ������£����ǲ���Ҫ���� flash_data_reset ����գ������㲻��Ҫ flash�����ݡ�

	md.a = 0x3210;
	md.b = 0x8765;
	flash_data_save(&data) ;

	//��������� md �����ݣ�Ȼ��� flash �ָ�����
	md.a = 0;
	md.b = 0;
	flash_data_load(&data) ;


	md.a = 0xabcd;
	md.b = 0x3456;
	flash_data_save(&data) ;

	//��������� md �����ݣ�Ȼ��� flash �ָ�����
	md.a = 0;
	md.b = 0;
	flash_data_load(&data) ;

}*/