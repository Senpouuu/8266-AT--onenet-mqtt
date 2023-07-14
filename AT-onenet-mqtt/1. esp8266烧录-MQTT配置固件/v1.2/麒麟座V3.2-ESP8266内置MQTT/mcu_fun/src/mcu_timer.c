/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	mcu_timer.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2018-02-26
	*
	*	版本： 		V1.0
	*
	*	说明： 		单片机Timer
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机相关组件
#include "mcu_rcc.h"
#include "mcu_timer.h"


/*
************************************************************
*	函数名称：	MCU_TIMER_Base_Init
*
*	函数功能：	单片机Timer定时功能初始化
*
*	入口参数：	TIMER_Group：Timer组
*				TIMER_ClockDivision：
*				TIMER_CounterMode：计数模式
*				TIMER_Period：定时器周期
*				TIMER_Prescaler：时钟分频
*				TIMER_RepetitionCounter：重复计数
*
*	返回参数：	0-成功	1-失败
*
*	说明：		
************************************************************
*/
void MCU_TIMER_Base_Init(TIM_TypeDef *TIM_Group, unsigned short TIM_ClockDivision, unsigned short TIM_CounterMode,
							unsigned short TIM_Period, unsigned short TIM_Prescaler, unsigned char TIM_RepetitionCounter)
{

	TIM_TimeBaseInitTypeDef timer_initstruct;
	
	MCU_RCC(TIM_Group, ENABLE);
	
	timer_initstruct.TIM_ClockDivision = TIM_ClockDivision;
	timer_initstruct.TIM_CounterMode = TIM_CounterMode;
	timer_initstruct.TIM_Period = TIM_Period;
	timer_initstruct.TIM_Prescaler = TIM_Prescaler;
	timer_initstruct.TIM_RepetitionCounter = TIM_RepetitionCounter;
	
	TIM_TimeBaseInit(TIM_Group, &timer_initstruct);

}

void MCU_Timer_PWM_Init(TIM_TypeDef *TIM_Group, TIMER_PWM_CHANNEL tpc, unsigned short TIM_OCMode, unsigned short TIM_OutputState,
							unsigned short TIM_OutputNState, unsigned short TIM_Pulse, unsigned short TIM_OCPolarity,
							unsigned short TIM_OCNPolarity, unsigned short TIM_OCIdleState, unsigned short TIM_OCNIdleState)
{

	TIM_OCInitTypeDef timer_oc_initstruct;
	
	timer_oc_initstruct.TIM_OCMode = TIM_OCMode;
	timer_oc_initstruct.TIM_OutputState = TIM_OutputState;
	timer_oc_initstruct.TIM_OutputNState = TIM_OutputNState;
	timer_oc_initstruct.TIM_Pulse = TIM_Pulse;
	timer_oc_initstruct.TIM_OCPolarity = TIM_OCPolarity;
	timer_oc_initstruct.TIM_OCNPolarity = TIM_OCNPolarity;
	timer_oc_initstruct.TIM_OCIdleState = TIM_OCIdleState;
	timer_oc_initstruct.TIM_OCNIdleState = TIM_OCNIdleState;
	
	if(tpc & 0x01)
	{
		TIM_OC1Init(TIM_Group, &timer_oc_initstruct);
		TIM_OC1PreloadConfig(TIM_Group, TIM_OCPreload_Enable);			//使能TIMx在CCR1上的预装载寄存器
	}
	
	if(tpc & 0x02)
	{
		TIM_OC2Init(TIM_Group, &timer_oc_initstruct);
		TIM_OC2PreloadConfig(TIM_Group, TIM_OCPreload_Enable);			//使能TIMx在CCR2上的预装载寄存器
	}
	
	if(tpc & 0x04)
	{
		TIM_OC3Init(TIM_Group, &timer_oc_initstruct);
		TIM_OC3PreloadConfig(TIM_Group, TIM_OCPreload_Enable);			//使能TIMx在CCR3上的预装载寄存器
	}
	
	if(tpc & 0x08)
	{
		TIM_OC4Init(TIM_Group, &timer_oc_initstruct);
		TIM_OC4PreloadConfig(TIM_Group, TIM_OCPreload_Enable);			//使能TIMx在CCR4上的预装载寄存器
	}
	
	TIM_CtrlPWMOutputs(TIM_Group, ENABLE);								//MOE 主输出使能
	
	TIM_ARRPreloadConfig(TIM_Group, ENABLE);							//ARPE使能

}
