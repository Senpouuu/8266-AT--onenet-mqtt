/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	adc.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2017-02-27
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		ADC��ʼ��������
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��������
#include "mcu_adc.h"
#include "mcu_dma.h"

//Ӳ������
#include "delay.h"
#include "adc.h"


static _Bool adc_busy[2] = {ADC_OK, ADC_OK};


/*****************************************************************************

ʹ��˵��
1.���ݳ����޸�adc.h�� ADC_DMA_EN ��� ADC_CH_NUM ��

2.�޸� adc_channel_list ͨ���б�����Ҫ�ɼ���ͨ��

*****************************************************************************/


unsigned short adc_value_list[ADC_CH_NUM];


//ע�⣺��ADCx_Init֮ǰ��������дͨ���б�!!!!!!!!!!!!!!!!!!!!!!!!!!!!
const unsigned char adc_channel_list[ADC_CH_NUM] = {ADC_Channel_13, ADC_Channel_16};


/*
************************************************************
*	�������ƣ�	ADC_IsBusReady
*
*	�������ܣ�	��ѯ�����Ƿ����
*
*	��ڲ�����	adc_x��ADC1 �� ADC3
*
*	���ز�����	0-����	1-δ����
*
*	˵����		
************************************************************
*/
_Bool ADC_IsBusReady(ADC_TypeDef *adc_x)
{
	
	_Bool result = ADC_Err;
	
	RTOS_ENTER_CRITICAL();
	
	if(adc_x == ADC1 && adc_busy[0] == ADC_OK)
	{
		adc_busy[0] = ADC_Err;
		
		result = ADC_OK;
	}
	else if(adc_x == ADC3 && adc_busy[1] == ADC_OK)
	{
		adc_busy[1] = ADC_Err;
		
		result = ADC_OK;
	}
	
	RTOS_EXIT_CRITICAL();
	
	return result;

}

/*
************************************************************
*	�������ƣ�	ADC_FreeBus
*
*	�������ܣ�	�ͷ�����
*
*	��ڲ�����	adc_x��ADC1 �� ADC3
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void ADC_FreeBus(ADC_TypeDef *adc_x)
{

	RTOS_ENTER_CRITICAL();
	
	if(adc_x == ADC1)
		adc_busy[0] = ADC_OK;
	else if(adc_x == ADC3)
		adc_busy[1] = ADC_OK;
	
	RTOS_EXIT_CRITICAL();

}

/*
************************************************************
*	�������ƣ�	ADCx_Init
*
*	�������ܣ�	ADCx��ʼ��
*
*	��ڲ�����	ADCx��ADC�豸
*				temp_flag��1-�����ڲ��¶Ȳ���	0-�������ڲ��¶Ȳ���
*
*	���ز�����	��
*
*	˵����		ֻ��ADC1�����ڲ��¶Ȳ�������
*				��Ҫ����Ҫ���г�ʼ����Ӧ��GPIO��������
************************************************************
*/
void ADCx_Init(ADC_TypeDef *ADCx, _Bool temp_flag)
{
	
#if(ADC_DMA_EN == 1)
	if(ADCx == ADC1)
	{
		MCU_DMA_Init(DMA1, DMA1_Channel1, (unsigned int)&ADCx->DR, (unsigned int)adc_value_list, DMA_DIR_PeripheralSRC, ADC_CH_NUM,
					DMA_PeripheralInc_Disable, DMA_MemoryInc_Enable, DMA_PeripheralDataSize_HalfWord,
					DMA_MemoryDataSize_HalfWord, DMA_Mode_Circular, DMA_Priority_Medium, DMA_M2M_Disable);
		
		DMA_Cmd(DMA1_Channel1, ENABLE);											//ʹ��DMAͨ��
	}
	else if(ADCx == ADC3)
	{
		MCU_DMA_Init(DMA2, DMA2_Channel5, (unsigned int)&ADCx->DR, (unsigned int)adc_value_list, DMA_DIR_PeripheralSRC, ADC_CH_NUM,
					DMA_PeripheralInc_Disable, DMA_MemoryInc_Enable, DMA_PeripheralDataSize_HalfWord,
					DMA_MemoryDataSize_HalfWord, DMA_Mode_Circular, DMA_Priority_Medium, DMA_M2M_Disable);
		
		DMA_Cmd(DMA2_Channel5, ENABLE);											//ʹ��DMAͨ��
	}
#endif
	
	MCU_ADCx_Init(ADCx, RCC_PCLK2_Div6, ADC_Mode_Independent, ENABLE, ADC_ExternalTrigConv_None,
					ADC_DataAlign_Right, ADC_CH_NUM, adc_channel_list, temp_flag);
	
#if(ADC_DMA_EN == 1)
	ADC_DMACmd(ADCx, ENABLE);
	
	ADC_SoftwareStartConvCmd(ADCx, ENABLE);
#endif

}

