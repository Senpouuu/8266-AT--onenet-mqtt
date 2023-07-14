/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	mcu_i2c.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2018-08-15
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		Ӳ��I2C
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��������
#include "mcu_rcc.h"
#include "mcu_gpio.h"
#include "mcu_i2c.h"


/*
************************************************************
*	�������ƣ�	MCU_I2C_Init
*
*	�������ܣ�	��Ƭ��I2C��ʼ��
*
*	��ڲ�����	I2C_Group��I2C��
*				I2C_Ack��Ӧ�𿪹�
*				I2C_AcknowledgedAddress����ַλ��
*				I2C_ClockSpeed��ͨ������
*				I2C_DutyCycle������ģʽ��ռ�ձ�
*				I2C_Mode��I2C����ģʽ
*				I2C_OwnAddress1��������ַ(��ģʽ��Ч)
*
*	���ز�����	0-�ɹ�	1-ʧ��
*
*	˵����		
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
