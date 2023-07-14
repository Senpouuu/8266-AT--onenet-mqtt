/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	oled.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2016-11-23
	*
	*	版本： 		V1.0
	*
	*	说明： 		OLED初始化，显示控制
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//硬件驱动
#include "oled.h"
#include "i2c.h"
#include "delay.h"

//字库
#include "font.h"

//C库
#include <stdarg.h>
#include <stdio.h>


OLED_INFO oled_info;


/*
************************************************************
*	函数名称：	OLED_Delay
*
*	函数功能：	OLED延时
*
*	入口参数：	time：延时时间
*
*	返回参数：	无
*
*	说明：		基于当前延时时基
************************************************************
*/
__inline static void OLED_Delay(unsigned int time)
{

	RTOS_TimeDly(time);

}

/*
************************************************************
*	函数名称：	OLED_WriteData
*
*	函数功能：	OLED写入一个数据
*
*	入口参数：	byte：需要写入的数据
*
*	返回参数：	写入结果
*
*	说明：		0-成功		1-失败
************************************************************
*/
_Bool OLED_WriteData(unsigned char byte)
{
	
	unsigned char addr = OLED_ADDRESS << 1;
	
	IIC_Start(OLED_IO);
	
#if(HW_I2C == 1)
	I2C_Send7bitAddress(OLED_IO, addr, I2C_Direction_Transmitter);
	IIC_CHECK_EVENT(OLED_IO, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);			//等待EV6
	IIC_CHECK_EVENT(OLED_IO, I2C_EVENT_MASTER_BYTE_TRANSMITTING);					//等待EV8
#else
	IIC_SendByte(OLED_IO, addr);
	if(IIC_WaitAck(OLED_IO, 5000))
	{
		IIC_Stop();
		return 1;
	}
#endif
	
	IIC_SendByte(OLED_IO, 0x40);
	if(IIC_WaitAck(OLED_IO, 5000))
	{
		IIC_Stop(OLED_IO);
		return 1;
	}
	
	IIC_SendByte(OLED_IO, byte);
	if(IIC_WaitAck(OLED_IO, 5000))
	{
		IIC_Stop(OLED_IO);
		return 1;
	}
	
	IIC_Stop(OLED_IO);
	
	return 0;

}

/*
************************************************************
*	函数名称：	OLED_WriteCom
*
*	函数功能：	OLED写入一个命令
*
*	入口参数：	cmd：需要写入的命令
*
*	返回参数：	写入结果
*
*	说明：		0-成功		1-失败
************************************************************
*/
_Bool OLED_WriteCom(unsigned char cmd)
{
	
	unsigned char addr = OLED_ADDRESS << 1;
	
	IIC_Start(OLED_IO);
	
#if(HW_I2C == 1)
	I2C_Send7bitAddress(OLED_IO, addr, I2C_Direction_Transmitter);
	IIC_CHECK_EVENT(OLED_IO, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);			//等待EV6
	IIC_CHECK_EVENT(OLED_IO, I2C_EVENT_MASTER_BYTE_TRANSMITTING);					//等待EV8
#else
	IIC_SendByte(OLED_IO, addr);
	if(IIC_WaitAck(OLED_IO, 5000))
	{
		IIC_Stop();
		return 1;
	}
#endif
	
	IIC_SendByte(OLED_IO, 0x00);
	if(IIC_WaitAck(OLED_IO, 5000))
	{
		IIC_Stop(OLED_IO);
		return 1;
	}
	
	IIC_SendByte(OLED_IO, cmd);
	if(IIC_WaitAck(OLED_IO, 5000))
	{
		IIC_Stop(OLED_IO);
		return 1;
	}
	
	IIC_Stop(OLED_IO);
	
	return 0;
	
}

