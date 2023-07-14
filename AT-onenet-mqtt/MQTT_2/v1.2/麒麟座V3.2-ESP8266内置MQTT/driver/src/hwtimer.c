/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	hwtimer.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-02-27
	*
	*	版本： 		V1.0
	*
	*	说明： 		单片机定时器初始化
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机相关组件
#include "mcu_timer.h"
#include "mcu_nvic.h"
#include "mcu_gpio.h"

//硬件驱动
#include "hwtimer.h"


TIMER_INFO timer_info;


/*
************************************************************
*	函数名称：	Timer_X_Init
*
*	函数功能：	Timer的定时配置
*
*	入口参数：	TIMx：TIMx
*				psc分频值
*				arr：重载值
*				pree_prio：抢占优先级
*				sub_prio：子优先级
*
*	返回参数：	无
*
*	说明：		定时功能
************************************************************
*/
void Timer_X_Init(TIM_TypeDef *TIMx, unsigned short psc, unsigned short arr, unsigned char pree_prio, unsigned char sub_prio)
{

	MCU_TIMER_Base_Init(TIMx, TIM_CKD_DIV1, TIM_CounterMode_Up, arr, psc, 0);
	
	if(TIMx == TIM1)
		MCU_NVIC_Init(TIM1_UP_IRQn, ENABLE, pree_prio, sub_prio);		//APB2总线---72MHz
	else if(TIMx == TIM2)
		MCU_NVIC_Init(TIM2_IRQn, ENABLE, pree_prio, sub_prio);			//APB1总线---36MHz
	else if(TIMx == TIM3)
		MCU_NVIC_Init(TIM3_IRQn, ENABLE, pree_prio, sub_prio);			//APB1总线---36MHz
	else if(TIMx == TIM4)
		MCU_NVIC_Init(TIM4_IRQn, ENABLE, pree_prio, sub_prio);			//APB1总线---36MHz
	else if(TIMx == TIM5)
		MCU_NVIC_Init(TIM5_IRQn, ENABLE, pree_prio, sub_prio);			//APB1总线---36MHz
	else if(TIMx == TIM6)
		MCU_NVIC_Init(TIM6_IRQn, ENABLE, pree_prio, sub_prio);			//APB1总线---36MHz
	else if(TIMx == TIM7)
		MCU_NVIC_Init(TIM7_IRQn, ENABLE, pree_prio, sub_prio);			//APB1总线---36MHz
	else if(TIMx == TIM8)
		MCU_NVIC_Init(TIM8_UP_IRQn, ENABLE, pree_prio, sub_prio);		//APB2总线---72MHz
	
	TIM_ITConfig(TIMx, TIM_IT_Update, ENABLE);							//使能更新中断
	
	TIM_Cmd(TIMx, ENABLE); 												//使能定时器

}

/*
************************************************************
*	函数名称：	Timer_X_PWM_Init
*
*	函数功能：	Timer的PWM配置
*
*	入口参数：	TIMx：TIMx
*				tp：通道1-1 	通道2-2	通道3-4	通道4-8		可或
*				gpio_group：GPIO组
*				gpio_pin：Pin
*				psc：分频值
*				arr：重载值
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void Timer_X_PWM_Init(TIM_TypeDef *TIMx, unsigned char tp, unsigned short pwm_mode,
						GPIO_TypeDef *gpio_group, unsigned short gpio_pin, unsigned short psc, unsigned short arr)
{

	MCU_GPIO_Init(gpio_group, gpio_pin, GPIO_Mode_AF_PP, GPIO_Speed_50MHz, (void *)0);
	
	MCU_TIMER_Base_Init(TIMx, TIM_CKD_DIV1, TIM_CounterMode_Up, arr, psc, 0);
	
	MCU_Timer_PWM_Init(TIMx, tp, pwm_mode, TIM_OutputState_Enable, 0, 0, TIM_OCPolarity_Low, 0, 0, 0);
	
	TIM_Cmd(TIMx, ENABLE);										//使能TIMx

}

/*
************************************************************
*	函数名称：	TIM1_UP_IRQHandler
*
*	函数功能：	Timer1更新中断
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
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
*	函数名称：	TIM2_IRQHandler
*
*	函数功能：	Timer2中断
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
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
*	函数名称：	TIM3_IRQHandler
*
*	函数功能：	Timer3中断
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
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
*	函数名称：	TIM4_IRQHandler
*
*	函数功能：	Timer4中断
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
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
*	函数名称：	TIM5_IRQHandler
*
*	函数功能：	Timer5中断
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
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
*	函数名称：	TIM6_IRQHandler
*
*	函数功能：	Timer6中断
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
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
*	函数名称：	TIM7_IRQHandler
*
*	函数功能：	Timer7中断
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
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
*	函数名称：	TIM8_UP_IRQHandler
*
*	函数功能：	Timer8更新中断
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
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
