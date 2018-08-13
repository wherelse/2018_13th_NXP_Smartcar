#ifndef DIR_CONTROL_H
#define DIR_CONTROL_H

void Dir_Control(void);
extern float DirOut;
extern float DirKp , DirKd ;
extern float  DirErr, DirErrLast;
extern float DirKp , DirKd ;
extern int bInCircle ;//圆环标志位，0：无圆环；1：向右进；2：向左进；
extern int enCircle ;
extern int cicle_value;
extern int ciclelow;
extern int ciclehigh;
extern int circletime;
#endif // !DIR_CONTROL_H

