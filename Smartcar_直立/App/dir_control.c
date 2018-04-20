#include "include.h"
float DirErr, DirErrLast;
float DirKp=0.1, DirKd=1;
float DirOut;
float DirErrList[10];
void Dir_Control(void)
{
	DirErr = ADC_deal(adc_value);
        DirErrList[0]=DirErrList[1];
        DirErrList[1]=DirErrList[2];
        DirErrList[2]=DirErrList[3];
        DirErrList[3]=DirErrList[4];
        DirErrList[4]=DirErrList[5];
        DirErrList[5]=DirErrList[6];
        DirErrList[6]=DirErrList[7];
        DirErrList[7]=DirErrList[8];
        DirErrList[8]=DirErrList[9];
        DirErrList[9]=DirErr;
	DirOut = DirKp * DirErr + DirKd * (DirErr - DirErrList[0]);
	//DirErrLast = DirErr;
}