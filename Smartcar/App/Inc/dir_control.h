#ifndef DIR_CONTROL_H
#define DIR_CONTROL_H

void Dir_Control(void);
extern float DirOut;
extern float DirKp , DirKd ;
extern float  DirErr, DirErrLast;
extern float DirKp , DirKd ;
extern int bInCircle ;//Բ����־λ��0����Բ����1�����ҽ���2���������
extern int enCircle ;
extern int cicle_value;
extern int ciclelow;
extern int ciclehigh;
extern int circletime;
#endif // !DIR_CONTROL_H