/*
************************************************************
*	函数名称：	OLED_Init
*
*	函数功能：	OLED初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void OLED_Init(void)
{
	
#if 1
	OLED_WriteCom(0xAE); //关闭显示
	OLED_WriteCom(0x20); //Set Memory Addressing Mode	
	OLED_WriteCom(0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	OLED_WriteCom(0xb0); //Set Page Start Address for Page Addressing Mode,0-7
	OLED_WriteCom(0xa1); //0xa0，X轴正常显示；0xa1，X轴镜像显示
	OLED_WriteCom(0xc8); //0xc0，Y轴正常显示；0xc8，Y轴镜像显示
	OLED_WriteCom(0x00); //设置列地址低4位
	OLED_WriteCom(0x10); //设置列地址高4位
	OLED_WriteCom(0x40); //设置起始线地址
	OLED_WriteCom(0x81); //设置对比度值
	OLED_WriteCom(0x7f); //------
	OLED_WriteCom(0xa6); //0xa6,正常显示模式;0xa7，
	OLED_WriteCom(0xa8); //--set multiplex ratio(1 to 64)
	OLED_WriteCom(0x3F); //------
	OLED_WriteCom(0xa4); //0xa4,显示跟随RAM的改变而改变;0xa5,显示内容忽略RAM的内容
	OLED_WriteCom(0xd3); //设置显示偏移
	OLED_WriteCom(0x00); //------
	OLED_WriteCom(0xd5); //设置内部显示时钟频率
	OLED_WriteCom(0xf0); //------
	OLED_WriteCom(0xd9); //--set pre-charge period//
	OLED_WriteCom(0x22); //------
	OLED_WriteCom(0xda); //--set com pins hardware configuration//
	OLED_WriteCom(0x12); //------
	OLED_WriteCom(0xdb); //--set vcomh//
	OLED_WriteCom(0x20); //------
	OLED_WriteCom(0x8d); //--set DC-DC enable//
	OLED_WriteCom(0x14); //------
	OLED_WriteCom(0xaf); //打开显示
#else
	OLED_WriteCom(0xAE);   //display off
	OLED_WriteCom(0x00);	//Set Memory Addressing Mode	
	OLED_WriteCom(0x10);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	OLED_WriteCom(0x40);	//Set Page Start Address for Page Addressing Mode,0-7
	OLED_WriteCom(0xb0);	//Set COM Output Scan Direction
	OLED_WriteCom(0x81);//---set low column address
	OLED_WriteCom(0xff);//---set high column address
	OLED_WriteCom(0xa1);//--set start line address
	OLED_WriteCom(0xa6);//--set contrast control register
	OLED_WriteCom(0xa8);
	OLED_WriteCom(0x3f);//--set segment re-map 0 to 127
	OLED_WriteCom(0xad);//--set normal display
	OLED_WriteCom(0x8b);//--set multiplex ratio(1 to 64)
	OLED_WriteCom(0x33);//
	OLED_WriteCom(0xc8);//0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	OLED_WriteCom(0xd3);//-set display offset
	OLED_WriteCom(0x00);//-not offset
	OLED_WriteCom(0xd5);//--set display clock divide ratio/oscillator frequency
	OLED_WriteCom(0x80);//--set divide ratio
	OLED_WriteCom(0xd9);//--set pre-charge period
	OLED_WriteCom(0x1f); //
	OLED_WriteCom(0xda);//--set com pins hardware configuration
	OLED_WriteCom(0x12);
	OLED_WriteCom(0xdb);//--set vcomh
	OLED_WriteCom(0x40);//0x20,0.77xVcc
//	IIC_Write_Command(0x8d);//--set DC-DC enable
//	IIC_Write_Command(0x14);//
	OLED_WriteCom(0xaf);//--turn on oled panel
#endif

}

/*
************************************************************
*	函数名称：	OLED_Exist
*
*	函数功能：	OLED设备在线检查
*
*	入口参数：	无
*
*	返回参数：	0-未检测到	1-检测到
*
*	说明：		
************************************************************
*/
_Bool OLED_Exist(void)
{

	//检测OLED
	if(!OLED_WriteCom(0xAE))			//关闭显示，判断ACK
		oled_info.device_ok = 1;
	
	return oled_info.device_ok;

}

