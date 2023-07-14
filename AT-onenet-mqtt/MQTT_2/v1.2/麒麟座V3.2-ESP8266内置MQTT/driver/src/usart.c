/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	usart.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2017-02-27
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		��Ƭ�����������ʼ������ʽ����ӡ
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��������
#include "mcu_usart.h"
#include "mcu_nvic.h"
#include "mcu_dma.h"

//Ӳ������
#include "usart.h"
#include "delay.h"

//C��
#include <stdarg.h>
#include <string.h>
#include <stdio.h>


static _Bool usart_busy[3] = {USART_OK, USART_OK, USART_OK};


ALTER_INFO alter_info;


#if(USART_DMA_TX_EN == 1)
unsigned char usart_printbuf[256];
#endif


/*
************************************************************
*	�������ƣ�	USART_IsBusReady
*
*	�������ܣ�	��ѯ�����Ƿ����
*
*	��ڲ�����	usart_x��USARTx
*
*	���ز�����	0-����	1-δ����
*
*	˵����		
************************************************************
*/
_Bool USART_IsBusReady(USART_TypeDef *usart_x)
{
	
	_Bool result = USART_Err;

	RTOS_ENTER_CRITICAL();
	
	if(usart_x == USART1 && usart_busy[0] == USART_OK)
	{
		usart_busy[0] = USART_Err;
		
		result = USART_OK;
	}
	else if(usart_x == USART2 && usart_busy[1] == USART_OK)
	{
		usart_busy[1] = USART_Err;
		
		result = USART_OK;
	}
	else if(usart_x == USART3 && usart_busy[2] == USART_OK)
	{
		usart_busy[2] = USART_Err;
		
		result = USART_OK;
	}
	
	RTOS_EXIT_CRITICAL();
	
	return result;

}

/*
************************************************************
*	�������ƣ�	USART_FreeBus
*
*	�������ܣ�	�ͷ�����
*
*	��ڲ�����	usart_x��USARTx
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void USART_FreeBus(USART_TypeDef *usart_x)
{
	
	RTOS_ENTER_CRITICAL();
	
	if(usart_x == USART1)
		usart_busy[0] = USART_OK;
	else if(usart_x == USART2)
		usart_busy[1] = USART_OK;
	else if(usart_x == USART3)
		usart_busy[2] = USART_OK;
	
	RTOS_EXIT_CRITICAL();

}

/*
************************************************************
*	�������ƣ�	Usart1_Init
*
*	�������ܣ�	����1��ʼ��
*
*	��ڲ�����	baud���趨�Ĳ�����
*
*	���ز�����	��
*
*	˵����		TX-PA9		RX-PA10
*				���ͣ�DMA1_Channel4
*				���գ�DMA1_Channel5
*				δʹ��DMA_TC�жϣ�������RTOS�汾���п��ܻ�ر����ж϶�������ѭ��
************************************************************
*/
void Usart1_Init(unsigned int baud)
{
	
	MCU_USART_Init(USART1, baud, USART_HardwareFlowControl_None, USART_Mode_Rx | USART_Mode_Tx,
					USART_Parity_No, USART_StopBits_1, USART_WordLength_8b);
	
	USART_Cmd(USART1, ENABLE);														//ʹ�ܴ���
	
#if(USART_DMA_RX_EN == 0)
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);									//ʹ�ܽ����ж�
#endif

	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);									//ʹ��IDLE�ж�
	
	MCU_NVIC_Init(USART1_IRQn, ENABLE, 1, 2);
	
#if(USART_DMA_TX_EN == 1)
	MCU_DMA_Init(DMA1, DMA1_Channel4, (unsigned int)&USART1->DR, NULL, DMA_DIR_PeripheralDST, 0,
					DMA_PeripheralInc_Disable, DMA_MemoryInc_Enable, DMA_PeripheralDataSize_Byte,
					DMA_MemoryDataSize_Byte, DMA_Mode_Normal, DMA_Priority_Medium, DMA_M2M_Disable);
	
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);									//ʹ��USART1��DMA���͹���
	
	USARTx_ResetMemoryBaseAddr(USART1, (unsigned int)usart_printbuf, 1, USART_TX_TYPE);//����һ������
#endif

#if(USART_DMA_RX_EN == 1)
	MCU_DMA_Init(DMA1, DMA1_Channel5, (unsigned int)&USART1->DR, NULL, DMA_DIR_PeripheralSRC, 0,
					DMA_PeripheralInc_Disable, DMA_MemoryInc_Enable, DMA_PeripheralDataSize_Byte,
					DMA_MemoryDataSize_Byte, DMA_Mode_Normal, DMA_Priority_Medium, DMA_M2M_Disable);
	
	USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);									//ʹ��USART1��DMA���չ���
#endif

}

