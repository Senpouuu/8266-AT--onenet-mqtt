/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	mcu_usart.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2018-08-13
	*
	*	�汾�� 		V1.1
	*
	*	˵���� 		��Ƭ��USART
	*
	*	�޸ļ�¼��	V1.1������USART3��UART4��UART5������
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��������
#include "mcu_rcc.h"
#include "mcu_gpio.h"
#include "mcu_usart.h"


/*
************************************************************
*	�������ƣ�	MCU_GPIO_Init
*
*	�������ܣ�	��Ƭ��GPIO��ʼ��
*
*	��ڲ�����	USART_Group��USART��
*				USART_BaudRate��������
*				USART_HardwareFlowControl��Ӳ������
*				USART_Mode��ģʽ
*				USART_Parity��У��
*				USART_StopBits��ֹͣλ
*				USART_WordLength�����ݿ��
*
*	���ز�����	0-�ɹ�	1-ʧ��
*
*	˵����		
************************************************************
*/
_Bool MCU_USART_Init(USART_TypeDef *USART_Group, unsigned int USART_BaudRate, unsigned short USART_HardwareFlowControl,
					unsigned short USART_Mode, unsigned short USART_Parity, unsigned short USART_StopBits, unsigned short USART_WordLength)
{
	
	_Bool result = 1;
	USART_InitTypeDef usart_initstruct;

	if(USART_Group == USART1)
	{
		//PA9	TXD
		MCU_GPIO_Init(GPIOA, GPIO_Pin_9, GPIO_Mode_AF_PP, GPIO_Speed_50MHz, (void *)0);
		
		//PA10	RXD
		MCU_GPIO_Init(GPIOA, GPIO_Pin_10, GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, (void *)0);
		
		result = 0;
	}
	else if(USART_Group == USART2)
	{
		//PA2	TXD
		MCU_GPIO_Init(GPIOA, GPIO_Pin_2, GPIO_Mode_AF_PP, GPIO_Speed_50MHz, (void *)0);
		
		//PA3	RXD
		MCU_GPIO_Init(GPIOA, GPIO_Pin_3, GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, (void *)0);
		
		result = 0;
	}
	else if(USART_Group == USART3)
	{
		//PB10	TXD
		MCU_GPIO_Init(GPIOB, GPIO_Pin_10, GPIO_Mode_AF_PP, GPIO_Speed_50MHz, (void *)0);
		
		//PB11	RXD
		MCU_GPIO_Init(GPIOB, GPIO_Pin_11, GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, (void *)0);
		
		result = 0;
	}
	else if(USART_Group == UART4)
	{
		//PC10	TXD
		MCU_GPIO_Init(GPIOC, GPIO_Pin_10, GPIO_Mode_AF_PP, GPIO_Speed_50MHz, (void *)0);
		
		//PC11	RXD
		MCU_GPIO_Init(GPIOC, GPIO_Pin_11, GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, (void *)0);
		
		result = 0;
	}
	else if(USART_Group == UART5)
	{
		//PC12	TXD
		MCU_GPIO_Init(GPIOC, GPIO_Pin_12, GPIO_Mode_AF_PP, GPIO_Speed_50MHz, (void *)0);
		
		//PD2	RXD
		MCU_GPIO_Init(GPIOD, GPIO_Pin_2, GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, (void *)0);
		
		result = 0;
	}
	
	if(result == 0)
	{
		MCU_RCC(USART_Group, ENABLE);
		
		usart_initstruct.USART_BaudRate = USART_BaudRate;
		usart_initstruct.USART_HardwareFlowControl = USART_HardwareFlowControl;						//��Ӳ������
		usart_initstruct.USART_Mode = USART_Mode;													//���պͷ���
		usart_initstruct.USART_Parity = USART_Parity;												//��У��
		usart_initstruct.USART_StopBits = USART_StopBits;											//1λֹͣλ
		usart_initstruct.USART_WordLength = USART_WordLength;										//8λ����λ
		
		USART_Init(USART_Group, &usart_initstruct);
	}
	
	return result;

}
