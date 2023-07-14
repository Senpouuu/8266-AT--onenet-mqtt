/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	led.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-02-27
	*
	*	版本： 		V1.0
	*
	*	说明： 		LED初始化，亮灭LED
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
#include "led.h"


LED_STATUS led_status;


const static GPIO_LIST led_gpio_list[LED_NUM] = {
													{GPIOC, GPIO_Pin_7, "led1"},
													{GPIOC, GPIO_Pin_8, "led2"},
													{GPIOA, GPIO_Pin_12, "led3"},
													{GPIOC, GPIO_Pin_10, "led4"},
												};


/*
************************************************************
*	函数名称：	LED_Init
*
*	函数功能：	LED初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		高电平关灯		低电平开灯
************************************************************
*/
void LED_Init(void)
{
	
	unsigned char i = 0;
	
	for(; i < LED_NUM; i++)
		MCU_GPIO_Init(led_gpio_list[i].gpio_group, led_gpio_list[i].gpio_pin, GPIO_Mode_Out_PP, GPIO_Speed_50MHz, led_gpio_list[i].gpio_name);
	
	LED_Ctl(LED_ALL, LED_OFF);

}

/*
************************************************************
*	函数名称：	LED_Ctl
*
*	函数功能：	LED控制
*
*	入口参数：	num：LED编号
*				status：开关状态
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void LED_Ctl(unsigned char num, LED_ENUM status)
{
	
	unsigned char i = 0;
	
	RTOS_ENTER_CRITICAL();
	
	if(num < LED_NUM)
	{
		MCU_GPIO_Output_Ctl(led_gpio_list[num].gpio_name, !status);
		
		led_status.led_status[num] = status;
	}
	else if(num == LED_ALL)
	{
		for(; i < LED_NUM; i++)
		{
			MCU_GPIO_Output_Ctl(led_gpio_list[i].gpio_name, !status);
			
			led_status.led_status[i] = status;
		}
	}
	
	RTOS_EXIT_CRITICAL();

}
