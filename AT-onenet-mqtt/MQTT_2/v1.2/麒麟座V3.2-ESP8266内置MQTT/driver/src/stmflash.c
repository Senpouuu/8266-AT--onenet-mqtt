/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	stmflash.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2017-11-23
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		STM32F1xx��Ƭ��Ƭ��flash����
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"

//Ӳ������
#include "stmflash.h"


#define STM32F1_FLASH_BASE		0x08000000	//STM32 FLASH����ʼ��ַ
#define STM32F1_2048_BYTE		2048		//ҳƫ��
#define STM32F1_1024_BYTE		1024		//ҳƫ��


/*
************************************************************
*	�������ƣ�	FLASH_GetStartAddr
*
*	�������ܣ�	����ҳ��ȡ��ַ
*
*	��ڲ�����	page����ַ(������2�ı���)
*				flash_type��flash�����ͣ�0-1KB	1-2KB
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
unsigned int FLASH_GetStartAddr(unsigned char page, _Bool flash_type)
{
	
	if(flash_type)
		return ((unsigned int)page * STM32F1_2048_BYTE) + STM32F1_FLASH_BASE;
	else
		return ((unsigned int)page * STM32F1_1024_BYTE) + STM32F1_FLASH_BASE;

}

/*
************************************************************
*	�������ƣ�	FLASH_Erase
*
*	�������ܣ�	STM32F10x��Ƭ���ڲ�Flash����
*
*	��ڲ�����	page��Ҫ��������ʼҳ��(���0~255)
*				num����Ҫ��������ҳ
*				flash_type��flash�����ͣ�0-1KB	1-2KB
*
*	���ز�����	��
*
*	˵����		1.����STM32F10x��Ƭ���ڲ�Flash��ʼ��ַ����0x0800 0000
				2.С��������ÿҳΪ1KB���ֱ���32ҳ��64ҳ��128ҳ
				3.������ÿҳ2KB����256ҳ
				4.������ÿҳ2KB����128ҳ
************************************************************
*/
void FLASH_Erase(unsigned char page, unsigned char num, _Bool flash_type)
{
	
	unsigned int addr = FLASH_GetStartAddr(page, flash_type);
	unsigned char i = 0;
	
	FLASH_Unlock();														//����
	
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

	if(flash_type)
	{
		for(; i < num;)
		{
			if(FLASH_ErasePage(addr) == FLASH_COMPLETE)
			{
				i++;
				addr += STM32F1_2048_BYTE;
			}
		}
	}
	else
	{
		for(; i < num;)
		{
			if(FLASH_ErasePage(addr) == FLASH_COMPLETE)
			{
				i++;
				addr += STM32F1_1024_BYTE;
			}
		}
	}
	
	FLASH_Lock();														//����

}

/*
************************************************************
*	�������ƣ�	Flash_Read
*
*	�������ܣ�	STM32F10x��Ƭ���ڲ�Flash��ȡ
*
*	��ڲ�����	page��Ҫ��������ʼҳ��(���0~255)
*				rBuf������(����)
*				len����ȡ�ĳ���
*				flash_type��flash�����ͣ�0-1KB	1-2KB
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void Flash_Read(unsigned char page, unsigned short *rBuf, unsigned short len, _Bool flash_type)
{

	unsigned short lenCount = 0;
	unsigned int addr = FLASH_GetStartAddr(page, flash_type);
	
	for(; lenCount < len; lenCount++)
	{
		rBuf[lenCount] = *(volatile unsigned short *)addr;
		
		addr += 2;											//����ĵ�ַ������2�ı���
	}

}

/*
************************************************************
*	�������ƣ�	Flash_Write
*
*	�������ܣ�	STM32F10x��Ƭ���ڲ�Flashд
*
*	��ڲ�����	addr����ַ(������2�ı���)
*				wBuf������(����)
*				len��д��ĳ���
*				flash_type��flash�����ͣ�0-1KB	1-2KB
*
*	���ز�����	��
*
*	˵����		д�Ƚϸ��ӵ㣬ע�����㣺
*					1.����
*					2.ֻ�е�ַ��ߵ�ֵΪ0xFFFFʱ�ܹ���д����������
*					3.д������
************************************************************
*/
void Flash_Write(unsigned char page, unsigned short *wBuf, unsigned short len, _Bool flash_type)
{
	
	unsigned short lenCount = 0;
	
	unsigned int addr = FLASH_GetStartAddr(page, flash_type);
	
	FLASH_Unlock();														//����
	
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	
	for(; lenCount < len; lenCount++)
	{
		FLASH_ProgramHalfWord(addr, wBuf[lenCount]);					//д��

		addr += 2;														//��ַ������2�ı���
	}
	
	FLASH_Lock();														//����

}