/*
************************************************************
*	函数名称：	OLED_Address
*
*	函数功能：	设置OLED显示地址
*
*	入口参数：	x：行地址
*				y：列地址
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void OLED_Address(unsigned char x, unsigned char y)
{

	OLED_WriteCom(0xb0 + x);					//设置行地址
	IIC_DelayUs(OLED_IO);
	OLED_WriteCom(((y & 0xf0) >> 4) | 0x10);	//设置列地址的高4位
	IIC_DelayUs(OLED_IO);
	OLED_WriteCom(y & 0x0f);					//设置列地址的低4位
	IIC_DelayUs(OLED_IO);
	
}

/*
************************************************************
*	函数名称：	OLED_ClearScreen
*
*	函数功能：	OLED全屏清除
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void OLED_ClearScreen(void)
{
	
	if(oled_info.device_ok)
	{
		unsigned char time_out = 200;
		
		while(IIC_IsBusReady(OLED_IO) == IIC_Err && --time_out)
			OLED_Delay(1);
		
		if(time_out)
		{
			unsigned char i = 0, j = 0;
			
			IIC_SpeedCtl(OLED_IO, 1);
			
			for(; i < 8; i++)
			{
				OLED_WriteCom(0xb0 + i);
				OLED_WriteCom(0x10);
				OLED_WriteCom(0x00);
				
				for(j = 0; j < 132; j++)
					OLED_WriteData(0x00);
			}
			
			IIC_FreeBus(OLED_IO);
		}
	}
	
}

/*
************************************************************
*	函数名称：	OLED_ClearAt
*
*	函数功能：	OLED清除指定行
*
*	入口参数：	x：需要清除的行
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void OLED_ClearAt(unsigned char x)
{
	
	if(oled_info.device_ok)
	{
		unsigned char time_out = 200;
		
		while(IIC_IsBusReady(OLED_IO) == IIC_Err && --time_out)
			OLED_Delay(1);
		
		if(time_out)
		{
			unsigned char i = 0;
			
			IIC_SpeedCtl(OLED_IO, 1);
			
			OLED_WriteCom(0xb0 + x);
			OLED_WriteCom(0x10);
			OLED_WriteCom(0x00);
				
			for(; i < 132; i++)
				OLED_WriteData(0x00);
			
			IIC_FreeBus(OLED_IO);
		}
	}

}

/*
************************************************************
*	函数名称：	OLED_Dis12864_Pic
*
*	函数功能：	显示一幅128*64的图片
*
*	入口参数：	dp：图片数据指针
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void OLED_Dis12864_Pic(const unsigned char *dp)
{
	
	if(oled_info.device_ok)
	{
		unsigned char time_out = 200;
		
		while(IIC_IsBusReady(OLED_IO) == IIC_Err && --time_out)
			OLED_Delay(1);
		
		if(time_out)
		{
			unsigned char i = 0, j = 0;
			
			IIC_SpeedCtl(OLED_IO, 1);
			
			for(; j < 8; j++)
			{
				OLED_Address(j, 0);
				
				for (i = 0; i < 128; i++)
					OLED_WriteData(*dp++);		//写数据到LCD,每写完一个8位的数据后列地址自动加1
			}
			
			IIC_FreeBus(OLED_IO);
		}
	}
	
}

/*
************************************************************
*	函数名称：	OLED_DisChar16x16
*
*	函数功能：	显示16x16的点阵数据
*
*	入口参数：	dp：图片数据指针
*
*	返回参数：	无
*
*	说明：		显示16x16点阵图像、汉字、生僻字或16x16点阵的其他图标
************************************************************
*/
void OLED_DisChar16x16(unsigned short x, unsigned short y, const unsigned char *dp)
{
	
	if(oled_info.device_ok)
	{
		unsigned char time_out = 200;
		
		while(IIC_IsBusReady(OLED_IO) == IIC_Err && --time_out)
			OLED_Delay(1);
		
		if(time_out)
		{
			unsigned char i = 0, j = 0;
			
			IIC_SpeedCtl(OLED_IO, 1);
			
			for(j = 2; j > 0; j--)
			{
				OLED_Address(x, y);
				
				for (i = 0; i < 16; i++)
					OLED_WriteData(*dp++);		//写数据到OLED,每写完一个8位的数据后列地址自动加1
				
				x++;
			}
			
			IIC_FreeBus(OLED_IO);
		}
	}
	
}

