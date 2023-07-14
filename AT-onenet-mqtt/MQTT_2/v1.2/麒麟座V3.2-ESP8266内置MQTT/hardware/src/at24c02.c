/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	AT24C02.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2018-08-16
	*
	*	�汾�� 		V1.2
	*
	*	˵���� 		EEPROM
	*
	*	�޸ļ�¼��	V1.1���޸���ʱ����д������bug
	*				V1.2���޸�д�����ʧ�ܵĴ���
	************************************************************
	************************************************************
	************************************************************
**/

//Ӳ������
#include "at24c02.h"
#include "i2c.h"
#include "delay.h"


EEPROM_INFO eeprom_info;


/*
************************************************************
*	�������ƣ�	AT24C02_Delay
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
__inline static void AT24C02_Delay(unsigned int time)
{

	RTOS_TimeDly(time);

}

/*
************************************************************
*	�������ƣ�	AT24C02_Exist
*
*	�������ܣ�	EEPROM�豸���߼��
*
*	��ڲ�����	��
*
*	���ز�����	0-δ��⵽	1-��⵽
*
*	˵����		
************************************************************
*/
_Bool AT24C02_Exist(void)
{
	
	unsigned char value = 0;

	//���EEPROM
	if(!I2C_ReadByte(AT24C02_IO, AT24C02_ADDRESS, 255, &value))
		eeprom_info.device_ok = 1;
	
	return eeprom_info.device_ok;

}

/*
************************************************************
*	�������ƣ�	AT24C02_WriteByte
*
*	�������ܣ�	дһ���ֽڵ�EEPROM
*
*	��ڲ�����	reg_addr���Ĵ�����ַ
*				byte����Ҫд�������
*
*	���ز�����	0-�ɹ�	1-ʧ��
*
*	˵����		
************************************************************
*/
_Bool AT24C02_WriteByte(unsigned char reg_addr, unsigned char byte)
{
	
	_Bool result = IIC_Err;

	if(eeprom_info.device_ok)
	{
		while(IIC_IsBusReady(AT24C02_IO) == IIC_Err)				//�ж�����
			AT24C02_Delay(1);
		
		result = I2C_WriteByte(AT24C02_IO, AT24C02_ADDRESS, reg_addr, &byte);
		
		IIC_FreeBus(AT24C02_IO);
	}
	
	return result;

}

/*
************************************************************
*	�������ƣ�	AT24C02_WriteBytes
*
*	�������ܣ�	д����ֽڵ�EEPROM
*
*	��ڲ�����	reg_addr���Ĵ�����ַ
*				byte����Ҫд������ݻ�����
*				len�����ݳ���
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
_Bool AT24C02_WriteBytes(unsigned char reg_addr, unsigned char *byte, unsigned char len)
{
	
	_Bool result = IIC_Err;

	if(eeprom_info.device_ok)
	{
		unsigned char count = 0, err_count = 0;
		
		while(IIC_IsBusReady(AT24C02_IO) == IIC_Err)			//�ж�����
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
			
			reg_addr++;											//��ַ����
			byte++;												//ƫ�Ƶ��¸�����
			count++;
			
			AT24C02_Delay(1);									//��Ҫ��ʱ��������һ�����ˣ�����ʮ���ֽ�ʱ��1ms����ʱ��ʱ�޷���֤ȫ���ֽ���ȷд��
		}
		
		IIC_FreeBus(AT24C02_IO);
		
		result = IIC_OK;
	}
	
	return result;

}

/*
************************************************************
*	�������ƣ�	AT24C02_ReadByte
*
*	�������ܣ�	��EEPROM��һ���ֽ�
*
*	��ڲ�����	reg_addr���Ĵ�����ַ
*				byte����Ҫ��ȡ�����ݵĻ����ַ
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
_Bool AT24C02_ReadByte(unsigned char reg_addr, unsigned char *byte)
{
	
	_Bool result = IIC_Err;

	if(eeprom_info.device_ok)
	{
		while(IIC_IsBusReady(AT24C02_IO) == IIC_Err)				//�ж�����
			AT24C02_Delay(1);
		
		result = I2C_ReadByte(AT24C02_IO, AT24C02_ADDRESS, reg_addr, byte);
		
		IIC_FreeBus(AT24C02_IO);
	}
	
	return result;

}

/*
************************************************************
*	�������ƣ�	AT24C02_ReadBytes
*
*	�������ܣ�	��EEPROM������ֽ�
*
*	��ڲ�����	reg_addr���Ĵ�����ַ
*				byte����Ҫд������ݻ�����
*				len�����ݳ���
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
_Bool AT24C02_ReadBytes(unsigned char reg_addr, unsigned char *byte, unsigned char len)
{
	
	_Bool result = IIC_Err;

	if(eeprom_info.device_ok)
	{
		while(IIC_IsBusReady(AT24C02_IO) == IIC_Err)				//�ж�����
			AT24C02_Delay(1);
		
		result = I2C_ReadBytes(AT24C02_IO, AT24C02_ADDRESS, reg_addr, byte, len);;
		
		IIC_FreeBus(AT24C02_IO);
	}
	
	return result;

}

/*
************************************************************
*	�������ƣ�	AT24C02_Clear
*
*	�������ܣ�	ָ����ַ��ʼд������ͬ����
*
*	��ڲ�����	start_addr���Ĵ�����ʼ��ַ
*				byte����Ҫд�������
*				len�����ݳ���
*
*	���ز�����	0-�ɹ�		1-ʧ��
*
*	˵����		д��ָ��λ��ָ�����ȵ�ָ������
************************************************************
*/
_Bool AT24C02_Clear(unsigned char start_addr, unsigned char byte, unsigned short len)
{
	
	_Bool result = IIC_Err;
	
	if(eeprom_info.device_ok)
	{
		unsigned short count = 0, err_count = 0;

		if(start_addr + len > 256)									//��������
			return result;

		while(IIC_IsBusReady(AT24C02_IO) == IIC_Err)				//�ж�����
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
			
			start_addr++;											//��ַ����
			count++;
			
			AT24C02_Delay(1);										//��Ҫ��ʱ��������һ�����ˣ�����ʮ���ֽ�ʱ��1ms����ʱ��ʱ�޷���֤ȫ���ֽ���ȷд��
		}
		
		IIC_FreeBus(AT24C02_IO);
		
		result = IIC_OK;
	}
	
	return result;

}
