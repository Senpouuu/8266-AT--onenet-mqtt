/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	cmd_callback.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2018-01-23
	*
	*	版本： 		V1.0
	*
	*	说明： 		命令下发的查询和执行回调
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//
#include "cmd_callback.h"

//单片机相关组件
#include "mcu_gpio.h"

//C库
#include <stdlib.h>

//硬件驱动
#include "usart.h"
#include "led.h"
#include "beep.h"
#include "spilcd.h"


#define CB_NULL		(void *)0


void CALLBACK_RedLed(char *cmd);

void CALLBACK_GreenLed(char *cmd);

void CALLBACK_YellowLed(char *cmd);

void CALLBACK_BlueLed(char *cmd);

void CALLBACK_Beep(char *cmd);

void CALLBACK_SpiLcd(char *cmd);


CMD_CALLBACK_INFO cmd_callback_info[] = {
											{"redled", CALLBACK_RedLed},
											{"greenled", CALLBACK_GreenLed},
											{"yellowled", CALLBACK_YellowLed},
											{"blueled", CALLBACK_BlueLed},
											{"beep", CALLBACK_Beep},
											{"lcdbl", CALLBACK_SpiLcd},
										};
unsigned char cmd_cb_cnt = sizeof(cmd_callback_info) / sizeof(cmd_callback_info[0]);


//==========================================================
//	函数名称：	CALLBACK_Find_CallBack
//
//	函数功能：	根据命令找到回调函数
//
//	入口参数：	cmd：命令
//
//	返回参数：	回调函数号	255-未找到
//
//	说明：		命令格式：XXX:YYY
//==========================================================
unsigned char CALLBACK_Find_CallBack(char *cmd)
{
	
	unsigned char i = 0;
	char *name_ptr = cmd_callback_info[0].cmd_name;
	char *cmd_ptr = cmd;
	
	for(; i < cmd_cb_cnt;)
	{
		while(1)
		{
			if(*name_ptr == '\0' && *cmd_ptr == ':')
				return i;
			
			if(*name_ptr++ != *cmd_ptr++)
			{
				name_ptr = cmd_callback_info[++i].cmd_name;
				cmd_ptr = cmd;
				
				break;
			}
		}
	}
	
	return 255;

}

//==========================================================
//	函数名称：	CALLBACK_Execute
//
//	函数功能：	命令执行
//
//	入口参数：	cmd：命令
//
//	返回参数：	0-执行成功	1-执行失败
//
//	说明：		命令格式：XXX:YYY
//==========================================================
_Bool CALLBACK_Execute(char *cmd)
{
	
	unsigned char index = 255;

	index = CALLBACK_Find_CallBack(cmd);
	if((index < cmd_cb_cnt) && cmd_callback_info[index].call_back)
	{
		cmd_callback_info[index].call_back(cmd);
		
		return 0;
	}
	else
		return 1;

}

//==========================================================
//	函数名称：	CALLBACK_Find_Value
//
//	函数功能：	找到下发指令的参数值
//
//	入口参数：	cmd：命令
//
//	返回参数：	参数指针
//
//	说明：		命令格式：命令格式：XXX:YYY
//==========================================================
char *CALLBACK_Find_Value(char *cmd)
{

	while(*cmd != ':')
	{
		if(*cmd++ == '\0')
			return CB_NULL;
	}
	
	return ++cmd;

}

//==========================================================
//	函数名称：	CALLBACK_Str2Dec
//
//	函数功能：	将字符串类型的数字转为数值
//
//	入口参数：	str：
//
//	返回参数：	值
//
//	说明：		
//==========================================================
int CALLBACK_Str2Dec(char *str)
{

	char *check_str = str;
	_Bool flag = 0;
	
	if(str == CB_NULL)
		return 0;
	
	do
	{
		
		if(*check_str < '0' || *check_str > '9')
		{
			if(flag)
				return atoi(str);
			else
				return 0;
		}
		else
			flag = 1;
		
	} while(*(++check_str) != '\0');
	
	return atoi(str);

}








//==========================================================
//	函数名称：	CALLBACK_Redled
//
//	函数功能：	命令执行回调
//
//	入口参数：	cmd：下发的命令
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void CALLBACK_RedLed(char *cmd)
{
	
	int value = CALLBACK_Str2Dec(CALLBACK_Find_Value(cmd));
	
	switch(value)
	{
		case 0:	LED_Ctl(0, LED_OFF);	break;
		case 1:	LED_Ctl(0, LED_ON);		break;
		default:						break;
	}

}

//==========================================================
//	函数名称：	CALLBACK_GreenLed
//
//	函数功能：	命令执行回调
//
//	入口参数：	cmd：下发的命令
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void CALLBACK_GreenLed(char *cmd)
{

	int value = CALLBACK_Str2Dec(CALLBACK_Find_Value(cmd));
	
	switch(value)
	{
		case 0:	LED_Ctl(1, LED_OFF);	break;
		case 1:	LED_Ctl(1, LED_ON);		break;
		default:						break;
	}

}

//==========================================================
//	函数名称：	CALLBACK_YellowLed
//
//	函数功能：	命令执行回调
//
//	入口参数：	cmd：下发的命令
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void CALLBACK_YellowLed(char *cmd)
{

	int value = CALLBACK_Str2Dec(CALLBACK_Find_Value(cmd));
	
	switch(value)
	{
		case 0:	LED_Ctl(2, LED_OFF);	break;
		case 1:	LED_Ctl(2, LED_ON);		break;
		default:						break;
	}

}

//==========================================================
//	函数名称：	CALLBACK_BlueLed
//
//	函数功能：	命令执行回调
//
//	入口参数：	cmd：下发的命令
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void CALLBACK_BlueLed(char *cmd)
{

	int value = CALLBACK_Str2Dec(CALLBACK_Find_Value(cmd));
	
	switch(value)
	{
		case 0:	LED_Ctl(3, LED_OFF);	break;
		case 1:	LED_Ctl(3, LED_ON);		break;
		default:						break;
	}

}

//==========================================================
//	函数名称：	CALLBACK_Beep
//
//	函数功能：	命令执行回调
//
//	入口参数：	cmd：下发的命令
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void CALLBACK_Beep(char *cmd)
{
	
	int value = CALLBACK_Str2Dec(CALLBACK_Find_Value(cmd));
	
	switch(value)
	{
		case 0:	BEEP_Ctl(0, BEEP_OFF);	break;
		case 1:	BEEP_Ctl(0, BEEP_ON);	break;
		default:						break;
	}

}

//==========================================================
//	函数名称：	CALLBACK_SpiLcd
//
//	函数功能：	命令执行回调
//
//	入口参数：	cmd：下发的命令
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void CALLBACK_SpiLcd(char *cmd)
{

	int value = CALLBACK_Str2Dec(CALLBACK_Find_Value(cmd));
	
	SPILCD_BL_Ctl(value);

}
