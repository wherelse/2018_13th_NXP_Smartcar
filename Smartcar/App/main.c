/*!
 *     COPYRIGHT NOTICE
 *     Copyright (c) 2017,山外科技
 *     All rights reserved.
 *     技术讨论：山外论坛 http://www.vcan123.com
 *
 *     除注明出处外，以下所有内容版权均属山外科技所有，未经允许，不得用于商业用途，
 *     修改内容时必须保留山外科技的版权声明。
 *
 * @file       main.c
 * @brief      山外KEA 平台主程序
 * @author     山外科技
 * @version    v6.0
 * @date       2017-12-10
 */

#include "common.h"
#include "include.h"


 //自定义需要保存的数据，务必确保不能全为 0XFF ，否则会识别异常
 //如果有这可能发生，那多加一个变量，值赋值为0即可
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
float vcan_send_buff[4]; //山外上位机虚拟示波器
int ErrLoop = 0;
int flag_run = 2;

int flashsaveflag = 0;
int stoptimes = 0;
int stopflag = 0;
int beepflag = 0;
int stopen = 0;
/*
  本例程用到中断，需要在 MKEA128_it.h 文件里添加：
  #define PIT_CH0_IRQHandler  pit0_irq
  表示pit通道0的中断函数为pit0_irq
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
	static int index = 0;//时序控制标志位
	GetAngle();//获取陀螺仪角度
	Dir_Control();//方向控制
	if (++index > 5)
	{
		led_turn(LED0); //闪烁 LED3
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
		else//停车
		{
			Right_Motor_Control(0);
			Left_Motor_Control(0);
		}
		if (Speed_Kp > 1)//速度环参数为0时速度开环
			SpeedControlOut();

	}
	BalanceControl();//平衡控制
	PIT_Flag_Clear(PIT0); //清中断标志位
}
#else
void pit0_irq(void)
{	
	static int index = 0;//时序控制标志位
	//GetAngle();//获取陀螺仪角度
	Dir_Control();//方向控制
	Speed_calulate();
	if (++index > 5)
	{
		led_turn(LED0); //闪烁 LED3
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
		else//停车
		{
			Right_Motor_Control(0);
			Left_Motor_Control(0);
		}
		if (Speed_Kp > 1)//速度环参数为0时速度开环
			SpeedControlOut();

	}

	circletime--;
	stoptimes--;
	if (circletime < 0)circletime = 0;
	if (stoptimes < 0)stoptimes = 0;
	//if (bInCircle != 0)circletime = 30;
	PIT_Flag_Clear(PIT0); //清中断标志位
}

#endif // BALANCE 

void pit1_irq(void)
{
	key_value = Key_Scan();
	SOLGUI_InputKey(key_value);
	SOLGUI_Menu_PageStage(); //
	SOLGUI_Refresh();        //OLED刷新
	if (key_get(Switch4) == 0)
		OLED_Fill(0x00);       
	PIT_Flag_Clear(PIT1);

}
void main(void)
{
	gpio_init(PTD4, GPO, 0); //初始化蜂鸣器
	gpio_init(PTH6, GPO, 0); //初始化LED0
	gpio_init(PTH7, GPO, 0); //初始化LED1
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
        
	/*PWM初始化：FTM2,CH0-CH3,频率2K,精度1000u*/
	ftm_pwm_init(FTM2, FTM_CH0, 2000, 0);
	ftm_pwm_init(FTM2, FTM_CH1, 2000, 0); //前进通道

	ftm_pwm_init(FTM2, FTM_CH2, 2000, 0); //前进通道
	ftm_pwm_init(FTM2, FTM_CH3, 2000, 0);


	/*oled初始化*/
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
	irq_init(IRQ_PIN, IRQ_PULLUP_EN | IRQ_FALLING);          //配置为下降沿中断
	enable_irq(IRQ_IRQn);
	EnableInterrupts; //打开总的中断开关

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
			flag_run = 0;//出赛道停车判断
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
 *  @brief      main函数
 *  @since      v6.0
 *  @note       山外 flash data

				本例程需要在线调试，看变量的值的变化。
				把 md 加入watch，单步调试，逐步分析md的元素数据变化。
 */

void flash_saveinit(void)
{
	//这部分是用户自己的数据，不一定是结构体，也可以是数组，但数据必须连续，且大小确定
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

        
	//这部分是配置 flash 保存参数
	data.sectornum_start = FLASH_SECTOR_NUM - 8;     //起始扇区      用最后的3个扇区来作为保存参数
	data.sectornum_end = FLASH_SECTOR_NUM - 1;       //结束扇区

	data.data_addr = &mydata;                          //数据的地址
	data.data_size = sizeof(mydata);                  //数据的大小

	//开始函数调用
	flash_data_init(&data);

	//重置flash(看个人需求)
	//flash_data_reset(&data);

	//一开始，不知道数据是否有效的
//	if (flash_data_load(&data))
//	{
//		OLED_P8x16Str(100,0,0,"0"); //加载最后一次存储的数据成功
//	}
}
	 //一般情况下，我们不需要调用 flash_data_reset 来清空，除非你不想要 flash的数据。

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
        
   // data.sectornum_start = FLASH_SECTOR_NUM - 4;     //起始扇区      用最后的3个扇区来作为保存参数
	//data.sectornum_end = FLASH_SECTOR_NUM - 1;       //结束扇区
	data.data_addr = &mydata;                          //数据的地址
	data.data_size = sizeof(mydata);                  //数据的大小
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