/*
************************************************************
*	�������ƣ�	ADCx_GetValue
*
*	�������ܣ�	��ȡһ��ADCx��ֵ
*
*	��ڲ�����	ADCx��ADC�豸
*				ch��ͨ��
*
*	���ز�����	ADCxת�����������
*
*	˵����		ADC_Channel_1~ADC_Channel_16
************************************************************
*/
unsigned short ADCx_GetValue(ADC_TypeDef *ADCx, unsigned char ch)
{

#if(ADC_DMA_EN == 1)
	unsigned char i = 0;
	
	while(ch != adc_channel_list[i])					//�ҵ�Ҫ��ȡ��ͨ��˳���
	{
		if(++i >= ADC_CH_NUM)
			return 0;
	}
	
	if(ADCx == ADC1)
		while(!DMA_GetFlagStatus(DMA1_FLAG_TC1));		//�ȴ�DMA�������
	else if(ADCx == ADC3)
		while(!DMA_GetFlagStatus(DMA2_FLAG_TC5));		//�ȴ�DMA�������
	
	return adc_value_list[i];							//�������ͨ��������
#else
	ADC_SoftwareStartConvCmd(ADCx, ENABLE);				//ʹ��ָ����ADC1�����ת����������
	 
	while(!ADC_GetFlagStatus(ADCx, ADC_FLAG_EOC));		//�ȴ�ת������

	return ADC_GetConversionValue(ADCx);				//�������һ��ADC1�������ת�����
#endif

}

/*
************************************************************
*	�������ƣ�	ADCx_GetValueTimes
*
*	�������ܣ�	��ȡ���ADCx��ֵ������ƽ��ֵ
*
*	��ڲ�����	ADCx��ADC�豸
*				ch��ͨ��
*				times������
*
*	���ز�����	��ѹƽ��ֵ
*
*	˵����		ADC_Channel_1~ADC_Channel_16
************************************************************
*/
unsigned short ADCx_GetValueTimes(ADC_TypeDef *ADCx, unsigned char ch, unsigned char times)
{

	float adc_value = 0;
	unsigned char i = 0;
		
	for(; i < times; i++)
	{
		adc_value += (float)ADCx_GetValue(ADCx, ch);
#if(ADC_DMA_EN == 0)
		RTOS_TimeDly(1);
#endif
	}
	
	return (unsigned short)(adc_value / times);

}

/*
************************************************************
*	�������ƣ�	ADCx_GetVoltag
*
*	�������ܣ�	��ȡ���ADCx�ĵ�ѹֵ
*
*	��ڲ�����	ADCx��ADC�豸
*				ch��ͨ��
*				times������
*
*	���ز�����	��ѹֵ
*
*	˵����		ADC_Channel_1~ADC_Channel_16
************************************************************
*/
float ADCx_GetVoltag(ADC_TypeDef *ADCx, unsigned char ch, unsigned char times)
{

	unsigned short voltag = ADCx_GetValueTimes(ADCx, ch, times);
	
	return (float)voltag / 4096 * 3.3;

}

/*
************************************************************
*	�������ƣ�	ADC1_GetTemperature
*
*	�������ܣ�	��ȡADCͨ�����ڲ��¶�ֵ
*
*	��ڲ�����	��
*
*	���ز�����	�¶�ֵ
*
*	˵����		
************************************************************
*/
float ADC1_GetTemperature(void)
{
	
	if(ADC_IsBusReady(ADC1) == ADC_OK)
	{
		float temp = ADCx_GetValueTimes(ADC1, ADC_Channel_16, 10);	//��ȡԭʼAD����
		
		temp = temp * 3.3 / 4096;									//ת��Ϊ��ѹֵ
			
		ADC_FreeBus(ADC1);
		
		return (1.43 - temp) / 0.0043 + 25;							//�������ǰ�¶�ֵ
	}
	else
		return 0;

}
