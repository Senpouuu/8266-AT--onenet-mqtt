/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	mcu_usart.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2018-08-13
	*
	*	版本： 		V1.1
	*
	*	说明： 		单片机USART
	*
	*	修改记录：	V1.1：增加USART3、UART4、UART5的配置
	************************************************************
	************************************************************
	************************************************************
**/

//单片机相关组件
#include "mcu_rcc.h"
#include "mcu_gpio.h"
#include "mcu_usart.h"


/*
************************************************************
*	函数名称：	MCU_GPIO_Init
*
*	函数功能：	单片机GPIO初始化
*
*	入口参数：	USART_Group：USART组
*				USART_BaudRate：波特率
*				USART_HardwareFlowControl：硬件流控
*				USART_Mode：模式
*				USART_Parity：校验
*				USART_StopBits：停止位
*				USART_WordLength：数据宽度
*
*	返回参数：	0-成功	1-失败
*
*	说明：		
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
		usart_initstruct.USART_HardwareFlowControl = USART_HardwareFlowControl;						//无硬件流控
		usart_initstruct.USART_Mode = USART_Mode;													//接收和发送
		usart_initstruct.USART_Parity = USART_Parity;												//无校验
		usart_initstruct.USART_StopBits = USART_StopBits;											//1位停止位
		usart_initstruct.USART_WordLength = USART_WordLength;										//8位数据位
		
		USART_Init(USART_Group, &usart_initstruct);
	}
	
	return result;

}
