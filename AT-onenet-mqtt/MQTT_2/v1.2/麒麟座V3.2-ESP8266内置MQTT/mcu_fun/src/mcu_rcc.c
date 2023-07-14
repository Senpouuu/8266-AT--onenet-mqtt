/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	mcu_fun.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2018-02-24
	*
	*	版本： 		V1.0
	*
	*	说明： 		单片机片内外设时钟开关
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f10x.h"

//单片机相关组件
#include "mcu_rcc.h"


/*
************************************************************
*	函数名称：	MCU_RCC
*
*	函数功能：	单片机片内外设时钟开关
*
*	入口参数：	periph：外设
*				status：状态
*
*	返回参数：	0-成功	1-失败
*
*	说明：		
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
