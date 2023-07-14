#ifndef _KEY_H_
#define _KEY_H_





#define KEY_NUM			4


/*******************************************
			按键按下与弹起
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
			按键计时
*******************************************/
#define KEYDOWN_LONG_TIME		20		//计算长按时长。目前keyboard函数每50ms调用一次。


void KEY_Init(void);

_Bool KEY_Scan(unsigned int num);

unsigned char Keyboard(void);


#endif
