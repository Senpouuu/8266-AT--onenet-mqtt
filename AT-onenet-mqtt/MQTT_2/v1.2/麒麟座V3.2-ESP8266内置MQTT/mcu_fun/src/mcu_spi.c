/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	mcu_spi.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2018-02-26
	*
	*	版本： 		V1.0
	*
	*	说明： 		单片机SPI
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机相关组件
#include "mcu_rcc.h"
#include "mcu_gpio.h"
#include "mcu_spi.h"


/*
************************************************************
*	函数名称：	MCU_SPIx_Init
*
*	函数功能：	单片机SPI初始化
*
*	入口参数：	SPI_Group：SPI组
*				SPI_Direction：传输方向
*				SPI_Mode：工作模式
*				SPI_DataSize：数据宽度
*				SPI_CPOL：串行时钟的稳态
*				SPI_CPHA：数据捕获时钟沿
*				SPI_NSS：NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理
*				SPI_BaudRatePrescaler：波特率预分频的值
*				SPI_FirstBit：数据传输从MSB位还是LSB位开始
*				SPI_CRCPolynomial：CRC值计算的多项式
*
*	返回参数：	0-成功	1-失败
*
*	说明：		
************************************************************
*/
_Bool MCU_SPIx_Init(SPI_TypeDef *SPI_Group, unsigned short SPI_Direction, unsigned short SPI_Mode, unsigned short SPI_DataSize,
					unsigned short SPI_CPOL, unsigned short SPI_CPHA, unsigned short SPI_NSS, unsigned short SPI_BaudRatePrescaler,
					unsigned short SPI_FirstBit, unsigned short SPI_CRCPolynomial)
{
	
	_Bool result = 1;
	SPI_InitTypeDef spi_initstruct;

	MCU_RCC(SPI_Group, ENABLE);
	
	if(SPI_Group == SPI1)
	{
		MCU_GPIO_Init(GPIOA, GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7, GPIO_Mode_AF_PP, GPIO_Speed_50MHz, (void *)0);
		
		GPIO_SetBits(GPIOA, GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);
		
		result = 0;
	}
	else if(SPI_Group == SPI2)
	{
		MCU_GPIO_Init(GPIOB, GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15, GPIO_Mode_AF_PP, GPIO_Speed_50MHz, (void *)0);
		
		GPIO_SetBits(GPIOB, GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
		
		result = 0;
	}
	else if(SPI_Group == SPI3)
	{
		MCU_GPIO_SWJ_JTAGDisable();
		
		MCU_GPIO_Init(GPIOB, GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5, GPIO_Mode_AF_PP, GPIO_Speed_50MHz, (void *)0);
		
		GPIO_SetBits(GPIOB, GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5);
		
		result = 0;
	}
	
	if(result == 0)
	{
		spi_initstruct.SPI_Direction = SPI_Direction;					//设置SPI单向或者双向的数据模式
		spi_initstruct.SPI_Mode = SPI_Mode;								//设置SPI工作模式
		spi_initstruct.SPI_DataSize = SPI_DataSize;						//设置SPI的数据大小
		spi_initstruct.SPI_CPOL = SPI_CPOL;								//设置串行时钟的稳态
		spi_initstruct.SPI_CPHA = SPI_CPHA;								//设置数据捕获时钟沿
		spi_initstruct.SPI_NSS = SPI_NSS;								//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理
		spi_initstruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler;	//定义波特率预分频的值
		spi_initstruct.SPI_FirstBit = SPI_FirstBit;						//指定数据传输从MSB位还是LSB位开始
		spi_initstruct.SPI_CRCPolynomial = SPI_CRCPolynomial;			//CRC值计算的多项式
		
		SPI_Init(SPI_Group, &spi_initstruct);							//初始化
	}
	
	return result;

}

/*
************************************************************
*	函数名称：	MCU_SPIx_TX_Init
*
*	函数功能：	单片机SPI_TX初始化
*
*	入口参数：	SPI_Group：SPI组
*				SPI_Direction：传输方向
*				SPI_Mode：工作模式
*				SPI_DataSize：数据宽度
*				SPI_CPOL：串行时钟的稳态
*				SPI_CPHA：数据捕获时钟沿
*				SPI_NSS：NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理
*				SPI_BaudRatePrescaler：波特率预分频的值
*				SPI_FirstBit：数据传输从MSB位还是LSB位开始
*				SPI_CRCPolynomial：CRC值计算的多项式
*
*	返回参数：	0-成功	1-失败
*
*	说明：		
************************************************************
*/
_Bool MCU_SPIx_TX_Init(SPI_TypeDef *SPI_Group, unsigned short SPI_Direction, unsigned short SPI_Mode, unsigned short SPI_DataSize,
					unsigned short SPI_CPOL, unsigned short SPI_CPHA, unsigned short SPI_NSS, unsigned short SPI_BaudRatePrescaler,
					unsigned short SPI_FirstBit, unsigned short SPI_CRCPolynomial)
{
	
	_Bool result = 1;
	SPI_InitTypeDef spi_initstruct;

	MCU_RCC(SPI_Group, ENABLE);
	
	if(SPI_Group == SPI1)
	{
		MCU_GPIO_Init(GPIOA, GPIO_Pin_5 | GPIO_Pin_7, GPIO_Mode_AF_PP, GPIO_Speed_50MHz, (void *)0);
		
		GPIO_SetBits(GPIOA, GPIO_Pin_5 | GPIO_Pin_7);
		
		result = 0;
	}
	else if(SPI_Group == SPI2)
	{
		MCU_GPIO_Init(GPIOB, GPIO_Pin_13 | GPIO_Pin_15, GPIO_Mode_AF_PP, GPIO_Speed_50MHz, (void *)0);
		
		GPIO_SetBits(GPIOB, GPIO_Pin_13 | GPIO_Pin_15);
		
		result = 0;
	}
	else if(SPI_Group == SPI3)
	{
		MCU_GPIO_Init(GPIOB, GPIO_Pin_3 | GPIO_Pin_5, GPIO_Mode_AF_PP, GPIO_Speed_50MHz, (void *)0);
		
		GPIO_SetBits(GPIOB, GPIO_Pin_3 | GPIO_Pin_5);
		
		result = 0;
	}
	
	if(result == 0)
	{
		spi_initstruct.SPI_Direction = SPI_Direction;					//设置SPI单向或者双向的数据模式
		spi_initstruct.SPI_Mode = SPI_Mode;								//设置SPI工作模式
		spi_initstruct.SPI_DataSize = SPI_DataSize;						//设置SPI的数据大小
		spi_initstruct.SPI_CPOL = SPI_CPOL;								//设置串行时钟的稳态
		spi_initstruct.SPI_CPHA = SPI_CPHA;								//设置数据捕获时钟沿
		spi_initstruct.SPI_NSS = SPI_NSS;								//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理
		spi_initstruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler;	//定义波特率预分频的值
		spi_initstruct.SPI_FirstBit = SPI_FirstBit;						//指定数据传输从MSB位还是LSB位开始
		spi_initstruct.SPI_CRCPolynomial = SPI_CRCPolynomial;			//CRC值计算的多项式
		
		SPI_Init(SPI_Group, &spi_initstruct);							//初始化
	}
	
	return result;

}
