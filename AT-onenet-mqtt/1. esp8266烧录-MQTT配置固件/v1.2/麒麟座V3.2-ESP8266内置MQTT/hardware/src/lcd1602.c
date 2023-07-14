/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	lcd1602.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2016-11-23
	*
	*	�汾�� 		V1.1
	*
	*	˵���� 		LCD1602��ʼ������ʾ
	*
	*	�޸ļ�¼��	V1.1��������EN�Žӿ�
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"

//��Ƭ��������
#include "mcu_gpio.h"

//Ӳ������
#include "lcd1602.h"
#include "delay.h"

//C��
#include <stdarg.h>
#include <stdio.h>


static _Bool lcd1602_busy = 0;


const static GPIO_LIST lcd_gpio_list[] = {
											{GPIOC, GPIO_Pin_6, NULL},
											{GPIOA, GPIO_Pin_11, NULL},
											{GPIOB, GPIO_Pin_4, NULL},
										};


//���ݡ��������
#define RS_H	lcd_gpio_list[0].gpio_group->BSRR = lcd_gpio_list[0].gpio_pin
#define RS_L	lcd_gpio_list[0].gpio_group->BRR = lcd_gpio_list[0].gpio_pin

//��д����
#define RW_H	lcd_gpio_list[1].gpio_group->BSRR = lcd_gpio_list[1].gpio_pin
#define RW_L	lcd_gpio_list[1].gpio_group->BRR = lcd_gpio_list[1].gpio_pin

//ʹ�ܿ���
#define EN_H	lcd_gpio_list[2].gpio_group->BSRR = lcd_gpio_list[2].gpio_pin
#define EN_L	lcd_gpio_list[2].gpio_group->BRR = lcd_gpio_list[2].gpio_pin


/*
************************************************************
*	�������ƣ�	LCD_Delay
*
*	�������ܣ�	��ʱ
*
*	��ڲ�����	time����ʱʱ��
*
*	���ز�����	��
*
*	˵����		���ڵ�ǰ��ʱʱ��
************************************************************
*/
__inline static void LCD_Delay(unsigned int time)
{

	RTOS_TimeDly(time);

}

/*
************************************************************
*	�������ƣ�	LCD1602_IsBusReady
*
*	�������ܣ�	��ѯ�����Ƿ����
*
*	��ڲ�����	��
*
*	���ز�����	0-����	1-δ����
*
*	˵����		
************************************************************
*/
__inline static _Bool LCD1602_IsBusReady(void)
{
	
	_Bool result = 1;

	RTOS_ENTER_CRITICAL();
	
	if(lcd1602_busy == 0)
	{
		lcd1602_busy = 1;
		
		result = 0;
	}
	
	RTOS_EXIT_CRITICAL();
	
	return result;

}

