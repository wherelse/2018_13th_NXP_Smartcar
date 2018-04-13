#include "include.h"
float DirErr, DirErrLast;
float DirKp=0.1, DirKd=1;
float DirOut;
void Dir_Control(void)
{
	DirErr = ADC_deal(adc_value);
	DirOut = DirKp * DirErr + DirKd * (DirErr - DirErrLast);
	DirErrLast = DirErr;
}