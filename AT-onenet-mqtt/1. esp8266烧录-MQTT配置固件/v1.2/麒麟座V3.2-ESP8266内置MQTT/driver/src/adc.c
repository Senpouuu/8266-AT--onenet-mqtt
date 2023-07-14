/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	adc.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-02-27
	*
	*	版本： 		V1.0
	*
	*	说明： 		ADC初始化、驱动
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机相关组件
#include "mcu_adc.h"
#include "mcu_dma.h"

//硬件驱动
#include "delay.h"
#include "adc.h"


static _Bool adc_busy[2] = {ADC_OK, ADC_OK};


/*****************************************************************************

使用说明
1.根据场景修改adc.h的 ADC_DMA_EN 宏和 ADC_CH_NUM 宏

2.修改 adc_channel_list 通道列表，填入要采集的通道

*****************************************************************************/


unsigned short adc_value_list[ADC_CH_NUM];


//注意：在ADCx_Init之前必须先填写通道列表!!!!!!!!!!!!!!!!!!!!!!!!!!!!
const unsigned char adc_channel_list[ADC_CH_NUM] = {ADC_Channel_13, ADC_Channel_16};


/*
************************************************************
*	函数名称：	ADC_IsBusReady
*
*	函数功能：	查询总线是否就绪
*
*	入口参数：	adc_x：ADC1 或 ADC3
*
*	返回参数：	0-就绪	1-未就绪
*
*	说明：		
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
*	函数名称：	ADC_FreeBus
*
*	函数功能：	释放总线
*
*	入口参数：	adc_x：ADC1 或 ADC3
*
*	返回参数：	无
*
*	说明：		
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
*	函数名称：	ADCx_Init
*
*	函数功能：	ADCx初始化
*
*	入口参数：	ADCx：ADC设备
*				temp_flag：1-启用内部温度测量	0-不启用内部温度测量
*
*	返回参数：	无
*
*	说明：		只有ADC1具有内部温度测量功能
*				重要：需要自行初始化对应的GPIO！！！！
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
		
		DMA_Cmd(DMA1_Channel1, ENABLE);											//使能DMA通道
	}
	else if(ADCx == ADC3)
	{
		MCU_DMA_Init(DMA2, DMA2_Channel5, (unsigned int)&ADCx->DR, (unsigned int)adc_value_list, DMA_DIR_PeripheralSRC, ADC_CH_NUM,
					DMA_PeripheralInc_Disable, DMA_MemoryInc_Enable, DMA_PeripheralDataSize_HalfWord,
					DMA_MemoryDataSize_HalfWord, DMA_Mode_Circular, DMA_Priority_Medium, DMA_M2M_Disable);
		
		DMA_Cmd(DMA2_Channel5, ENABLE);											//使能DMA通道
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
*	函数名称：	ADCx_GetValue
*
*	函数功能：	获取一次ADCx的值
*
*	入口参数：	ADCx：ADC设备
*				ch：通道
*
*	返回参数：	ADCx转换后的数字量
*
*	说明：		ADC_Channel_1~ADC_Channel_16
************************************************************
*/
unsigned short ADCx_GetValue(ADC_TypeDef *ADCx, unsigned char ch)
{

#if(ADC_DMA_EN == 1)
	unsigned char i = 0;
	
	while(ch != adc_channel_list[i])					//找到要获取的通道顺序号
	{
		if(++i >= ADC_CH_NUM)
			return 0;
	}
	
	if(ADCx == ADC1)
		while(!DMA_GetFlagStatus(DMA1_FLAG_TC1));		//等待DMA传输完成
	else if(ADCx == ADC3)
		while(!DMA_GetFlagStatus(DMA2_FLAG_TC5));		//等待DMA传输完成
	
	return adc_value_list[i];							//返回这个通道的数据
#else
	ADC_SoftwareStartConvCmd(ADCx, ENABLE);				//使能指定的ADC1的软件转换启动功能
	 
	while(!ADC_GetFlagStatus(ADCx, ADC_FLAG_EOC));		//等待转换结束

	return ADC_GetConversionValue(ADCx);				//返回最近一次ADC1规则组的转换结果
#endif

}

/*
************************************************************
*	函数名称：	ADCx_GetValueTimes
*
*	函数功能：	获取多次ADCx的值，并求平均值
*
*	入口参数：	ADCx：ADC设备
*				ch：通道
*				times：次数
*
*	返回参数：	电压平均值
*
*	说明：		ADC_Channel_1~ADC_Channel_16
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
*	函数名称：	ADCx_GetVoltag
*
*	函数功能：	获取多次ADCx的电压值
*
*	入口参数：	ADCx：ADC设备
*				ch：通道
*				times：次数
*
*	返回参数：	电压值
*
*	说明：		ADC_Channel_1~ADC_Channel_16
************************************************************
*/
float ADCx_GetVoltag(ADC_TypeDef *ADCx, unsigned char ch, unsigned char times)
{

	unsigned short voltag = ADCx_GetValueTimes(ADCx, ch, times);
	
	return (float)voltag / 4096 * 3.3;

}

/*
************************************************************
*	函数名称：	ADC1_GetTemperature
*
*	函数功能：	获取ADC通道的内部温度值
*
*	入口参数：	无
*
*	返回参数：	温度值
*
*	说明：		
************************************************************
*/
float ADC1_GetTemperature(void)
{
	
	if(ADC_IsBusReady(ADC1) == ADC_OK)
	{
		float temp = ADCx_GetValueTimes(ADC1, ADC_Channel_16, 10);	//获取原始AD数据
		
		temp = temp * 3.3 / 4096;									//转换为电压值
			
		ADC_FreeBus(ADC1);
		
		return (1.43 - temp) / 0.0043 + 25;							//计算出当前温度值
	}
	else
		return 0;

}
