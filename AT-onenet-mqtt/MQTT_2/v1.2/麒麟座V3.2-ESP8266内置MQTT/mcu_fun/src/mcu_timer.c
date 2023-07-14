/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	mcu_timer.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2018-02-26
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		��Ƭ��Timer
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��������
#include "mcu_rcc.h"
#include "mcu_timer.h"


/*
************************************************************
*	�������ƣ�	MCU_TIMER_Base_Init
*
*	�������ܣ�	��Ƭ��Timer��ʱ���ܳ�ʼ��
*
*	��ڲ�����	TIMER_Group��Timer��
*				TIMER_ClockDivision��
*				TIMER_CounterMode������ģʽ
*				TIMER_Period����ʱ������
*				TIMER_Prescaler��ʱ�ӷ�Ƶ
*				TIMER_RepetitionCounter���ظ�����
*
*	���ز�����	0-�ɹ�	1-ʧ��
*
*	˵����		
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
		TIM_OC1PreloadConfig(TIM_Group, TIM_OCPreload_Enable);			//ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���
	}
	
	if(tpc & 0x02)
	{
		TIM_OC2Init(TIM_Group, &timer_oc_initstruct);
		TIM_OC2PreloadConfig(TIM_Group, TIM_OCPreload_Enable);			//ʹ��TIMx��CCR2�ϵ�Ԥװ�ؼĴ���
	}
	
	if(tpc & 0x04)
	{
		TIM_OC3Init(TIM_Group, &timer_oc_initstruct);
		TIM_OC3PreloadConfig(TIM_Group, TIM_OCPreload_Enable);			//ʹ��TIMx��CCR3�ϵ�Ԥװ�ؼĴ���
	}
	
	if(tpc & 0x08)
	{
		TIM_OC4Init(TIM_Group, &timer_oc_initstruct);
		TIM_OC4PreloadConfig(TIM_Group, TIM_OCPreload_Enable);			//ʹ��TIMx��CCR4�ϵ�Ԥװ�ؼĴ���
	}
	
	TIM_CtrlPWMOutputs(TIM_Group, ENABLE);								//MOE �����ʹ��
	
	TIM_ARRPreloadConfig(TIM_Group, ENABLE);							//ARPEʹ��

}
