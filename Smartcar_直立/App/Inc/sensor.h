#ifndef _SENSOR_H_
#define _SENSOR_H_
#include "common.h"
#define  qinghua  0
#define  kaerman  1
#define  MPU3050_USED   //使用3050  
//MPU6050_USED    //使用6050

#ifdef MPU3050_USED
/**********全局变量外部申明********/
extern float g_AngleOfCar;
extern float angle_offset;
extern int AngleSpeed;
extern int AngleAccel;
/**********函数申明********/
void KalmanFilter(void);

typedef enum
{
	GyroY,
	GyroX
}Gyron;

#endif

#endif