/*
************************************************************
*	�������ƣ�	LCD1602_FreeBus
*
*	�������ܣ�	�ͷ�����
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
__inline static void LCD1602_FreeBus(void)
{

	RTOS_ENTER_CRITICAL();
	
	lcd1602_busy = 0;
	
	RTOS_EXIT_CRITICAL();

}

/*
************************************************************
*	�������ƣ�	LCD1602_SendByte
*
*	�������ܣ�	��LCD1602дһ���ֽ�
*
*	��ڲ�����	byte����Ҫд�������
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
__inline static void LCD1602_SendByte(unsigned char byte)
{
	
	unsigned short value = 0;
	
	value = GPIO_ReadOutputData(GPIOB);					//��ȡGPIOB������
	value &= ~(0x001F << 5);							//���bit5~8
	value |= ((unsigned short)byte & 0x001F) << 5;		//��Ҫд�������ȡ��5λ������5λ
	GPIO_Write(GPIOB, value);							//д��GPIOB
	
	value = GPIO_ReadOutputData(GPIOC);					//��ȡGPIOC������
	value &= ~(0x0007 << 0);							//���bit0~2
	value |= ((unsigned short)byte & 0x00E0) >> 5;		//��Ҫд�������ȡ��3λ������5λ
	GPIO_Write(GPIOC, value);							//д��GPIOC
	
	DelayUs(10);

}

/*
************************************************************
*	�������ƣ�	LCD1602_WriteCom
*
*	�������ܣ�	��LCD1602д����
*
*	��ڲ�����	byte����Ҫд�������
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
__inline static void LCD1602_WriteCom(unsigned char byte)
{

	RS_L;						//RS���ͣ�����ģʽ
	RW_L;						//RW���ͣ�дģʽ
	
	LCD1602_SendByte(byte);		//����һ���ֽ�
	
	EN_H;
	DelayUs(20);
	EN_L;
	DelayUs(5);

}

/*
************************************************************
*	�������ƣ�	LCD1602_WriteCom_Busy
*
*	�������ܣ�	��LCD1602д����
*
*	��ڲ�����	byte����Ҫд�������
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
__inline static void LCD1602_WriteCom_Busy(unsigned char byte)
{
	
	LCD_Delay(2);

	RS_L;
	RW_L;
	
	LCD1602_SendByte(byte);
	
	EN_H;
	DelayUs(20);
	EN_L;
	DelayUs(5);

}

/*
************************************************************
*	�������ƣ�	LCD1602_WriteData
*
*	�������ܣ�	��LCD1602дһ������
*
*	��ڲ�����	byte����Ҫд�������
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
__inline static void LCD1602_WriteData(unsigned char byte)
{

	RS_H;						//RS���ߣ�����ģʽ
	RW_L;						//RW���ͣ�дģʽ
	
	LCD1602_SendByte(byte);		//����һ���ֽ�

	EN_H;
	DelayUs(20);
	EN_L;
	DelayUs(5);

}

/*
************************************************************
*	�������ƣ�	LCD1602_Init
*
*	�������ܣ�	LCD1602��ʼ��
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		RW-PA11		RS-PC6		EN-PB4
*				DATA0~4-PB5~9			DATA5~7-PC0~2
************************************************************
*/
void LCD1602_Init(void)
{

	GPIO_InitTypeDef gpio_initstruct;
	unsigned char i = 0;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
	
	MCU_GPIO_SWJ_JTAGDisable();
	
	for(; i < 3; i++)
		MCU_GPIO_Init(lcd_gpio_list[i].gpio_group, lcd_gpio_list[i].gpio_pin, GPIO_Mode_Out_PP, GPIO_Speed_50MHz, lcd_gpio_list[i].gpio_name);

	gpio_initstruct.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio_initstruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
	gpio_initstruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpio_initstruct);

	gpio_initstruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
	GPIO_Init(GPIOC, &gpio_initstruct);

	LCD_Delay(3);
	LCD1602_WriteCom(0x38);
	LCD_Delay(1);
	LCD1602_WriteCom(0x38);
	LCD_Delay(1);
	LCD1602_WriteCom(0x38);
	LCD1602_WriteCom_Busy(0x38);
	LCD1602_WriteCom_Busy(0x08);
	LCD1602_WriteCom_Busy(0x01);
	LCD1602_WriteCom_Busy(0x06);
	LCD1602_WriteCom_Busy(0x0c);
	
    EN_L;

}

/*
************************************************************
*	�������ƣ�	LCD1602_Clear
*
*	�������ܣ�	LCD1602���ָ����
*
*	��ڲ�����	pos��ָ������
*
*	���ز�����	��
*
*	˵����		0x80-��һ��		0xC0-�ڶ���		0xFF-����
************************************************************
*/
void LCD1602_Clear(unsigned char pos)
{

	switch(pos)
	{
		case 0x80:
			
			LCD1602_DisString(0x80, "                ");
		
		break;
		
		case 0xC0:
			
			LCD1602_DisString(0xC0, "                ");
		
		break;
		
		case 0xFF:
			
			LCD1602_WriteCom_Busy(0x01);
		
		break;
	}

}

/*
************************************************************
*	�������ƣ�	LCD1602_DisString
*
*	�������ܣ�	����LCD1602��ʾ������
*
*	��ڲ�����	pos��Ҫ��ʾ����
*				fmt����������
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void LCD1602_DisString(unsigned char pos, char *fmt,...)
{
	
	if(LCD1602_IsBusReady() == 0)
	{
		unsigned char lcd_printbuf[33];
		unsigned char count = 0;
		unsigned char remain = 0;					//һ��ʣ��ռ�
		va_list ap;
		unsigned char *str = lcd_printbuf;
		
		va_start(ap,fmt);
		vsnprintf((char *)lcd_printbuf, sizeof(lcd_printbuf), fmt, ap);		//��ʽ��
		va_end(ap);
		
		remain = 0x8f - pos;						//������bug����ǰֻ��д16�Σ����Ҫ���ݿ�ʼ��λ��������
		
		LCD1602_WriteCom_Busy(pos);
		
		while(*str != 0)
		{
			LCD1602_WriteData(*str++);
			
			if(++count > remain && pos <= 0x8f)
			{
				count = 0;
				LCD1602_WriteCom_Busy(0xC0);
				LCD_Delay(1);
			}
		}
		
		LCD1602_FreeBus();
	}

}
