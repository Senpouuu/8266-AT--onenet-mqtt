/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	mcu_adc.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2018-02-27
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		��Ƭ��ADC
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��������
#include "mcu_rcc.h"
#include "mcu_adc.h"


void MCU_ADCx_Init(ADC_TypeDef *ADC_Group, unsigned int ADC_Clock, unsigned int ADC_Mode, FunctionalState ADC_ContinuousConvMode,
						unsigned int ADC_ExternalTrigConv, unsigned int ADC_DataAlign,unsigned char ADC_NbrOfChannel,
						const unsigned char *adc_channel_list, _Bool temp_flag)
{
	
	unsigned char i = 0;

	ADC_InitTypeDef adc_initstruct;
	
	MCU_RCC(ADC_Group, ENABLE);
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);								//����ADC��Ƶ����x 72M/x=y,ADC���ʱ�䲻�ܳ���14M
	
	ADC_DeInit(ADC_Group);											//��λADCx,������ ADCx ��ȫ���Ĵ�������Ϊȱʡֵ
	
	if(ADC_NbrOfChannel > 1)
	{
		adc_initstruct.ADC_NbrOfChannel = ADC_NbrOfChannel;			//˳����й���ת����ADCͨ������Ŀ
		adc_initstruct.ADC_ScanConvMode = ENABLE;					//ģ��ת�������ڶ�ͨ��ģʽ
	}
	else if(ADC_NbrOfChannel == 1)
	{
		adc_initstruct.ADC_NbrOfChannel = 1;						//˳����й���ת����ADCͨ������Ŀ
		adc_initstruct.ADC_ScanConvMode = DISABLE;					//ģ��ת�������ڵ�ͨ��ģʽ
	}

	adc_initstruct.ADC_ContinuousConvMode = ADC_ContinuousConvMode;	//ģ��ת������������ת��ģʽ
	adc_initstruct.ADC_DataAlign = ADC_DataAlign;					//ADC�����Ҷ���
	adc_initstruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv;		//ת��������������ⲿ��������
	adc_initstruct.ADC_Mode = ADC_Mode;								//ADC����ģʽ:�����ڶ���ģʽ
	
	ADC_Init(ADC_Group, &adc_initstruct);							//��ʼ��ADC
	
	for(; i < ADC_NbrOfChannel; i++)								//ADCͨ���ɼ�˳��,��������
		ADC_RegularChannelConfig(ADC_Group, adc_channel_list[i], i + 1, ADC_SampleTime_41Cycles5);
	
																	//����ʱ����㷽ʽ��
																	//TCONV = �������� + 12.5������
																	//ADCʱ�ӣ��籾����12MHz
																	//TCONV = 41.5 + 12.5 = 54����
																	//54 x 1/12 = 4.5us
																	//
																	//��12MHz�£�������ת��һ�ε�ʱ�䣺
																	//ADC_SampleTime_1Cycles5��1.17us
																	//ADC_SampleTime_7Cycles5��1.67us
																	//ADC_SampleTime_13Cycles5��2.17us
																	//ADC_SampleTime_28Cycles5��3.42us
																	//ADC_SampleTime_41Cycles5��4.5us
																	//ADC_SampleTime_55Cycles5��5.67us
																	//ADC_SampleTime_71Cycles5��7.0us
																	//ADC_SampleTime_239Cycles5��21.0us
	
	if(ADC_Group == ADC1 && temp_flag)
		ADC_TempSensorVrefintCmd(ENABLE);							//�����ڲ��¶ȴ�����//ADC1ͨ��16
	
	ADC_Cmd(ADC_Group, ENABLE);										//ʹ��ָ����ADC
	
	ADC_ResetCalibration(ADC_Group);								//ʹ�ܸ�λУ׼  
	 
	while(ADC_GetResetCalibrationStatus(ADC_Group));				//�ȴ���λУ׼����
	
	ADC_StartCalibration(ADC_Group);								//����ADУ׼
 
	while(ADC_GetCalibrationStatus(ADC_Group));						//�ȴ�У׼����

}
