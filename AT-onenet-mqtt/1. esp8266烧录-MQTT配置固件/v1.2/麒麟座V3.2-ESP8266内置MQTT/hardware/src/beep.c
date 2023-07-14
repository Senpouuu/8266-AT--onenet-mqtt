/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	beep.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-02-27
	*
	*	版本： 		V1.0
	*
	*	说明： 		BEEP初始化、控制
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f10x.h"

//单片机相关组件
#include "mcu_gpio.h"

//驱动
#include "delay.h"
#include "beep.h"


BEEP_INFO beep_info;


const static GPIO_LIST beep_gpio_list[BEEP_NUM] = {
														{GPIOB, GPIO_Pin_3, "beep"},
													};


/*
************************************************************
*	函数名称：	BEEP_Init
*
*	函数功能：	蜂鸣器初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void BEEP_Init(void)
{
	
	unsigned char i = 0;
	
	MCU_GPIO_SWJ_JTAGDisable();
	
	for(; i < BEEP_NUM; i++)
		MCU_GPIO_Init(beep_gpio_list[i].gpio_group, beep_gpio_list[i].gpio_pin, GPIO_Mode_Out_PP, GPIO_Speed_50MHz, beep_gpio_list[i].gpio_name);
	
	BEEP_Ctl(BEEP_ALL, BEEP_OFF);

}

/*
************************************************************
*	函数名称：	BEEP_Ctl
*
*	函数功能：	蜂鸣器控制
*
*	入口参数：	num：BEEP编号
*				status：开关状态
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void BEEP_Ctl(unsigned char num, BEEP_ENUM status)
{
	
	unsigned char i = 0;
	
	RTOS_ENTER_CRITICAL();
	
	if(num < BEEP_NUM)
	{
		MCU_GPIO_Output_Ctl(beep_gpio_list[num].gpio_name, status);
		
		beep_info.beep_status[num] = status;
	}
	else if(num == BEEP_ALL)
	{
		for(; i < BEEP_NUM; i++)
		{
			MCU_GPIO_Output_Ctl(beep_gpio_list[i].gpio_name, status);
			
			beep_info.beep_status[i] = status;
		}
	}
	
	RTOS_EXIT_CRITICAL();

}
