#ifndef __MAIN_H
#define __MAIN_H

extern int adc_value[4],key_value ;
extern char KEY_NUM;
extern float vcan_send_buff[4]; //山外上位机虚拟示波器
extern int ErrLoop, flag_run;

void flash_saveinit(void);
void flash_savedata(void);
void flash_loaddata(void);

#endif