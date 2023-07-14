#ifndef _MCU_I2C_H_
#define _MCU_I2C_H_


//单片机头文件
#include "stm32f10x.h"


_Bool MCU_I2C_Init(I2C_TypeDef *I2C_Group, unsigned short I2C_Ack, unsigned short I2C_AcknowledgedAddress, unsigned int I2C_ClockSpeed,
					unsigned short I2C_DutyCycle, unsigned short I2C_Mode, unsigned short I2C_OwnAddress1);


#endif
