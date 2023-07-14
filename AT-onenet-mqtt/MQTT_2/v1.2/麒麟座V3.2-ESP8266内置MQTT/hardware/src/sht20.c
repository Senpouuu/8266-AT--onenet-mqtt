/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	sht20.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2016-11-23
	*
	*	版本： 		V1.0
	*
	*	说明： 		SHT20驱动
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f10x.h"

//硬件驱动
#include "sht20.h"
#include "i2c.h"
#include "delay.h"
#include "usart.h"


const int16_t POLYNOMIAL = 0x131;


SHT20_INFO sht20_info;


/*
************************************************************
*	函数名称：	SHT20_Delay
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
__inline static void SHT20_Delay(unsigned int time)
{

	RTOS_TimeDly(time);

}

/*
************************************************************
*	函数名称：	SHT20_Exist
*
*	函数功能：	SHT20设备在线检查
*
*	入口参数：	无
*
*	返回参数：	0-未检测到	1-检测到
*
*	说明：		
************************************************************
*/
_Bool SHT20_Exist(void)
{
	
	unsigned char value = 0;

	//检测SHT20
	if(!I2C_ReadByte(SHT20_IO, SHT20_ADDRESS, 0XE7, &value))	//读取用户寄存器
		sht20_info.device_ok = 1;
	
	return sht20_info.device_ok;

}

/*
************************************************************
*	函数名称：	SHT20_reset
*
*	函数功能：	SHT20复位
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void SHT20_reset(void)
{
	
    I2C_WriteByte(SHT20_IO, SHT20_ADDRESS, SHT20_SOFT_RESET, (void *)0);
	
}

/*
************************************************************
*	函数名称：	SHT20_read_user_reg
*
*	函数功能：	SHT20读取用户寄存器
*
*	入口参数：	无
*
*	返回参数：	读取到的用户寄存器的值
*
*	说明：		
************************************************************
*/
unsigned char  SHT20_read_user_reg(void)
{
	
    unsigned char val = 0;
	
    I2C_ReadByte(SHT20_IO, SHT20_ADDRESS, SHT20_READ_REG, &val);
	
    return val;
	
}

/*
************************************************************
*	函数名称：	SHT2x_CheckCrc
*
*	函数功能：	检查数据正确性
*
*	入口参数：	data：读取到的数据
*				nbrOfBytes：需要校验的数量
*				checksum：读取到的校对比验值
*
*	返回参数：	校验结果
*
*	说明：		0-成功		1-失败
************************************************************
*/
char SHT2x_CheckCrc(char data[], char nbrOfBytes, char checksum)
{
	
    char crc = 0;
    char bit = 0;
    char byteCtr = 0;
	
    //calculates 8-Bit checksum with given polynomial
    for(byteCtr = 0; byteCtr < nbrOfBytes; ++byteCtr)
    {
        crc ^= (data[byteCtr]);
        for ( bit = 8; bit > 0; --bit)
        {
            if (crc & 0x80) crc = (crc << 1) ^ POLYNOMIAL;
            else crc = (crc << 1);
        }
    }
	
    if(crc != checksum)
		return 1;
    else
		return 0;
	
}

/*
************************************************************
*	函数名称：	SHT2x_CalcTemperatureC
*
*	函数功能：	温度计算
*
*	入口参数：	u16sT：读取到的温度原始数据
*
*	返回参数：	计算后的温度数据
*
*	说明：		
************************************************************
*/
float SHT2x_CalcTemperatureC(unsigned short u16sT)
{
	
    float temperatureC = 0;

    u16sT &= ~0x0003;           // clear bits [1..0] (status bits)
    //-- calculate temperature [癈] --
    temperatureC = -46.85 + 175.72 / 65536 * (float)u16sT; //T= -46.85 + 175.72 * ST/2^16
	
    return temperatureC;
	
}

/*
************************************************************
*	函数名称：	SHT2x_CalcRH
*
*	函数功能：	湿度计算
*
*	入口参数：	u16sRH：读取到的湿度原始数据
*
*	返回参数：	计算后的湿度数据
*
*	说明：		
************************************************************
*/
float SHT2x_CalcRH(unsigned short u16sRH)
{
	
    float humidityRH = 0;
	
    u16sRH &= ~0x0003;          // clear bits [1..0] (status bits)
    //-- calculate relative humidity [%RH] --
    //humidityRH = -6.0 + 125.0/65536 * (float)u16sRH; // RH= -6 + 125 * SRH/2^16
    humidityRH = ((float)u16sRH * 0.00190735) - 6;
	
    return humidityRH;
	
}

