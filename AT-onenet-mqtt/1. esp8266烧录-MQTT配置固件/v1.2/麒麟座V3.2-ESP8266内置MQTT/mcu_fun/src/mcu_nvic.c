/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	mcu_nvic.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2018-02-25
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		��Ƭ��NVIC
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��������
#include "mcu_nvic.h"


/*
************************************************************
*	�������ƣ�	MCU_NVIC_Init
*
*	�������ܣ�	�ж�����
*
*	��ڲ�����	NVIC_IRQChannel��ͨ��
*				NVIC_IRQChannelCmd��ENABLE or DISABLE
*				NVIC_IRQChannelPreemptionPriority����ռ�ж����ȼ�
*				NVIC_IRQChannelSubPriority���ж������ȼ�
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void MCU_NVIC_Init(unsigned char NVIC_IRQChannel, FunctionalState NVIC_IRQChannelCmd,
					unsigned char NVIC_IRQChannelPreemptionPriority, unsigned char NVIC_IRQChannelSubPriority)
{

	NVIC_InitTypeDef nvic_initstruct;
	
	nvic_initstruct.NVIC_IRQChannel = NVIC_IRQChannel;
	nvic_initstruct.NVIC_IRQChannelCmd = NVIC_IRQChannelCmd;
	nvic_initstruct.NVIC_IRQChannelPreemptionPriority = NVIC_IRQChannelPreemptionPriority;
	nvic_initstruct.NVIC_IRQChannelSubPriority = NVIC_IRQChannelSubPriority;
	
	NVIC_Init(&nvic_initstruct);

}
