/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	AT24C02.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2018-08-16
	*
	*	版本： 		V1.2
	*
	*	说明： 		EEPROM
	*
	*	修改记录：	V1.1：修复有时数据写入错误的bug
	*				V1.2：修改写、清除失败的处理
	************************************************************
	************************************************************
	************************************************************
**/

//硬件驱动
#include "at24c02.h"
#include "i2c.h"
#include "delay.h"


EEPROM_INFO eeprom_info;


/*
************************************************************
*	函数名称：	AT24C02_Delay
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
__inline static void AT24C02_Delay(unsigned int time)
{

	RTOS_TimeDly(time);

}

/*
************************************************************
*	函数名称：	AT24C02_Exist
*
*	函数功能：	EEPROM设备在线检查
*
*	入口参数：	无
*
*	返回参数：	0-未检测到	1-检测到
*
*	说明：		
************************************************************
*/
_Bool AT24C02_Exist(void)
{
	
	unsigned char value = 0;

	//检测EEPROM
	if(!I2C_ReadByte(AT24C02_IO, AT24C02_ADDRESS, 255, &value))
		eeprom_info.device_ok = 1;
	
	return eeprom_info.device_ok;

}

/*
************************************************************
*	函数名称：	AT24C02_WriteByte
*
*	函数功能：	写一个字节到EEPROM
*
*	入口参数：	reg_addr：寄存器地址
*				byte：需要写入的数据
*
*	返回参数：	0-成功	1-失败
*
*	说明：		
************************************************************
*/
_Bool AT24C02_WriteByte(unsigned char reg_addr, unsigned char byte)
{
	
	_Bool result = IIC_Err;

	if(eeprom_info.device_ok)
	{
		while(IIC_IsBusReady(AT24C02_IO) == IIC_Err)				//判断总线
			AT24C02_Delay(1);
		
		result = I2C_WriteByte(AT24C02_IO, AT24C02_ADDRESS, reg_addr, &byte);
		
		IIC_FreeBus(AT24C02_IO);
	}
	
	return result;

}

/*
************************************************************
*	函数名称：	AT24C02_WriteBytes
*
*	函数功能：	写多个字节到EEPROM
*
*	入口参数：	reg_addr：寄存器地址
*				byte：需要写入的数据缓存区
*				len：数据长度
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
_Bool AT24C02_WriteBytes(unsigned char reg_addr, unsigned char *byte, unsigned char len)
{
	
	_Bool result = IIC_Err;

	if(eeprom_info.device_ok)
	{
		unsigned char count = 0, err_count = 0;
		
		while(IIC_IsBusReady(AT24C02_IO) == IIC_Err)			//判断总线
			AT24C02_Delay(1);
		
		for(; count < len;)
		{
			if(I2C_WriteByte(AT24C02_IO, AT24C02_ADDRESS, reg_addr, byte) == IIC_Err)
			{
				AT24C02_Delay(1);
				
				if(++err_count >= 5)
				{
					IIC_FreeBus(AT24C02_IO);
					
					return result;
				}
				else
					continue;
			}
			
			reg_addr++;											//地址自增
			byte++;												//偏移到下个数据
			count++;
			
			AT24C02_Delay(1);									//必要延时，数据量一旦大了，比如十几字节时，1ms的延时有时无法保证全部字节正确写入
		}
		
		IIC_FreeBus(AT24C02_IO);
		
		result = IIC_OK;
	}
	
	return result;

}

/*
************************************************************
*	函数名称：	AT24C02_ReadByte
*
*	函数功能：	从EEPROM读一个字节
*
*	入口参数：	reg_addr：寄存器地址
*				byte：需要读取的数据的缓存地址
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
_Bool AT24C02_ReadByte(unsigned char reg_addr, unsigned char *byte)
{
	
	_Bool result = IIC_Err;

	if(eeprom_info.device_ok)
	{
		while(IIC_IsBusReady(AT24C02_IO) == IIC_Err)				//判断总线
			AT24C02_Delay(1);
		
		result = I2C_ReadByte(AT24C02_IO, AT24C02_ADDRESS, reg_addr, byte);
		
		IIC_FreeBus(AT24C02_IO);
	}
	
	return result;

}

/*
************************************************************
*	函数名称：	AT24C02_ReadBytes
*
*	函数功能：	从EEPROM读多个字节
*
*	入口参数：	reg_addr：寄存器地址
*				byte：需要写入的数据缓存区
*				len：数据长度
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
_Bool AT24C02_ReadBytes(unsigned char reg_addr, unsigned char *byte, unsigned char len)
{
	
	_Bool result = IIC_Err;

	if(eeprom_info.device_ok)
	{
		while(IIC_IsBusReady(AT24C02_IO) == IIC_Err)				//判断总线
			AT24C02_Delay(1);
		
		result = I2C_ReadBytes(AT24C02_IO, AT24C02_ADDRESS, reg_addr, byte, len);;
		
		IIC_FreeBus(AT24C02_IO);
	}
	
	return result;

}

/*
************************************************************
*	函数名称：	AT24C02_Clear
*
*	函数功能：	指定地址开始写入多个相同数据
*
*	入口参数：	start_addr：寄存器起始地址
*				byte：需要写入的数据
*				len：数据长度
*
*	返回参数：	0-成功		1-失败
*
*	说明：		写入指定位置指定长度的指定数据
************************************************************
*/
_Bool AT24C02_Clear(unsigned char start_addr, unsigned char byte, unsigned short len)
{
	
	_Bool result = IIC_Err;
	
	if(eeprom_info.device_ok)
	{
		unsigned short count = 0, err_count = 0;

		if(start_addr + len > 256)									//超出限制
			return result;

		while(IIC_IsBusReady(AT24C02_IO) == IIC_Err)				//判断总线
			AT24C02_Delay(1);
		
		for(; count < len;)
		{
			if(I2C_WriteByte(AT24C02_IO, AT24C02_ADDRESS, start_addr, &byte) == IIC_Err)
			{
				AT24C02_Delay(1);
				
				if(++err_count >= 5)
				{
					IIC_FreeBus(AT24C02_IO);
					
					return result;
				}
				else
					continue;
			}
			
			start_addr++;											//地址自增
			count++;
			
			AT24C02_Delay(1);										//必要延时，数据量一旦大了，比如十几字节时，1ms的延时有时无法保证全部字节正确写入
		}
		
		IIC_FreeBus(AT24C02_IO);
		
		result = IIC_OK;
	}
	
	return result;

}