/*
************************************************************
*	函数名称：	SHT2x_MeasureHM
*
*	函数功能：	测量温湿度
*
*	入口参数：	cmd：测量温度还是湿度
*				pMeasurand：不为空则保存为ushort值到此地址
*
*	返回参数：	测量结果
*
*	说明：		
************************************************************
*/
float SHT2x_MeasureHM(unsigned char cmd, unsigned short *pMeasurand)
{
	
    char checksum = 0;
    char data[2];
	unsigned char addr = 0;
    unsigned short tmp = 0;
    float t = 0;
	
    addr = SHT20_ADDRESS << 1;
	
	IIC_Start(SHT20_IO);
	
#if(HW_I2C == 1)
	I2C_Send7bitAddress(SHT20_IO, addr, I2C_Direction_Transmitter);
	IIC_CHECK_EVENT(SHT20_IO, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);			//等待EV6
	IIC_CHECK_EVENT(SHT20_IO, I2C_EVENT_MASTER_BYTE_TRANSMITTING);					//等待EV8
#else
	IIC_SendByte(SHT20_IO, addr);
	if(IIC_WaitAck(SHT20_IO, 50000))
		return 0.0;
#endif
	
	IIC_SendByte(SHT20_IO, cmd);
	if(IIC_WaitAck(SHT20_IO, 50000))
		return 0.0;
	
	IIC_Start(SHT20_IO);
	
#if(HW_I2C == 1)
	I2C_Send7bitAddress(SHT20_IO, addr, I2C_Direction_Receiver);
	IIC_CHECK_EVENT(SHT20_IO, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);				//等待EV6
	
	IIC_Ack(SHT20_IO);																//开启自动应答
#else
	IIC_SendByte(SHT20_IO, addr + 1);
	while(IIC_WaitAck(SHT20_IO, 50000))
	{
		IIC_Start(SHT20_IO);
		IIC_SendByte(SHT20_IO, addr + 1);
	}
#endif
	
	SHT20_Delay(14);
	
	data[0] = IIC_RecvByte(SHT20_IO);
	IIC_Ack(SHT20_IO);
	data[1] = IIC_RecvByte(SHT20_IO);
	IIC_Ack(SHT20_IO);
	
	checksum = IIC_RecvByte(SHT20_IO);
	IIC_NAck(SHT20_IO);
	
	IIC_Stop(SHT20_IO);
	
	SHT2x_CheckCrc(data, 2, checksum);
    tmp = (data[0] << 8) + data[1];
    if(cmd == SHT20_Measurement_T_HM)
    {
        t = SHT2x_CalcTemperatureC(tmp);
    }
    else
    {
        t = SHT2x_CalcRH(tmp);
    }
	
    if(pMeasurand)
    {
        *pMeasurand = (unsigned short)t;
    }
	
    return t;
	
}

/*
************************************************************
*	函数名称：	SHT20_GetValue
*
*	函数功能：	获取温湿度数据
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		温湿度结果保存在SHT20结构体里
************************************************************
*/
void SHT20_GetValue(void)
{
	
	if(sht20_info.device_ok && IIC_IsBusReady(SHT20_IO) == IIC_OK)
	{
		unsigned char val = 0;
		
		IIC_SpeedCtl(SHT20_IO, 5);
		
		SHT20_read_user_reg();
		DelayUs(100);
		
		sht20_info.tempreture = SHT2x_MeasureHM(SHT20_Measurement_T_HM, (void *)0);
		SHT20_Delay(15);
		
		sht20_info.humidity = SHT2x_MeasureHM(SHT20_Measurement_RH_HM, (void *)0);
		SHT20_Delay(6);
		
		SHT20_read_user_reg();
		SHT20_Delay(6);
		
		I2C_WriteByte(SHT20_IO, SHT20_ADDRESS, SHT20_WRITE_REG, &val);
		DelayUs(100);
		
		SHT20_read_user_reg();
		DelayUs(100);
		
		SHT20_reset();
		DelayUs(100);
		
		IIC_FreeBus(SHT20_IO);
	}

}
