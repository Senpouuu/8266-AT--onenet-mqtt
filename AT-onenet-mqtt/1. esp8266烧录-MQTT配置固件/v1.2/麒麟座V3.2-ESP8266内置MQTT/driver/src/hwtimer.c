/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	hwtimer.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2017-02-27
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		��Ƭ����ʱ����ʼ��
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��������
#include "mcu_timer.h"
#include "mcu_nvic.h"
#include "mcu_gpio.h"

//Ӳ������
#include "hwtimer.h"


TIMER_INFO timer_info;


/*
************************************************************
*	�������ƣ�	Timer_X_Init
*
*	�������ܣ�	Timer�Ķ�ʱ����
*
*	��ڲ�����	TIMx��TIMx
*				psc��Ƶֵ
*				arr������ֵ
*				pree_prio����ռ���ȼ�
*				sub_prio�������ȼ�
*
*	���ز�����	��
*
*	˵����		��ʱ����
************************************************************
*/
void Timer_X_Init(TIM_TypeDef *TIMx, unsigned short psc, unsigned short arr, unsigned char pree_prio, unsigned char sub_prio)
{

	MCU_TIMER_Base_Init(TIMx, TIM_CKD_DIV1, TIM_CounterMode_Up, arr, psc, 0);
	
	if(TIMx == TIM1)
		MCU_NVIC_Init(TIM1_UP_IRQn, ENABLE, pree_prio, sub_prio);		//APB2����---72MHz
	else if(TIMx == TIM2)
		MCU_NVIC_Init(TIM2_IRQn, ENABLE, pree_prio, sub_prio);			//APB1����---36MHz
	else if(TIMx == TIM3)
		MCU_NVIC_Init(TIM3_IRQn, ENABLE, pree_prio, sub_prio);			//APB1����---36MHz
	else if(TIMx == TIM4)
		MCU_NVIC_Init(TIM4_IRQn, ENABLE, pree_prio, sub_prio);			//APB1����---36MHz
	else if(TIMx == TIM5)
		MCU_NVIC_Init(TIM5_IRQn, ENABLE, pree_prio, sub_prio);			//APB1����---36MHz
	else if(TIMx == TIM6)
		MCU_NVIC_Init(TIM6_IRQn, ENABLE, pree_prio, sub_prio);			//APB1����---36MHz
	else if(TIMx == TIM7)
		MCU_NVIC_Init(TIM7_IRQn, ENABLE, pree_prio, sub_prio);			//APB1����---36MHz
	else if(TIMx == TIM8)
		MCU_NVIC_Init(TIM8_UP_IRQn, ENABLE, pree_prio, sub_prio);		//APB2����---72MHz
	
	TIM_ITConfig(TIMx, TIM_IT_Update, ENABLE);							//ʹ�ܸ����ж�
	
	TIM_Cmd(TIMx, ENABLE); 												//ʹ�ܶ�ʱ��

}

/*
************************************************************
*	�������ƣ�	Timer_X_PWM_Init
*
*	�������ܣ�	Timer��PWM����
*
*	��ڲ�����	TIMx��TIMx
*				tp��ͨ��1-1 	ͨ��2-2	ͨ��3-4	ͨ��4-8		�ɻ�
*				gpio_group��GPIO��
*				gpio_pin��Pin
*				psc����Ƶֵ
*				arr������ֵ
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void Timer_X_PWM_Init(TIM_TypeDef *TIMx, unsigned char tp, unsigned short pwm_mode,
						GPIO_TypeDef *gpio_group, unsigned short gpio_pin, unsigned short psc, unsigned short arr)
{

	MCU_GPIO_Init(gpio_group, gpio_pin, GPIO_Mode_AF_PP, GPIO_Speed_50MHz, (void *)0);
	
	MCU_TIMER_Base_Init(TIMx, TIM_CKD_DIV1, TIM_CounterMode_Up, arr, psc, 0);
	
	MCU_Timer_PWM_Init(TIMx, tp, pwm_mode, TIM_OutputState_Enable, 0, 0, TIM_OCPolarity_Low, 0, 0, 0);
	
	TIM_Cmd(TIMx, ENABLE);										//ʹ��TIMx

}

/*
************************************************************
*	�������ƣ�	TIM1_UP_IRQHandler
*
*	�������ܣ�	Timer1�����ж�
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void TIM1_UP_IRQHandler(void)
{

	if(TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
		
		timer_info.time_cnt++;
	}

}

/*
************************************************************
*	�������ƣ�	TIM2_IRQHandler
*
*	�������ܣ�	Timer2�ж�
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void TIM2_IRQHandler(void)
{

	if(TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		
		timer_info.time_cnt++;
	}

}

/*
************************************************************
*	�������ƣ�	TIM3_IRQHandler
*
*	�������ܣ�	Timer3�ж�
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void TIM3_IRQHandler(void)
{

	if(TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		
		timer_info.time_cnt++;
	}

}

/*
************************************************************
*	�������ƣ�	TIM4_IRQHandler
*
*	�������ܣ�	Timer4�ж�
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void TIM4_IRQHandler(void)
{

	if(TIM_GetITStatus(TIM4, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
		
		timer_info.time_cnt++;
	}

}

/*
************************************************************
*	�������ƣ�	TIM5_IRQHandler
*
*	�������ܣ�	Timer5�ж�
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void TIM5_IRQHandler(void)
{

	if(TIM_GetITStatus(TIM5, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
		
		timer_info.time_cnt++;
	}

}

/*
************************************************************
*	�������ƣ�	TIM6_IRQHandler
*
*	�������ܣ�	Timer6�ж�
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void TIM6_IRQHandler(void)
{

	if(TIM_GetITStatus(TIM6, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
		
		timer_info.time_cnt++;
	}

}

/*
************************************************************
*	�������ƣ�	TIM7_IRQHandler
*
*	�������ܣ�	Timer7�ж�
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void TIM7_IRQHandler(void)
{

	if(TIM_GetITStatus(TIM7, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
		
		timer_info.time_cnt++;
	}

}

/*
************************************************************
*	�������ƣ�	TIM8_UP_IRQHandler
*
*	�������ܣ�	Timer8�����ж�
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void TIM8_UP_IRQHandler(void)
{

	if(TIM_GetITStatus(TIM8, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM8, TIM_IT_Update);
		
		timer_info.time_cnt++;
	}

}