/*
************************************************************
*	�������ƣ�	Usart2_Init
*
*	�������ܣ�	����2��ʼ��
*
*	��ڲ�����	baud���趨�Ĳ�����
*
*	���ز�����	��
*
*	˵����		TX-PA2		RX-PA3
*				���ͣ�DMA1_Channel7
*				���գ�DMA1_Channel6
*				δʹ��DMA_TC�жϣ�������RTOS�汾���п��ܻ�ر����ж϶�������ѭ��
************************************************************
*/
void Usart2_Init(unsigned int baud)
{
	
	MCU_USART_Init(USART2, baud, USART_HardwareFlowControl_None, USART_Mode_Rx | USART_Mode_Tx,
					USART_Parity_No, USART_StopBits_1, USART_WordLength_8b);
	
	USART_Cmd(USART2, ENABLE);														//ʹ�ܴ���

#if(USART_DMA_RX_EN == 0)
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);									//ʹ�ܽ����ж�
#endif

	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);									//ʹ��IDLE�ж�
	
	MCU_NVIC_Init(USART2_IRQn, ENABLE, 0, 0);
	
#if(USART_DMA_TX_EN == 1)
	MCU_DMA_Init(DMA1, DMA1_Channel7, (unsigned int)&USART2->DR, NULL, DMA_DIR_PeripheralDST, 0,
					DMA_PeripheralInc_Disable, DMA_MemoryInc_Enable, DMA_PeripheralDataSize_Byte,
					DMA_MemoryDataSize_Byte, DMA_Mode_Normal, DMA_Priority_Medium, DMA_M2M_Disable);
	
	USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);									//ʹ��USART2��DMA���͹���
	
	USARTx_ResetMemoryBaseAddr(USART2, (unsigned int)usart_printbuf, 1, USART_TX_TYPE);//����һ������
#endif
	
#if(USART_DMA_RX_EN == 1)
	MCU_DMA_Init(DMA1, DMA1_Channel6, (unsigned int)&USART2->DR, NULL, DMA_DIR_PeripheralSRC, 0,
					DMA_PeripheralInc_Disable, DMA_MemoryInc_Enable, DMA_PeripheralDataSize_Byte,
					DMA_MemoryDataSize_Byte, DMA_Mode_Normal, DMA_Priority_Medium, DMA_M2M_Disable);
	
	MCU_NVIC_Init(DMA1_Channel6_IRQn, ENABLE, 1, 2);
	
	DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, ENABLE);									//������������ж�

	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);									//ʹ��USART2��DMA���չ���
#endif

}

