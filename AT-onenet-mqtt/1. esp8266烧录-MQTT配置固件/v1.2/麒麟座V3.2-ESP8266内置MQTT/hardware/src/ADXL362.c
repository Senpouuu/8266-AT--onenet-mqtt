/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	adxl362.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-12-01
	*
	*	版本： 		V1.2
	*
	*	说明： 		
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f10x.h"

//单片机相关组件
#include "mcu_gpio.h"

//硬件驱动
#include "delay.h"
#include "spi.h"
#include "adxl362.h"

//C库
#include <math.h>


ADXL362 adxl362_info;


const static GPIO_LIST spi1_cs_gpio_list[1] = {
													{GPIOA, GPIO_Pin_4, NULL},
												};


#define CS_H			spi1_cs_gpio_list[0].gpio_group->BSRR = spi1_cs_gpio_list[0].gpio_pin

#define CS_L			spi1_cs_gpio_list[0].gpio_group->BRR = spi1_cs_gpio_list[0].gpio_pin

												
/*
************************************************************
*	函数名称：	ADXL362_Delay
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
__inline static void ADXL362_Delay(unsigned int time)
{

	RTOS_TimeDly(time);

}

/*
************************************************************
*	函数名称：	ADXL362_WriteBytes
*
*	函数功能：	写数据
*
*	入口参数：	tx_buf：待发数据缓存
*				tx_bytes：待发数据量
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void ADXL362_WriteBytes(unsigned char *tx_buf, unsigned char tx_bytes)
{

	CS_L;

#if(SPI_DMA_TX_EN == 1)
	SPIx_DMA_ReadWriteByte(ADXL362_IO, tx_buf, adxl362_info.rx_buf, tx_bytes, tx_bytes);
#else
	{
		unsigned char i = 0;
		
		for(i = 0; i < tx_bytes; i++)
			SPIx_ReadWriteByte(ADXL362_IO, tx_buf[i]);
	}
#endif

	CS_H;

}

/*
************************************************************
*	函数名称：	ADXL362_ReadBytes
*
*	函数功能：	写数据
*
*	入口参数：	tx_buf：待发数据缓存
*				rx_buf：接收缓存
*				tx_bytes：待发数据量
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void ADXL362_ReadBytes(unsigned char *tx_buf, unsigned char *rx_buf, unsigned char tx_bytes)
{
	
	CS_L;

#if(SPI_DMA_RX_EN == 1)
	SPIx_DMA_ReadWriteByte(ADXL362_IO, tx_buf, rx_buf, tx_bytes, tx_bytes);
#else
	{
		unsigned char i = 0;
		
		for(i = 0; i < tx_bytes; i++)
			rx_buf[i] = SPIx_ReadWriteByte(ADXL362_IO, tx_buf[i]);
	}
#endif

	CS_H;

}

/*
************************************************************
*	函数名称：	ADXL362_SetRegisterValue
*
*	函数功能：	设置寄存器的值
*
*	入口参数：	addr：寄存器地址
*				value：值
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void ADXL362_SetRegisterValue(unsigned char addr, unsigned char value)
{

	adxl362_info.tx_buf[0] = ADXL362_WRITE_REG;
	adxl362_info.tx_buf[1] = addr;
	adxl362_info.tx_buf[2] = value;
	
	ADXL362_WriteBytes(adxl362_info.tx_buf, 3);

}

/*
************************************************************
*	函数名称：	ADXL362_GetRegisterValue
*
*	函数功能：	获取寄存器的值
*
*	入口参数：	addr：寄存器地址
*				rx_buf：接收缓存
*				tx_bytes：发送/接收的字节数
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void ADXL362_GetRegisterValue(unsigned char addr, unsigned char *rx_buf, unsigned char rx_bytes)
{
	
	unsigned char i = 0;

	adxl362_info.tx_buf[0] = ADXL362_READ_REG;
	adxl362_info.tx_buf[1] = addr;
	
	ADXL362_ReadBytes(adxl362_info.tx_buf, adxl362_info.rx_buf, rx_bytes + 2);
	
	for(i = 0; i < rx_bytes; i++)
		rx_buf[i] = adxl362_info.rx_buf[i + 2];

}

/*
************************************************************
*	函数名称：	ADXL362_Init
*
*	函数功能：	ADXL362初始化
*
*	入口参数：	无
*
*	返回参数：	0-成功	1-失败
*
*	说明：		
************************************************************
*/
_Bool ADXL362_Init(void)
{
	
	unsigned char reg_value = 0;
	
	MCU_GPIO_Init(spi1_cs_gpio_list[0].gpio_group, spi1_cs_gpio_list[0].gpio_pin, GPIO_Mode_Out_PP, GPIO_Speed_50MHz, spi1_cs_gpio_list[0].gpio_name);

	SPIx_Init(ADXL362_IO);
	
	ADXL362_GetRegisterValue(ADXL362_REG_DEVID_AD, &reg_value, 1);
	ADXL362_GetRegisterValue(ADXL362_REG_PARTID, &reg_value, 1);
	
	if(reg_value == ADXL362_PART_ID)
		adxl362_info.device_ok = 1;

	return adxl362_info.device_ok;

}

