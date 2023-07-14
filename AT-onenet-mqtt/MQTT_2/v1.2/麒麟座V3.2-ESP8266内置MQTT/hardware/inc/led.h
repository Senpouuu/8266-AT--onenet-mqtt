#ifndef _LED_H_
#define _LED_H_





#define LED_NUM			4

#define LED_ALL			255


typedef struct
{

	_Bool led_status[LED_NUM];

} LED_STATUS;

extern LED_STATUS led_status;


typedef enum
{

	LED_OFF = 0,
	LED_ON

} LED_ENUM;


void LED_Init(void);

void LED_Ctl(unsigned char num, LED_ENUM status);


#endif