/*
************************************************************
*	�������ƣ�	USARTx_ResetMemoryBaseAddr
*
*	�������ܣ�	����DMA�ڴ��ַ��ʹ�ܷ���
*
*	��ڲ�����	usart_x��������
*				mem_addr���ڴ��ֵַ
*				num�����η��͵����ݳ���(�ֽ�)
*				type�����ͻ��ǽ���
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void USARTx_ResetMemoryBaseAddr(USART_TypeDef *usart_x, unsigned int mem_addr, unsigned short num, _Bool type)
{

#if(USART_DMA_TX_EN == 1)
	if(type == USART_TX_TYPE)
	{
		if(usart_x == USART1)
			MCU_DMA_ResetMemoryBaseAddr(DMA1_Channel4, mem_addr, num);
		else if(usart_x == USART2)
			MCU_DMA_ResetMemoryBaseAddr(DMA1_Channel7, mem_addr, num);
	}
#endif
	
#if(USART_DMA_RX_EN == 1)
	if(type == USART_RX_TYPE)
	{
		if(usart_x == USART1)
			MCU_DMA_ResetMemoryBaseAddr(DMA1_Channel5, mem_addr, num);
		else if(usart_x == USART2)
			MCU_DMA_ResetMemoryBaseAddr(DMA1_Channel6, mem_addr, num);
	}
#endif

}

/*
************************************************************
*	�������ƣ�	Usart_SendString
*
*	�������ܣ�	�������ݷ���
*
*	��ڲ�����	usart_x��������
*				str��Ҫ���͵�����
*				len�����ݳ���
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void Usart_SendString(USART_TypeDef *usart_x, unsigned char *str, unsigned short len)
{

	if(USART_IsBusReady(usart_x) == USART_OK)
	{
#if(USART_DMA_TX_EN == 0)
		unsigned short count = 0;
		
		for(; count < len; count++)
		{
			USART_SendData(usart_x, *str++);								//��������
			while(USART_GetFlagStatus(usart_x, USART_FLAG_TC) == RESET);	//�ȴ��������
		}
#else
		unsigned int mAddr = (unsigned int)str;
		
		if(usart_x == USART1)
		{
			while(DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET);				//�ȴ�ͨ��4�������
			DMA_ClearFlag(DMA1_FLAG_TC4);									//���ͨ��4������ɱ�־
		}
		else if(usart_x == USART2)
		{
			while(DMA_GetFlagStatus(DMA1_FLAG_TC7) == RESET);				//�ȴ�ͨ��7�������
			DMA_ClearFlag(DMA1_FLAG_TC7);									//���ͨ��7������ɱ�־
		}
		
		USARTx_ResetMemoryBaseAddr(usart_x, mAddr, len, USART_TX_TYPE);
#endif
	
		USART_FreeBus(usart_x);
	}

}

/*
************************************************************
*	�������ƣ�	UsartPrintf
*
*	�������ܣ�	��ʽ����ӡ
*
*	��ڲ�����	usart_x��������
*				fmt����������
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void UsartPrintf(USART_TypeDef *usart_x, char *fmt, ...)
{

	if(USART_IsBusReady(usart_x) == USART_OK)
	{
		va_list ap;
	
#if(USART_DMA_TX_EN == 0)
		unsigned char usart_printbuf[256];
#endif
	
		unsigned char *str = usart_printbuf;
	
#if(USART_DMA_TX_EN == 1)
		if(usart_x == USART1)
		{
			while(DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET);					//�ȴ�ͨ��4�������
			DMA_ClearFlag(DMA1_FLAG_TC4);										//���ͨ��4������ɱ�־
		}
		else if(usart_x == USART2)
		{
			while(DMA_GetFlagStatus(DMA1_FLAG_TC7) == RESET);					//�ȴ�ͨ��7�������
			DMA_ClearFlag(DMA1_FLAG_TC7);										//���ͨ��7������ɱ�־
		}
		
		memset(usart_printbuf, 0, sizeof(usart_printbuf));						//���buffer
#endif
	
		va_start(ap, fmt);
		vsprintf((char *)usart_printbuf, fmt, ap);								//��ʽ��
		va_end(ap);
	
#if(USART_DMA_TX_EN == 1)
		USARTx_ResetMemoryBaseAddr(usart_x, (unsigned int)usart_printbuf,
								strlen((const char *)str), USART_TX_TYPE);
#else
		while(*str != 0)
		{
			USART_SendData(usart_x, *str++);
			while(USART_GetFlagStatus(usart_x, USART_FLAG_TC) == RESET);
		}
#endif
		
		USART_FreeBus(usart_x);
	}

}

void DebugPrintf(char *fmt, ...)
{

	if(USART_IsBusReady(USART_DEBUG) == USART_OK)
	{
		va_list ap;
	
#if(USART_DMA_TX_EN == 0)
		unsigned char usart_printbuf[256];
#endif
	
		unsigned char *str = usart_printbuf;
	
#if(USART_DMA_TX_EN == 1)
		if(USART_DEBUG == USART1)
		{
			while(DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET);					//�ȴ�ͨ��4�������
			DMA_ClearFlag(DMA1_FLAG_TC4);										//���ͨ��4������ɱ�־
		}
		else if(USART_DEBUG == USART2)
		{
			while(DMA_GetFlagStatus(DMA1_FLAG_TC7) == RESET);					//�ȴ�ͨ��7�������
			DMA_ClearFlag(DMA1_FLAG_TC7);										//���ͨ��7������ɱ�־
		}
		
		memset(usart_printbuf, 0, sizeof(usart_printbuf));						//���buffer
#endif
	
		va_start(ap, fmt);
		vsprintf((char *)usart_printbuf, fmt, ap);								//��ʽ��
		va_end(ap);
	
#if(USART_DMA_TX_EN == 1)
		USARTx_ResetMemoryBaseAddr(USART_DEBUG, (unsigned int)usart_printbuf,
								strlen((const char *)str), USART_TX_TYPE);
#else
		while(*str != 0)
		{
			USART_SendData(USART_DEBUG, *str++);
			while(USART_GetFlagStatus(USART_DEBUG, USART_FLAG_TC) == RESET);
		}
#endif
		
		USART_FreeBus(USART_DEBUG);
	}

}

/*
************************************************************
*	�������ƣ�	USART1_IRQHandler
*
*	�������ܣ�	����1�շ��ж�
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void USART1_IRQHandler(void)
{
	
	RTOS_EnterInt();

#if(USART_DMA_RX_EN == 0)
	if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET)		//�����ж�
	{
		alter_info.alter_count %= sizeof(alter_info.alter_buf);		//��ֹ���ڱ�ˢ��
		
        alter_info.alter_buf[alter_info.alter_count++] = USART1->DR;
		
		USART_ClearFlag(USART1, USART_FLAG_RXNE);
	}
#endif

	if(USART_GetFlagStatus(USART1, USART_FLAG_IDLE) != RESET)
	{
		alter_info.rev_idle = 1;
		alter_info.alter_count = 0;
		
		USART1->DR;													//��ȡ����ע�⣺������Ҫ�������ܹ�����жϱ�־λ
		USART_ClearFlag(USART1, USART_IT_IDLE);
		
#if(USART_DMA_RX_EN == 1)
		DMA_Cmd(DMA1_Channel5, DISABLE);
		
		DMA1_Channel5->CNDTR = sizeof(alter_info.alter_buf);		//���������´ν��յĳ��ȣ������޷������´�DMA����
		
		DMA_Cmd(DMA1_Channel5, ENABLE);
#endif
	}
	
	RTOS_ExitInt();

}
