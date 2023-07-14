#ifndef _USART_H_
#define _USART_H_


#include "stm32f10x.h"


#define USART_DEBUG			USART1	//调试打印所使用的串口组


#define USART_DMA_TX_EN		1		//1-使能DMA发送		0-失能DMA发送

#define USART_DMA_RX_EN		1		//1-使能DMA接收		0-失能DMA接收


#define USART_TX_TYPE		0

#define USART_RX_TYPE		1


#define USART_OK			0

#define USART_Err			1


typedef struct
{

	char alter_buf[150];
	unsigned char alter_count;
	_Bool rev_idle;

} ALTER_INFO;

extern ALTER_INFO alter_info;


_Bool USART_IsBusReady(USART_TypeDef *usart_x);

void USART_FreeBus(USART_TypeDef *usart_x);

void Usart1_Init(unsigned int baud);

void Usart2_Init(unsigned int baud);

void USARTx_ResetMemoryBaseAddr(USART_TypeDef *usart_x, unsigned int mem_addr, unsigned short num, _Bool type);

void Usart_SendString(USART_TypeDef *usart_x, unsigned char *str, unsigned short len);

void UsartPrintf(USART_TypeDef *usart_x, char *fmt, ...);

void DebugPrintf(char *fmt, ...);


#endif