/*
************************************************************
*	函数名称：	ADXL362_SetPowerMode
*
*	函数功能：	设置电源模式
*
*	入口参数：	0-待机模式	1-测量模式
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void ADXL362_SetPowerMode(_Bool power_mode)
{

	unsigned char reg_value = 0, time_out = 200;
	
	while(SPIx_IsBusReady(ADXL362_IO) == SPI_Err && --time_out)
		ADXL362_Delay(2);
	
	if(time_out)
	{
		ADXL362_GetRegisterValue(ADXL362_REG_POWER_CTL, &reg_value, 1);
		
		reg_value &= 0xFC;
		reg_value |= power_mode ? 0x02 : 0x00;
		
		ADXL362_SetRegisterValue(ADXL362_REG_POWER_CTL, reg_value);
		
		SPIx_FreeBus(ADXL362_IO);
	}

}

/*
************************************************************
*	函数名称：	ADXL362_SetRange
*
*	函数功能：	设置测量范围
*
*	入口参数：	ADXL362_RANGE_2G、ADXL362_RANGE_4G、ADXL362_RANGE_8G
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void ADXL362_SetRange(unsigned char g_range)
{
	
	unsigned char reg_value = 0, time_out = 200;
	
	while(SPIx_IsBusReady(ADXL362_IO) == SPI_Err && --time_out)
		ADXL362_Delay(2);
	
	if(time_out)
	{
		ADXL362_GetRegisterValue(ADXL362_REG_FILTER_CTL, &reg_value, 1);
		
		reg_value &= 0x3F;
		switch(g_range)
		{
			case ADXL362_RANGE_2G: reg_value |= 0x00; break;
			case ADXL362_RANGE_4G: reg_value |= 0x01; break;
			case ADXL362_RANGE_8G: reg_value |= 0x10; break;
		}
		
		ADXL362_SetRegisterValue(ADXL362_REG_FILTER_CTL, reg_value);
		
		SPIx_FreeBus(ADXL362_IO);
	}
	
}

/*
************************************************************
*	函数名称：	ADXL362_SetOutputRate
*
*	函数功能：	设置数据输出速率
*
*	入口参数：	ADXL362_ODR_12_5_HZ、ADXL362_ODR_25_HZ、ADXL362_ODR_50_HZ
*				ADXL362_ODR_100_HZ、ADXL362_ODR_200_HZ、ADXL362_ODR_400_HZ
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void ADXL362_SetOutputRate(unsigned char out_rate)
{
	
	unsigned char reg_value = 0, time_out = 200;
	
	while(SPIx_IsBusReady(ADXL362_IO) == SPI_Err && --time_out)
		ADXL362_Delay(2);
	
	if(time_out)
	{
		ADXL362_GetRegisterValue(ADXL362_REG_FILTER_CTL, &reg_value, 1);
		
		reg_value &= 0xF8;
		switch(out_rate)
		{
			case ADXL362_ODR_12_5_HZ: reg_value |= 0x00; break;
			case ADXL362_ODR_25_HZ: reg_value |= 0x01; break;
			case ADXL362_ODR_50_HZ: reg_value |= 0x02; break;
			case ADXL362_ODR_100_HZ: reg_value |= 0x03; break;
			case ADXL362_ODR_200_HZ: reg_value |= 0x04; break;
			case ADXL362_ODR_400_HZ: reg_value |= 0x05; break;
		}
		
		ADXL362_SetRegisterValue(ADXL362_REG_FILTER_CTL, reg_value);
		
		SPIx_FreeBus(ADXL362_IO);
	}
	
}

/*
************************************************************
*	函数名称：	ADXL362_GetValue
*
*	函数功能：	获取三轴值
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void ADXL362_GetValue(void)
{
	
	short x_short = 0, y_short = 0, z_short = 0;
	
	unsigned char time_out = 200;

	ADXL362_SetPowerMode(1);
	
	while(SPIx_IsBusReady(ADXL362_IO) == SPI_Err && --time_out)
		ADXL362_Delay(2);
	
	if(time_out)
	{
		ADXL362_GetRegisterValue(ADXL362_REG_XDATA_L, adxl362_info.rx_buf, 6);
		x_short = ((short)adxl362_info.rx_buf[1] << 8) + adxl362_info.rx_buf[0];
		y_short = ((short)adxl362_info.rx_buf[3] << 8) + adxl362_info.rx_buf[2];
		z_short = ((short)adxl362_info.rx_buf[5] << 8) + adxl362_info.rx_buf[4];
		
		adxl362_info.x = (float)x_short * 0.001;					//数值转换
		adxl362_info.y = (float)y_short * 0.001;
		adxl362_info.z = (float)z_short * 0.001;
		
		adxl362_info.z_angle = atan(sqrt((x_short * x_short + y_short * y_short)) / z_short) * 57.29583;	//计算Z轴角度
		
		SPIx_FreeBus(ADXL362_IO);
	}
	
	ADXL362_SetPowerMode(0);

}
