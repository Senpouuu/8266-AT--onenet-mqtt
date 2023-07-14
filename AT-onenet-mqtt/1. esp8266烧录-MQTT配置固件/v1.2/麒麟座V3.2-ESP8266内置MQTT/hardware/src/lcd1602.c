/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	lcd1602.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2016-11-23
	*
	*	版本： 		V1.1
	*
	*	说明： 		LCD1602初始化，显示
	*
	*	修改记录：	V1.1：更改了EN脚接口
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f10x.h"

//单片机相关组件
#include "mcu_gpio.h"

//硬件驱动
#include "lcd1602.h"
#include "delay.h"

//C库
#include <stdarg.h>
#include <stdio.h>


static _Bool lcd1602_busy = 0;


const static GPIO_LIST lcd_gpio_list[] = {
											{GPIOC, GPIO_Pin_6, NULL},
											{GPIOA, GPIO_Pin_11, NULL},
											{GPIOB, GPIO_Pin_4, NULL},
										};


//数据、命令控制
#define RS_H	lcd_gpio_list[0].gpio_group->BSRR = lcd_gpio_list[0].gpio_pin
#define RS_L	lcd_gpio_list[0].gpio_group->BRR = lcd_gpio_list[0].gpio_pin

//读写控制
#define RW_H	lcd_gpio_list[1].gpio_group->BSRR = lcd_gpio_list[1].gpio_pin
#define RW_L	lcd_gpio_list[1].gpio_group->BRR = lcd_gpio_list[1].gpio_pin

//使能控制
#define EN_H	lcd_gpio_list[2].gpio_group->BSRR = lcd_gpio_list[2].gpio_pin
#define EN_L	lcd_gpio_list[2].gpio_group->BRR = lcd_gpio_list[2].gpio_pin


/*
************************************************************
*	函数名称：	LCD_Delay
*
*	函数功能：	延时
*
*	入口参数：	time：延时时间
*
*	返回参数：	无
*
*	说明：		基于当前延时时基
************************************************************
*/
__inline static void LCD_Delay(unsigned int time)
{

	RTOS_TimeDly(time);

}

/*
************************************************************
*	函数名称：	LCD1602_IsBusReady
*
*	函数功能：	查询总线是否就绪
*
*	入口参数：	无
*
*	返回参数：	0-就绪	1-未就绪
*
*	说明：		
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
*	函数名称：	LCD1602_FreeBus
*
*	函数功能：	释放总线
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
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
*	函数名称：	LCD1602_SendByte
*
*	函数功能：	向LCD1602写一个字节
*
*	入口参数：	byte：需要写入的数据
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
__inline static void LCD1602_SendByte(unsigned char byte)
{
	
	unsigned short value = 0;
	
	value = GPIO_ReadOutputData(GPIOB);					//读取GPIOB的数据
	value &= ~(0x001F << 5);							//清除bit5~8
	value |= ((unsigned short)byte & 0x001F) << 5;		//将要写入的数据取低5位并左移5位
	GPIO_Write(GPIOB, value);							//写入GPIOB
	
	value = GPIO_ReadOutputData(GPIOC);					//读取GPIOC的数据
	value &= ~(0x0007 << 0);							//清除bit0~2
	value |= ((unsigned short)byte & 0x00E0) >> 5;		//将要写入的数据取高3位并右移5位
	GPIO_Write(GPIOC, value);							//写入GPIOC
	
	DelayUs(10);

}

/*
************************************************************
*	函数名称：	LCD1602_WriteCom
*
*	函数功能：	向LCD1602写命令
*
*	入口参数：	byte：需要写入的命令
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
__inline static void LCD1602_WriteCom(unsigned char byte)
{

	RS_L;						//RS拉低，命令模式
	RW_L;						//RW拉低，写模式
	
	LCD1602_SendByte(byte);		//发送一个字节
	
	EN_H;
	DelayUs(20);
	EN_L;
	DelayUs(5);

}

/*
************************************************************
*	函数名称：	LCD1602_WriteCom_Busy
*
*	函数功能：	向LCD1602写命令
*
*	入口参数：	byte：需要写入的命令
*
*	返回参数：	无
*
*	说明：		
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
*	函数名称：	LCD1602_WriteData
*
*	函数功能：	向LCD1602写一个数据
*
*	入口参数：	byte：需要写入的数据
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
__inline static void LCD1602_WriteData(unsigned char byte)
{

	RS_H;						//RS拉高，数据模式
	RW_L;						//RW拉低，写模式
	
	LCD1602_SendByte(byte);		//发送一个字节

	EN_H;
	DelayUs(20);
	EN_L;
	DelayUs(5);

}

/*
************************************************************
*	函数名称：	LCD1602_Init
*
*	函数功能：	LCD1602初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		RW-PA11		RS-PC6		EN-PB4
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
*	函数名称：	LCD1602_Clear
*
*	函数功能：	LCD1602清除指定行
*
*	入口参数：	pos：指定的行
*
*	返回参数：	无
*
*	说明：		0x80-第一行		0xC0-第二行		0xFF-两行
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
*	函数名称：	LCD1602_DisString
*
*	函数功能：	设置LCD1602显示的内容
*
*	入口参数：	pos：要显示的行
*				fmt：不定长参
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void LCD1602_DisString(unsigned char pos, char *fmt,...)
{
	
	if(LCD1602_IsBusReady() == 0)
	{
		unsigned char lcd_printbuf[33];
		unsigned char count = 0;
		unsigned char remain = 0;					//一行剩余空间
		va_list ap;
		unsigned char *str = lcd_printbuf;
		
		va_start(ap,fmt);
		vsnprintf((char *)lcd_printbuf, sizeof(lcd_printbuf), fmt, ap);		//格式化
		va_end(ap);
		
		remain = 0x8f - pos;						//这里解决bug，以前只管写16次，这次要根据开始的位置来计算
		
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
