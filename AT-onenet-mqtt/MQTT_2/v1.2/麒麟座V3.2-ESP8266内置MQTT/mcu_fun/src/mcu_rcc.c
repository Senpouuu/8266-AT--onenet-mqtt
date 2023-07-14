/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	mcu_fun.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2018-02-24
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		��Ƭ��Ƭ������ʱ�ӿ���
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"

//��Ƭ��������
#include "mcu_rcc.h"


/*
************************************************************
*	�������ƣ�	MCU_RCC
*
*	�������ܣ�	��Ƭ��Ƭ������ʱ�ӿ���
*
*	��ڲ�����	periph������
*				status��״̬
*
*	���ز�����	0-�ɹ�	1-ʧ��
*
*	˵����		
************************************************************
*/
_Bool MCU_RCC(void *periph, _Bool status)
{
	
	unsigned int periph_address = (unsigned int)periph;
	_Bool result = 1;
	
	if(periph_address >= APB1PERIPH_BASE && periph_address < APB2PERIPH_BASE)
	{
		RCC_APB1PeriphClockCmd(1 << ((periph_address - APB1PERIPH_BASE) >> 10), status ? ENABLE : DISABLE);
		
		result = 0;
	}
	else if(periph_address >= APB2PERIPH_BASE && periph_address < AHBPERIPH_BASE)
	{
		RCC_APB2PeriphClockCmd(1 << ((periph_address - APB2PERIPH_BASE) >> 10), status ? ENABLE : DISABLE);
		
		result = 0;
	}
	else if(periph_address >= AHBPERIPH_BASE)
	{
		RCC_AHBPeriphClockCmd(1 << ((periph_address - AHBPERIPH_BASE) >> 10), status ? ENABLE : DISABLE);
		
		result = 0;
	}

	return result;

}
