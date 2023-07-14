#ifndef _MCU_USART_H_
#define _MCU_USART_H_


//单片机头文件
#include "stm32f10x.h"


_Bool MCU_USART_Init(USART_TypeDef *USART_Group, unsigned int USART_BaudRate, unsigned short USART_HardwareFlowControl,
					unsigned short USART_Mode, unsigned short USART_Parity, unsigned short USART_StopBits, unsigned short USART_WordLength);


#endif
