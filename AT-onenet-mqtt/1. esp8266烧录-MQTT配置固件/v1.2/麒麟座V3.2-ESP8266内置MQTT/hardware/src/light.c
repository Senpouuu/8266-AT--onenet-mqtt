/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	light.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2017-04-06
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		��������
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"

//��Ƭ��������
#include "mcu_gpio.h"
#include "mcu_adc.h"

//Ӳ������
#include "adc.h"
#include "light.h"


LIGHT_INFO light_info;


const static ADC_LIST light_adc_list[1] = {
												{GPIOC, GPIO_Pin_3, (void *)0, ADC1, ADC_Channel_13},
											};


/*
************************************************************
*	�������ƣ�	LIGHT_Init
*
*	�������ܣ�	���������ʼ��
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void LIGHT_Init(void)
{
	
	MCU_GPIO_Init(light_adc_list[0].gpio_list.gpio_group, light_adc_list[0].gpio_list.gpio_pin, GPIO_Mode_AIN, GPIO_Speed_50MHz, light_adc_list[0].gpio_list.gpio_name);
	
	ADCx_Init(ADC1, 1);

}

/*
************************************************************
*	�������ƣ�	LIGHT_GetVoltag
*
*	�������ܣ�	��ȡ��������
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
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
