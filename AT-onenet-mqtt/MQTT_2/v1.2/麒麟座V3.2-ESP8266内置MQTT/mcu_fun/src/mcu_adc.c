/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	mcu_adc.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2018-02-27
	*
	*	版本： 		V1.0
	*
	*	说明： 		单片机ADC
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机相关组件
#include "mcu_rcc.h"
#include "mcu_adc.h"


void MCU_ADCx_Init(ADC_TypeDef *ADC_Group, unsigned int ADC_Clock, unsigned int ADC_Mode, FunctionalState ADC_ContinuousConvMode,
						unsigned int ADC_ExternalTrigConv, unsigned int ADC_DataAlign,unsigned char ADC_NbrOfChannel,
						const unsigned char *adc_channel_list, _Bool temp_flag)
{
	
	unsigned char i = 0;

	ADC_InitTypeDef adc_initstruct;
	
	MCU_RCC(ADC_Group, ENABLE);
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);								//设置ADC分频因子x 72M/x=y,ADC最大时间不能超过14M
	
	ADC_DeInit(ADC_Group);											//复位ADCx,将外设 ADCx 的全部寄存器重设为缺省值
	
	if(ADC_NbrOfChannel > 1)
	{
		adc_initstruct.ADC_NbrOfChannel = ADC_NbrOfChannel;			//顺序进行规则转换的ADC通道的数目
		adc_initstruct.ADC_ScanConvMode = ENABLE;					//模数转换工作在多通道模式
	}
	else if(ADC_NbrOfChannel == 1)
	{
		adc_initstruct.ADC_NbrOfChannel = 1;						//顺序进行规则转换的ADC通道的数目
		adc_initstruct.ADC_ScanConvMode = DISABLE;					//模数转换工作在单通道模式
	}

	adc_initstruct.ADC_ContinuousConvMode = ADC_ContinuousConvMode;	//模数转换工作在连续转换模式
	adc_initstruct.ADC_DataAlign = ADC_DataAlign;					//ADC数据右对齐
	adc_initstruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv;		//转换由软件而不是外部触发启动
	adc_initstruct.ADC_Mode = ADC_Mode;								//ADC工作模式:工作在独立模式
	
	ADC_Init(ADC_Group, &adc_initstruct);							//初始化ADC
	
	for(; i < ADC_NbrOfChannel; i++)								//ADC通道采集顺序,采样周期
		ADC_RegularChannelConfig(ADC_Group, adc_channel_list[i], i + 1, ADC_SampleTime_41Cycles5);
	
																	//采样时间计算方式：
																	//TCONV = 采样周期 + 12.5个周期
																	//ADC时钟，如本例的12MHz
																	//TCONV = 41.5 + 12.5 = 54周期
																	//54 x 1/12 = 4.5us
																	//
																	//在12MHz下，各周期转换一次的时间：
																	//ADC_SampleTime_1Cycles5：1.17us
																	//ADC_SampleTime_7Cycles5：1.67us
																	//ADC_SampleTime_13Cycles5：2.17us
																	//ADC_SampleTime_28Cycles5：3.42us
																	//ADC_SampleTime_41Cycles5：4.5us
																	//ADC_SampleTime_55Cycles5：5.67us
																	//ADC_SampleTime_71Cycles5：7.0us
																	//ADC_SampleTime_239Cycles5：21.0us
	
	if(ADC_Group == ADC1 && temp_flag)
		ADC_TempSensorVrefintCmd(ENABLE);							//开启内部温度传感器//ADC1通道16
	
	ADC_Cmd(ADC_Group, ENABLE);										//使能指定的ADC
	
	ADC_ResetCalibration(ADC_Group);								//使能复位校准  
	 
	while(ADC_GetResetCalibrationStatus(ADC_Group));				//等待复位校准结束
	
	ADC_StartCalibration(ADC_Group);								//开启AD校准
 
	while(ADC_GetCalibrationStatus(ADC_Group));						//等待校准结束

}
