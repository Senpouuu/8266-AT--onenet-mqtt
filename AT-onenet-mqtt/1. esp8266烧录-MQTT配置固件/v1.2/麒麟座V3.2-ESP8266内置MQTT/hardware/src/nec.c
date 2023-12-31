/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	nec.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-09-12
	*
	*	版本： 		V1.0
	*
	*	说明： 		红外协议
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//硬件驱动
#include "ir.h"
#include "nec.h"
#include "delay.h"


/*
************************************************************
*	函数名称：	NEC_SendHead
*
*	函数功能：	发送头码
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void NEC_SendHead(void)
{

	IR_SendFreq(9000);
	
	IR_SendStop(4500);

}

/*
************************************************************
*	函数名称：	NEC_SendEnd
*
*	函数功能：	发送一个停止码
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void NEC_SendEnd(void)
{

	IR_SendFreq(560);
	
	IR_SendStop(0);

}

/*
************************************************************
*	函数名称：	NEC_SendOneZero
*
*	函数功能：	发送一个逻辑1或者0
*
*	入口参数：	logical：1还是0
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void NEC_SendOneZero(_Bool logical)
{

	if(logical)
	{
		IR_SendFreq(560);
		
		IR_SendStop(1680);
	}
	else
	{
		IR_SendFreq(560);
		
		IR_SendStop(560);
	}

}

/*
************************************************************
*	函数名称：	NEC_SendByte
*
*	函数功能：	发送一个字节
*
*	入口参数：	byte：发送的字节
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void NEC_SendByte(unsigned char byte)
{

	unsigned char i = 0;
	
	for(; i < 8; i++)
	{
		NEC_SendOneZero(byte & 0x80);
		
		byte <<= 1;
	}

}

/*
************************************************************
*	函数名称：	NEC_SendData
*
*	函数功能：	发送一个数据
*
*	入口参数：	addr：地址
*				data：数据
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void NEC_SendData(unsigned char addr, unsigned char data)
{
	
	RTOS_ENTER_CRITICAL();

	NEC_SendHead();
	
	NEC_SendByte(addr);
	NEC_SendByte(~addr);
	NEC_SendByte(data);
	NEC_SendByte(~data);
	
	NEC_SendEnd();
	
	RTOS_EXIT_CRITICAL();

}

/*
************************************************************
*	函数名称：	NEC_SendMultiData
*
*	函数功能：	发送多个数据
*
*	入口参数：	buf：数据缓存
*				num：数据长度
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void NEC_SendMultiData(unsigned char *buf, unsigned char num)
{

	unsigned char i = 0;
	
	RTOS_ENTER_CRITICAL();
	
	NEC_SendHead();
	
	for(; i < num; i++)
	{
		NEC_SendByte(buf[i]);
	}
	
	NEC_SendEnd();
	
	RTOS_EXIT_CRITICAL();

}
