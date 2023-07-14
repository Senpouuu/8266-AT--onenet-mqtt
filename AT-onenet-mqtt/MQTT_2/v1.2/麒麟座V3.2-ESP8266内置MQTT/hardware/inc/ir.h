#ifndef _IR_H_
#define _IR_H_





#define IR_PWM		TIM8


#define HW_IR_EN	1		//1-使用硬件PWM驱动	0-使用普通IO模拟驱动


_Bool IR_Init(unsigned int hz);

void IR_SendFreq(unsigned short time);

void IR_SendStop(unsigned short time);


#endif
