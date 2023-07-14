/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	mcu_usart.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2018-03-02
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		��Ƭ��EXTI
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��������
#include "mcu_rcc.h"
#include "mcu_gpio.h"
#include "mcu_exti.h"


/*
************************************************************
*	�������ƣ�	MCU_EXTI_Init
*
*	�������ܣ�	��Ƭ��EXTI��ʼ��
*
*	��ڲ�����	GPIO_Group��GPIO��
*				GPIO_Pin�����ź�
*				EXTI_Mode������ģʽ
*				EXTI_Trigger������ѡ��
*				EXTI_LineCmd���Ƿ�ʹ��
*
*	���ز�����	0-�ɹ�	1-ʧ��
*
*	˵����		
************************************************************
*/
_Bool MCU_EXTI_Init(GPIO_TypeDef *GPIO_Group, unsigned short GPIO_Pin,
						EXTIMode_TypeDef EXTI_Mode, EXTITrigger_TypeDef EXTI_Trigger, FunctionalState EXTI_LineCmd)
{
	
	EXTI_InitTypeDef exti_initstruct;

	unsigned char port_source = (((unsigned int)((void *)GPIO_Group) - APB2PERIPH_BASE) >> 10) - 2;
	unsigned char pin_source = 0xFF;
	unsigned int exti_line = GPIO_Pin;
	unsigned char i = 1;
	_Bool result = 1;
	
	if(GPIO_Pin == GPIO_Pin_0)
	{
		pin_source = GPIO_PinSource0;
		result = 0;
	}
	else
	{
		for(; i < GPIO_PinSource15 + 1; i++)
		{
			if(GPIO_Pin >> i == 1)
			{
				pin_source = i;
				result = 0;
				break;
			}
		}
	}
	
	if(result == 0)
	{
		MCU_RCC(AFIO, ENABLE);											//�ⲿ�жϣ���Ҫʹ��AFIOʱ��
		
		GPIO_EXTILineConfig(port_source, pin_source);
		
		exti_initstruct.EXTI_Line = exti_line;
		exti_initstruct.EXTI_LineCmd = EXTI_LineCmd;
		exti_initstruct.EXTI_Mode = EXTI_Mode;
		exti_initstruct.EXTI_Trigger = EXTI_Trigger;
		
		EXTI_Init(&exti_initstruct);
	}
	
	return result;

}
