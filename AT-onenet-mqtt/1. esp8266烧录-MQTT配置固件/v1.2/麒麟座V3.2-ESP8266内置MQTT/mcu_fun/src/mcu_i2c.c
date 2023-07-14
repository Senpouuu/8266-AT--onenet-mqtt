/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	mcu_i2c.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2018-08-15
	*
	*	版本： 		V1.0
	*
	*	说明： 		硬件I2C
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机相关组件
#include "mcu_rcc.h"
#include "mcu_gpio.h"
#include "mcu_i2c.h"


/*
************************************************************
*	函数名称：	MCU_I2C_Init
*
*	函数功能：	单片机I2C初始化
*
*	入口参数：	I2C_Group：I2C组
*				I2C_Ack：应答开关
*				I2C_AcknowledgedAddress：地址位数
*				I2C_ClockSpeed：通信速率
*				I2C_DutyCycle：快速模式的占空比
*				I2C_Mode：I2C工作模式
*				I2C_OwnAddress1：本机地址(从模式有效)
*
*	返回参数：	0-成功	1-失败
*
*	说明：		
************************************************************
*/
_Bool MCU_I2C_Init(I2C_TypeDef *I2C_Group, unsigned short I2C_Ack, unsigned short I2C_AcknowledgedAddress, unsigned int I2C_ClockSpeed,
					unsigned short I2C_DutyCycle, unsigned short I2C_Mode, unsigned short I2C_OwnAddress1)
{
	
	_Bool result = 1;
	I2C_InitTypeDef i2c_initstruct;

	if(I2C_Group == I2C1)
	{
		MCU_GPIO_Init(GPIOB, GPIO_Pin_6 | GPIO_Pin_7, GPIO_Mode_AF_OD, GPIO_Speed_50MHz, (void *)0);
		result = 0;
	}
	else if(I2C_Group == I2C2)
	{
		MCU_GPIO_Init(GPIOB, GPIO_Pin_10 | GPIO_Pin_11, GPIO_Mode_AF_OD, GPIO_Speed_50MHz, (void *)0);
		result = 0;
	}
	
	if(result == 0)
	{
		MCU_RCC(I2C_Group, ENABLE);
		
		I2C_DeInit(I2C_Group);
        I2C_Cmd(I2C_Group, DISABLE);
		
		i2c_initstruct.I2C_Ack = I2C_Ack;
		i2c_initstruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress;
		i2c_initstruct.I2C_ClockSpeed = I2C_ClockSpeed;
		i2c_initstruct.I2C_DutyCycle = I2C_DutyCycle;
		i2c_initstruct.I2C_Mode = I2C_Mode;
		i2c_initstruct.I2C_OwnAddress1 = I2C_OwnAddress1;
		
		I2C_Init(I2C_Group, &i2c_initstruct);
	}
	
	return result;

}
