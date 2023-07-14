#ifndef _MCU_TIMER_H_
#define _MCU_TIMER_H_


//单片机头文件
#include "stm32f10x.h"


typedef enum
{

	TPC_1 = 1,
	TPC_2 = 2,
	TPC_3 = 4,
	TPC_4 = 8,

} TIMER_PWM_CHANNEL;


void MCU_TIMER_Base_Init(TIM_TypeDef *TIM_Group, unsigned short TIM_ClockDivision, unsigned short TIM_CounterMode,
							unsigned short TIM_Period, unsigned short TIM_Prescaler, unsigned char TIM_RepetitionCounter);

void MCU_Timer_PWM_Init(TIM_TypeDef *TIM_Group, TIMER_PWM_CHANNEL tpc, unsigned short TIM_OCMode, unsigned short TIM_OutputState,
							unsigned short TIM_OutputNState, unsigned short TIM_Pulse, unsigned short TIM_OCPolarity,
							unsigned short TIM_OCNPolarity, unsigned short TIM_OCIdleState, unsigned short TIM_OCNIdleState);


#endif
