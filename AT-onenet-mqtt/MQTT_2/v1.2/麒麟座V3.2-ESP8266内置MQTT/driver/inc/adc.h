#ifndef _ADC_H_
#define _ADC_H_


#include "stm32f10x.h"


#define ADC_DMA_EN		1		//1-使能DMA		0-失能DMA功能


#define ADC_CH_NUM		2		//范围在1~16		多通道时，强烈建议使能DMA


#define ADC_OK			0

#define ADC_Err			1


_Bool ADC_IsBusReady(ADC_TypeDef *adc_x);

void ADC_FreeBus(ADC_TypeDef *adc_x);

void ADCx_Init(ADC_TypeDef *ADCx, _Bool temp_flag);

unsigned short ADCx_GetValue(ADC_TypeDef *ADCx, unsigned char ch);

unsigned short ADCx_GetValueTimes(ADC_TypeDef *ADCx, unsigned char ch, unsigned char times);

float ADCx_GetVoltag(ADC_TypeDef *ADCx, unsigned char ch, unsigned char times);

float ADC1_GetTemperature(void);


#endif
