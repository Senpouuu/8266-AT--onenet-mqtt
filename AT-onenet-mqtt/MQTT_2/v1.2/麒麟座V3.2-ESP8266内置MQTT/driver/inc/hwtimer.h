#ifndef _HWTIMER_H_
#define _HWTIMER_H_


#include "stm32f10x.h"


typedef struct
{

	unsigned int time_cnt;

} TIMER_INFO;

extern TIMER_INFO timer_info;


void Timer_X_Init(TIM_TypeDef *TIMx, unsigned short psc, unsigned short arr, unsigned char pree_prio, unsigned char sub_prio);

void Timer_X_PWM_Init(TIM_TypeDef *TIMx, unsigned char tp, unsigned short pwm_mode, 
							GPIO_TypeDef *gpio_group, unsigned short gpio_pin, unsigned short psc, unsigned short arr);


#endif
