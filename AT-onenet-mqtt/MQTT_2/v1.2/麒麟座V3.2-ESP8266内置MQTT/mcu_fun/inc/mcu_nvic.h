#ifndef _MCU_NVIC_H_
#define _MCU_NVIC_H_


//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"


void MCU_NVIC_Init(unsigned char NVIC_IRQChannel, FunctionalState NVIC_IRQChannelCmd,
					unsigned char NVIC_IRQChannelPreemptionPriority, unsigned char NVIC_IRQChannelSubPriority);


#endif