/*
************************************************************
*	函数名称：	OLED_DisString6x8
*
*	函数功能：	显示6x8的点阵数据
*
*	入口参数：	x：显示行
*				y：显示列
*				fmt：不定长参
*
*	返回参数：	无
*
*	说明：		能显示7行
************************************************************
*/
void OLED_DisString6x8(unsigned char x, unsigned char y, char *fmt, ...)
{
	
	if(oled_info.device_ok)
	{
		unsigned char time_out = 200;
		
		while(IIC_IsBusReady(OLED_IO) == IIC_Err && --time_out)
			OLED_Delay(1);
		
		if(time_out)
		{
			unsigned char oled_disbuf[128];
			unsigned char i = 0, ch = 0;
			va_list ap;
			unsigned char *str = oled_disbuf;
			
			IIC_SpeedCtl(OLED_IO, 1);
			
			va_start(ap,fmt);
			vsnprintf((char *)oled_disbuf, sizeof(oled_disbuf), fmt, ap);
			va_end(ap);
			
			y += 2;
			
			while(*str != '\0')
			{
				ch = *str - 32;
				
				if(y > 126)
				{
					y = 2;
					x++;
				}
				
				OLED_Address(x, y);
				for(i = 0; i < 6; i++)
					OLED_WriteData(F6x8[ch][i]);
				
				y += 6;
				str++;
			}
			
			IIC_FreeBus(OLED_IO);
		}
	}

}

/*
************************************************************
*	函数名称：	OLED_DisString8x16
*
*	函数功能：	显示8x16的点阵数据
*
*	入口参数：	x：显示行
*				y：显示列
*				fmt：不定长参
*
*	返回参数：	无
*
*	说明：		能显示4行
************************************************************
*/
void OLED_DisString8x16(unsigned char x, unsigned char y, char *fmt, ...)
{
	
	if(oled_info.device_ok)
	{
		unsigned char time_out = 200;
		
		while(IIC_IsBusReady(OLED_IO) == IIC_Err && --time_out)
			OLED_Delay(1);
		
		if(time_out)
		{
			unsigned char oled_disbuf[128];
			unsigned char i = 0, ch = 0;
			va_list ap;
			unsigned char *str = oled_disbuf;
			
			IIC_SpeedCtl(OLED_IO, 1);
			
			va_start(ap,fmt);
			vsnprintf((char *)oled_disbuf, sizeof(oled_disbuf), fmt, ap);
			va_end(ap);
			
			y += 2;
			
			while(*str != '\0')
			{
				ch = *str - 32;
				
				if(y > 128)
				{
					y = 2;
					x += 2;
				}
				
				OLED_Address(x, y);
				for(i = 0; i < 8; i++)
					OLED_WriteData(F8X16[(ch << 4) + i]);
				
				OLED_Address(x + 1, y);
				for(i = 0; i < 8; i++)
					OLED_WriteData(F8X16[(ch << 4) + i + 8]);
				
#if(HW_I2C == 1)
				IIC_DelayUs(OLED_IO);
#endif
				
				y += 8;
				str++;
			}
			
			IIC_FreeBus(OLED_IO);
		}
	}

}
