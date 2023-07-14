/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	light.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-04-06
	*
	*	版本： 		V1.0
	*
	*	说明： 		光敏电阻
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
#include "mcu_adc.h"

//硬件驱动
#include "adc.h"
#include "light.h"


LIGHT_INFO light_info;


const static ADC_LIST light_adc_list[1] = {
												{GPIOC, GPIO_Pin_3, (void *)0, ADC1, ADC_Channel_13},
											};


/*
************************************************************
*	函数名称：	LIGHT_Init
*
*	函数功能：	光敏电阻初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void LIGHT_Init(void)
{
	
	MCU_GPIO_Init(light_adc_list[0].gpio_list.gpio_group, light_adc_list[0].gpio_list.gpio_pin, GPIO_Mode_AIN, GPIO_Speed_50MHz, light_adc_list[0].gpio_list.gpio_name);
	
	ADCx_Init(ADC1, 1);

}

/*
************************************************************
*	函数名称：	LIGHT_GetVoltag
*
*	函数功能：	获取光敏电阻
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void LIGHT_GetVoltag(void)
{
	
	if(ADC_IsBusReady(light_adc_list[0].adc_x) == ADC_OK)
	{
		light_info.voltag = ADCx_GetVoltag(light_adc_list[0].adc_x, light_adc_list[0].adc_channel, 10);
	
		ADC_FreeBus(light_adc_list[0].adc_x);
	}

}
