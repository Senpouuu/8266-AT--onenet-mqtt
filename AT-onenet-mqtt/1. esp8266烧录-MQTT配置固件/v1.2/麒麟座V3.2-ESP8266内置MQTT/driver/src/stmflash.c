/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	stmflash.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-11-23
	*
	*	版本： 		V1.0
	*
	*	说明： 		STM32F1xx单片机片内flash驱动
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f10x.h"

//硬件驱动
#include "stmflash.h"


#define STM32F1_FLASH_BASE		0x08000000	//STM32 FLASH的起始地址
#define STM32F1_2048_BYTE		2048		//页偏移
#define STM32F1_1024_BYTE		1024		//页偏移


/*
************************************************************
*	函数名称：	FLASH_GetStartAddr
*
*	函数功能：	根据页获取地址
*
*	入口参数：	page：地址(必须是2的倍数)
*				flash_type：flash的类型，0-1KB	1-2KB
*
*	返回参数：	无
*
*	说明：		
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
*	函数名称：	FLASH_Erase
*
*	函数功能：	STM32F10x单片机内部Flash擦除
*
*	入口参数：	page：要擦除的起始页码(最大0~255)
*				num：需要擦除多少页
*				flash_type：flash的类型，0-1KB	1-2KB
*
*	返回参数：	无
*
*	说明：		1.所有STM32F10x单片机内部Flash起始地址都是0x0800 0000
				2.小、中容量每页为1KB，分别有32页、64页、128页
				3.大容量每页2KB，共256页
				4.互联型每页2KB，共128页
************************************************************
*/
void FLASH_Erase(unsigned char page, unsigned char num, _Bool flash_type)
{
	
	unsigned int addr = FLASH_GetStartAddr(page, flash_type);
	unsigned char i = 0;
	
	FLASH_Unlock();														//解锁
	
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
	
	FLASH_Lock();														//上锁

}

/*
************************************************************
*	函数名称：	Flash_Read
*
*	函数功能：	STM32F10x单片机内部Flash读取
*
*	入口参数：	page：要擦除的起始页码(最大0~255)
*				rBuf：缓存(半字)
*				len：读取的长度
*				flash_type：flash的类型，0-1KB	1-2KB
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void Flash_Read(unsigned char page, unsigned short *rBuf, unsigned short len, _Bool flash_type)
{

	unsigned short lenCount = 0;
	unsigned int addr = FLASH_GetStartAddr(page, flash_type);
	
	for(; lenCount < len; lenCount++)
	{
		rBuf[lenCount] = *(volatile unsigned short *)addr;
		
		addr += 2;											//传入的地址必须是2的倍数
	}

}

/*
************************************************************
*	函数名称：	Flash_Write
*
*	函数功能：	STM32F10x单片机内部Flash写
*
*	入口参数：	addr：地址(必须是2的倍数)
*				wBuf：缓存(半字)
*				len：写入的长度
*				flash_type：flash的类型，0-1KB	1-2KB
*
*	返回参数：	无
*
*	说明：		写比较复杂点，注意三点：
*					1.解锁
*					2.只有地址里边的值为0xFFFF时能够被写入其他数据
*					3.写完上锁
************************************************************
*/
void Flash_Write(unsigned char page, unsigned short *wBuf, unsigned short len, _Bool flash_type)
{
	
	unsigned short lenCount = 0;
	
	unsigned int addr = FLASH_GetStartAddr(page, flash_type);
	
	FLASH_Unlock();														//解锁
	
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	
	for(; lenCount < len; lenCount++)
	{
		FLASH_ProgramHalfWord(addr, wBuf[lenCount]);					//写入

		addr += 2;														//地址必须是2的倍数
	}
	
	FLASH_Lock();														//上锁

}

