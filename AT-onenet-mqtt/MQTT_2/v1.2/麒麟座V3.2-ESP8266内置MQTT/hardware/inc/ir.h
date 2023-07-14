#ifndef _IR_H_
#define _IR_H_





#define IR_PWM		TIM8


#define HW_IR_EN	1		//1-ʹ��Ӳ��PWM����	0-ʹ����ͨIOģ������


_Bool IR_Init(unsigned int hz);

void IR_SendFreq(unsigned short time);

void IR_SendStop(unsigned short time);


#endif
