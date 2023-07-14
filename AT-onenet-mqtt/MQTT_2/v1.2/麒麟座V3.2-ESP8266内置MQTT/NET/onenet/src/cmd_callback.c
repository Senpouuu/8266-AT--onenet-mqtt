/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	cmd_callback.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2018-01-23
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		�����·��Ĳ�ѯ��ִ�лص�
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//
#include "cmd_callback.h"

//��Ƭ��������
#include "mcu_gpio.h"

//C��
#include <stdlib.h>

//Ӳ������
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
//	�������ƣ�	CALLBACK_Find_CallBack
//
//	�������ܣ�	���������ҵ��ص�����
//
//	��ڲ�����	cmd������
//
//	���ز�����	�ص�������	255-δ�ҵ�
//
//	˵����		�����ʽ��XXX:YYY
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
//	�������ƣ�	CALLBACK_Execute
//
//	�������ܣ�	����ִ��
//
//	��ڲ�����	cmd������
//
//	���ز�����	0-ִ�гɹ�	1-ִ��ʧ��
//
//	˵����		�����ʽ��XXX:YYY
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
//	�������ƣ�	CALLBACK_Find_Value
//
//	�������ܣ�	�ҵ��·�ָ��Ĳ���ֵ
//
//	��ڲ�����	cmd������
//
//	���ز�����	����ָ��
//
//	˵����		�����ʽ�������ʽ��XXX:YYY
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
//	�������ƣ�	CALLBACK_Str2Dec
//
//	�������ܣ�	���ַ������͵�����תΪ��ֵ
//
//	��ڲ�����	str��
//
//	���ز�����	ֵ
//
//	˵����		
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
//	�������ƣ�	CALLBACK_Redled
//
//	�������ܣ�	����ִ�лص�
//
//	��ڲ�����	cmd���·�������
//
//	���ز�����	��
//
//	˵����		
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
//	�������ƣ�	CALLBACK_GreenLed
//
//	�������ܣ�	����ִ�лص�
//
//	��ڲ�����	cmd���·�������
//
//	���ز�����	��
//
//	˵����		
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
//	�������ƣ�	CALLBACK_YellowLed
//
//	�������ܣ�	����ִ�лص�
//
//	��ڲ�����	cmd���·�������
//
//	���ز�����	��
//
//	˵����		
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
//	�������ƣ�	CALLBACK_BlueLed
//
//	�������ܣ�	����ִ�лص�
//
//	��ڲ�����	cmd���·�������
//
//	���ز�����	��
//
//	˵����		
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
//	�������ƣ�	CALLBACK_Beep
//
//	�������ܣ�	����ִ�лص�
//
//	��ڲ�����	cmd���·�������
//
//	���ز�����	��
//
//	˵����		
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
//	�������ƣ�	CALLBACK_SpiLcd
//
//	�������ܣ�	����ִ�лص�
//
//	��ڲ�����	cmd���·�������
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void CALLBACK_SpiLcd(char *cmd)
{

	int value = CALLBACK_Str2Dec(CALLBACK_Find_Value(cmd));
	
	SPILCD_BL_Ctl(value);

}
