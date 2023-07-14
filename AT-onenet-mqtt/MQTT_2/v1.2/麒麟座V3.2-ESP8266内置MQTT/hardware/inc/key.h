#ifndef _KEY_H_
#define _KEY_H_





#define KEY_NUM			4


/*******************************************
			���������뵯��
*******************************************/
#define KEY_DOWN		1
#define KEY_UP			0

#define KEY_X_DOWN		0
#define KEY_X_UP		1
#define KEY_X_DOUBLE	2
#define KEY_X_DOWNLONG	3

#define KEY_NONE		255


extern unsigned char key_event[KEY_NUM][4];


/*******************************************
			������ʱ
*******************************************/
#define KEYDOWN_LONG_TIME		20		//���㳤��ʱ����Ŀǰkeyboard����ÿ50ms����һ�Ρ�


void KEY_Init(void);

_Bool KEY_Scan(unsigned int num);

unsigned char Keyboard(void);


#endif
