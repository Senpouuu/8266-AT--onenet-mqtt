#ifndef _MCU_ADC_H_
#define _MCU_ADC_H_


#include "stm32f10x.h"

#include "mcu_gpio.h"


typedef struct
{

	GPIO_LIST gpio_list;
	
	ADC_TypeDef *adc_x;
	
	unsigned char adc_channel;

} ADC_LIST;


void MCU_ADCx_Init(ADC_TypeDef *ADC_Group, unsigned int ADC_Clock, unsigned int ADC_Mode, FunctionalState ADC_ContinuousConvMode,
						unsigned int ADC_ExternalTrigConv, unsigned int ADC_DataAlign,unsigned char ADC_NbrOfChannel,
						const unsigned char *adc_channel_list, _Bool temp_flag);


#endif
