/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	i2c.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2020-06-29
	*
	*	�汾�� 		V1.3
	*
	*	˵���� 		IIC��������
	*
	*	�޸ļ�¼��	V1.1�����IIC����˫���߻���
	*					  ����Ӳ��IIC����
	*				V1.2������I2C��ģʽ����
	*				V1.3���޸�I2C�����ڶ��̵߳����µ�bug
	************************************************************
	************************************************************
	************************************************************
**/

//����
#include "i2c.h"
#include "usart.h"
#include "delay.h"

#if(HW_I2C == 1)
#include "mcu_i2c.h"
#include "mcu_nvic.h"
#endif


static _Bool i2c_busy[2] = {IIC_OK, IIC_OK};


IIC_INFO iic_info;


#if(HW_I2C == 0)
const GPIO_LIST i2c_gpio_list[4] = {
										{GPIOB, GPIO_Pin_6, "iic1_scl"},
										{GPIOB, GPIO_Pin_7, "iic1_sda"},
										
										{GPIOB, GPIO_Pin_10, "iic_scl"},
										{GPIOB, GPIO_Pin_11, "iic_sda"},
									};
#endif

/*
************************************************************
*	�������ƣ�	IIC_SpeedCtl
*
*	�������ܣ�	IIC�ٶȿ���
*
*	��ڲ�����	i2c_x��IIC��
*				speed����ʱ����
*
*	���ز�����	��
*
*	˵����		��λ��΢��
************************************************************
*/
void IIC_SpeedCtl(I2C_TypeDef *i2c_x, unsigned short speed)
{

	if(i2c_x == I2C1)
		iic_info.i2c1_speed = speed;
	else if(i2c_x == I2C2)
		iic_info.i2c2_speed = speed;

}

/*
************************************************************
*	�������ƣ�	IIC_DelayUs
*
*	�������ܣ�	IIC��ʱ
*
*	��ڲ�����	i2c_x��I2C��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void IIC_DelayUs(I2C_TypeDef *i2c_x)
{

	if(i2c_x == I2C1)
		DelayUs(iic_info.i2c1_speed);
	else if(i2c_x == I2C2)
		DelayUs(iic_info.i2c2_speed);

}

/*
************************************************************
*	�������ƣ�	IIC_IsBusReady
*
*	�������ܣ�	��ѯ�����Ƿ����
*
*	��ڲ�����	i2c_x��IIC��
*
*	���ز�����	0-����	1-δ����
*
*	˵����		
************************************************************
*/
_Bool IIC_IsBusReady(I2C_TypeDef *i2c_x)
{
	
	_Bool result = IIC_Err;
	
	RTOS_ENTER_CRITICAL();
	
	if(i2c_busy[i2c_x == I2C1 ? 0 : 1] == IIC_OK
#if(HW_I2C == 1)
		//&& I2C_GetFlagStatus(i2c_x, I2C_FLAG_BUSY) == RESET
#endif
	)
	{
		i2c_busy[i2c_x == I2C1 ? 0 : 1] = IIC_Err;
		
		result = IIC_OK;
	}
	
	RTOS_EXIT_CRITICAL();
	
	return result;

}

/*
************************************************************
*	�������ƣ�	IIC_FreeBus
*
*	�������ܣ�	�ͷ�����
*
*	��ڲ�����	i2c_x��IIC��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void IIC_FreeBus(I2C_TypeDef *i2c_x)
{

	RTOS_ENTER_CRITICAL();
	
	i2c_busy[i2c_x == I2C1 ? 0 : 1] = IIC_OK;
	
	RTOS_EXIT_CRITICAL();

}

/*
************************************************************
*	�������ƣ�	IIC_Init
*
*	�������ܣ�	IIC����IO��ʼ��
*
*	��ڲ�����	i2c_x��IIC��
*
*	���ز�����	0-�ɹ�	1-ʧ��
*
*	˵����		ʹ�ÿ�©��ʽ���������Բ����л�IO�ڵ������������
************************************************************
*/
_Bool IIC_Init(I2C_TypeDef *i2c_x)
{
	
	_Bool result = 1;
	
#if(HW_I2C == 1)
	#if(I2C_MASTER == 1)
		UsartPrintf(USART_DEBUG, "Tips:	I2C%d is Master Mode\r\n", i2c_x == I2C1 ? 1 : 2);
		
		MCU_I2C_Init(i2c_x, I2C_Ack_Enable, I2C_AcknowledgedAddress_7bit, 350000, I2C_DutyCycle_2, I2C_Mode_I2C, 0);
		I2C_Cmd(i2c_x, ENABLE);
		
		I2C_ITConfig(i2c_x, I2C_IT_ERR, ENABLE);								//ʹ�ܴ����ж�
		
		if(i2c_x == I2C1)
			MCU_NVIC_Init(I2C1_ER_IRQn, ENABLE, 1, 0);
		else if(i2c_x == I2C2)
			MCU_NVIC_Init(I2C2_ER_IRQn, ENABLE, 1, 0);
	#else
		UsartPrintf(USART_DEBUG, "Tips:	I2C%d is Slave Mode, OwnAddr: 0x%02X\r\n", i2c_x == I2C1 ? 1 : 2, SLAVE_ADDR);
		
		MCU_I2C_Init(i2c_x, I2C_Ack_Enable, I2C_AcknowledgedAddress_7bit, 350000, I2C_DutyCycle_2, I2C_Mode_I2C, SLAVE_ADDR << 1);
		I2C_Cmd(i2c_x, ENABLE);
		
		I2C_ITConfig(i2c_x, I2C_IT_EVT | I2C_IT_ERR |
										I2C_IT_BUF, ENABLE);					//ʹ���¼������󡢻������ж�
		
		if(i2c_x == I2C1)
		{
			MCU_NVIC_Init(I2C1_EV_IRQn, ENABLE, 1, 0);
			MCU_NVIC_Init(I2C1_ER_IRQn, ENABLE, 1, 1);
		}
		else if(i2c_x == I2C2)
		{
			MCU_NVIC_Init(I2C2_EV_IRQn, ENABLE, 1, 0);
			MCU_NVIC_Init(I2C2_ER_IRQn, ENABLE, 1, 1);
		}
	#endif
	
	if(IIC_IsBusReady(i2c_x) == IIC_OK)
	{
		result = 0;
		
		IIC_FreeBus(i2c_x);
		
		UsartPrintf(USART_DEBUG, "Tips:	I2C%d is Ready\r\n", i2c_x == I2C1 ? 1 : 2);
	}
	else
		UsartPrintf(USART_DEBUG, "Tips:	I2C%d is Err\r\n", i2c_x == I2C1 ? 1 : 2);
#else
	if(i2c_x == I2C1)
	{
		result = 0;
		
		MCU_GPIO_Init(i2c_gpio_list[0].gpio_group, i2c_gpio_list[0].gpio_pin, GPIO_Mode_Out_OD, GPIO_Speed_50MHz, i2c_gpio_list[0].gpio_name);
		MCU_GPIO_Init(i2c_gpio_list[1].gpio_group, i2c_gpio_list[1].gpio_pin, GPIO_Mode_Out_OD, GPIO_Speed_50MHz, i2c_gpio_list[1].gpio_name);
	}
	else if(i2c_x == I2C2)
	{
		result = 0;
		
		MCU_GPIO_Init(i2c_gpio_list[2].gpio_group, i2c_gpio_list[2].gpio_pin, GPIO_Mode_Out_OD, GPIO_Speed_50MHz, i2c_gpio_list[2].gpio_name);
		MCU_GPIO_Init(i2c_gpio_list[3].gpio_group, i2c_gpio_list[3].gpio_pin, GPIO_Mode_Out_OD, GPIO_Speed_50MHz, i2c_gpio_list[3].gpio_name);
	}
	
	IIC_SpeedCtl(i2c_x, 5);
	
	IIC_SDA_H(i2c_x);															//����SDA�ߣ����ڿ���״̬
	IIC_SCL_H(i2c_x);															//����SCL�ߣ����ڿ���״̬
#endif
	
	return result;

}

/*
************************************************************
*	�������ƣ�	IIC_Start
*
*	�������ܣ�	���IIC��ʼ�ź�
*
*	��ڲ�����	i2c_x��IIC��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void IIC_Start(I2C_TypeDef *i2c_x)
{
	
#if(HW_I2C == 1)
	I2C_GenerateSTART(i2c_x, ENABLE);
	while(I2C_CheckEvent(i2c_x, I2C_EVENT_MASTER_MODE_SELECT) == ERROR);		//�ȴ�EV5
#else
	IIC_SDA_H(i2c_x);															//����SDA��
	IIC_SCL_H(i2c_x);															//����SCL��
	IIC_DelayUs(i2c_x);															//��ʱ���ٶȿ���
	
	IIC_SDA_L(i2c_x);															//��SCL��Ϊ��ʱ��SDA��һ���½��ش���ʼ�ź�
	IIC_DelayUs(i2c_x);															//��ʱ���ٶȿ���
	IIC_SCL_L(i2c_x);															//ǯסSCL�ߣ��Ա㷢������
#endif

}

/*
************************************************************
*	�������ƣ�	IIC_Stop
*
*	�������ܣ�	���IICֹͣ�ź�
*
*	��ڲ�����	i2c_x��IIC��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void IIC_Stop(I2C_TypeDef *i2c_x)
{

#if(HW_I2C == 1)
	I2C_GenerateSTOP(i2c_x, ENABLE);
#else
	IIC_SDA_L(i2c_x);															//����SDA��
	IIC_SCL_L(i2c_x);															//����SCL��
	IIC_DelayUs(i2c_x);															//��ʱ���ٶȿ���
	
	IIC_SCL_H(i2c_x);															//����SCL��
	IIC_SDA_H(i2c_x);															//����SDA�ߣ���SCL��Ϊ��ʱ��SDA��һ�������ش���ֹͣ�ź�
	IIC_DelayUs(i2c_x);
#endif

}

/*
************************************************************
*	�������ƣ�	IIC_WaitAck
*
*	�������ܣ�	���IIC�ȴ�Ӧ��
*
*	��ڲ�����	i2c_x��IIC��
*				time_out����ʱʱ��
*
*	���ز�����	��
*
*	˵����		��λ��΢��
************************************************************
*/
_Bool IIC_WaitAck(I2C_TypeDef *i2c_x, unsigned int time_out)
{
	
	
#if(HW_I2C == 1)
	
#else
	IIC_SDA_H(i2c_x);IIC_DelayUs(i2c_x);										//����SDA��
	IIC_SCL_H(i2c_x);IIC_DelayUs(i2c_x);										//����SCL��
	
	while(IIC_SDA_R(i2c_x))														//�������SDA��Ϊ1����ȴ���Ӧ���ź�Ӧ��0
	{
		if(--time_out == 0)
		{
			UsartPrintf(USART1, "WaitAck TimeOut\r\n");

			IIC_Stop(i2c_x);													//��ʱδ�յ�Ӧ����ֹͣ����
			
			return IIC_Err;														//����ʧ��
		}
		
		IIC_DelayUs(i2c_x);
	}
	
	IIC_SCL_L(i2c_x);															//����SCL�ߣ��Ա�����շ�����
#endif
	
	return IIC_OK;																//���سɹ�
	
}

/*
************************************************************
*	�������ƣ�	IIC_Ack
*
*	�������ܣ�	���IIC����һ��Ӧ��
*
*	��ڲ�����	i2c_x��IIC��
*
*	���ز�����	��
*
*	˵����		��SDA��Ϊ��ʱ��SCL��һ�������������һ��Ӧ���ź�
************************************************************
*/
void IIC_Ack(I2C_TypeDef *i2c_x)
{
	
#if(HW_I2C == 1)
	I2C_AcknowledgeConfig(i2c_x, ENABLE);										//�����Զ�Ӧ��
#else
	IIC_SCL_L(i2c_x);															//����SCL��
	IIC_SDA_L(i2c_x);															//����SDA��
	IIC_DelayUs(i2c_x);
	IIC_SCL_H(i2c_x);															//����SCL��
	IIC_DelayUs(i2c_x);
	IIC_SCL_L(i2c_x);															//����SCL��
#endif
	
}

/*
************************************************************
*	�������ƣ�	IIC_NAck
*
*	�������ܣ�	���IIC����һ�Ǹ�Ӧ��
*
*	��ڲ�����	i2c_x��IIC��
*
*	���ز�����	��
*
*	˵����		��SDA��Ϊ��ʱ��SCL��һ�������������һ����Ӧ���ź�
************************************************************
*/
void IIC_NAck(I2C_TypeDef *i2c_x)
{
	
#if(HW_I2C == 1)
	I2C_AcknowledgeConfig(i2c_x, DISABLE);										//�ر��Զ�Ӧ��
#else
	IIC_SCL_L(i2c_x);															//����SCL��
	IIC_SDA_H(i2c_x);															//����SDA��
	IIC_DelayUs(i2c_x);
	IIC_SCL_H(i2c_x);															//����SCL��
	IIC_DelayUs(i2c_x);
	IIC_SCL_L(i2c_x);															//����SCL��
#endif
	
}

/*
************************************************************
*	�������ƣ�	IIC_SendByte
*
*	�������ܣ�	���IIC����һ���ֽ�
*
*	��ڲ�����	i2c_x��IIC��
*				byte����Ҫ���͵��ֽ�
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
_Bool IIC_SendByte(I2C_TypeDef *i2c_x, unsigned char byte)
{

#if(HW_I2C == 1)
	I2C_SendData(i2c_x, byte);
	IIC_CHECK_EVENT(i2c_x, I2C_EVENT_MASTER_BYTE_TRANSMITTED);					//�ȴ�EV8_2
#else
	unsigned char count = 0;
	
    IIC_SCL_L(i2c_x);															//����ʱ�ӿ�ʼ���ݴ���
	
    for(; count < 8; count++)													//ѭ��8�Σ�ÿ�η���һ��bit
    {
		if(byte & 0x80)															//�������λ
			IIC_SDA_H(i2c_x);
		else
			IIC_SDA_L(i2c_x);
		
		byte <<= 1;																//byte����1λ
		
		IIC_DelayUs(i2c_x);
		IIC_SCL_H(i2c_x);
		IIC_DelayUs(i2c_x);
		IIC_SCL_L(i2c_x);
    }
#endif
	
	return IIC_OK;

}

/*
************************************************************
*	�������ƣ�	IIC_RecvByte
*
*	�������ܣ�	���IIC����һ���ֽ�
*
*	��ڲ�����	i2c_x��IIC��
*
*	���ز�����	���յ����ֽ�����
*
*	˵����		
************************************************************
*/
unsigned char IIC_RecvByte(I2C_TypeDef *i2c_x)
{
	
#if(HW_I2C == 1)
	IIC_CHECK_EVENT(i2c_x, I2C_EVENT_MASTER_BYTE_RECEIVED);						//�ȴ�EV7
	return I2C_ReceiveData(i2c_x);
#else
	unsigned char count = 0, receive = 0;
	
	IIC_SDA_H(i2c_x);															//����SDA�ߣ���©״̬�£����������Ա��ȡ����
	
    for(; count < 8; count++ )													//ѭ��8�Σ�ÿ�η���һ��bit
	{
		IIC_SCL_L(i2c_x);
		IIC_DelayUs(i2c_x);
		IIC_SCL_H(i2c_x);
		
        receive <<= 1;															//����һλ
		
        if(IIC_SDA_R(i2c_x))													//���SDA��Ϊ1����receive����������ÿ���������Ƕ�bit0��+1��Ȼ����һ��ѭ����������һ��
			receive++;
		
		IIC_DelayUs(i2c_x);
    }
	
    return receive;
#endif
	
}

/*
************************************************************
*	�������ƣ�	I2C_WriteByte
*
*	�������ܣ�	���IICдһ������
*
*	��ڲ�����	i2c_x��IIC��
*				slave_addr���ӻ���ַ
*				reg_addr���Ĵ�����ַ
*				byte����Ҫд�������
*
*	���ز�����	0-д��ɹ�	1-д��ʧ��
*
*	˵����		*byte�ǻ���д�����ݵı����ĵ�ַ����Ϊ��Щ�Ĵ���ֻ��Ҫ�����¼Ĵ�����������Ҫд��ֵ
************************************************************
*/
_Bool I2C_WriteByte(I2C_TypeDef *i2c_x, unsigned char slave_addr, unsigned char reg_addr, unsigned char *byte)
{
	
	unsigned char addr = 0;

	addr = slave_addr << 1;														//IIC��ַ��7bit��������Ҫ����1λ��bit0��1-��	0-д

	IIC_Start(i2c_x);															//��ʼ�ź�
	
#if(HW_I2C == 1)
	I2C_Send7bitAddress(i2c_x, addr, I2C_Direction_Transmitter);
	IIC_CHECK_EVENT(i2c_x, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);			//�ȴ�EV6
	IIC_CHECK_EVENT(i2c_x, I2C_EVENT_MASTER_BYTE_TRANSMITTING);					//�ȴ�EV8
#else
	IIC_SendByte(i2c_x, addr);													//�����豸��ַ(д)
	if(IIC_WaitAck(i2c_x, 5000))												//�ȴ�Ӧ��
		return IIC_Err;
#endif
	
	IIC_SendByte(i2c_x,reg_addr);												//���ͼĴ�����ַ
	if(IIC_WaitAck(i2c_x, 5000))												//�ȴ�Ӧ��
		return IIC_Err;
	
	if(byte)
	{
		IIC_SendByte(i2c_x, *byte);												//��������
		if(IIC_WaitAck(i2c_x, 5000))											//�ȴ�Ӧ��
			return IIC_Err;
	}
	
	IIC_Stop(i2c_x);															//ֹͣ�ź�
	
	return IIC_OK;

}

/*
************************************************************
*	�������ƣ�	I2C_ReadByte
*
*	�������ܣ�	���IIC��ȡһ���ֽ�
*
*	��ڲ�����	i2c_x��IIC��
*				slave_addr���ӻ���ַ
*				reg_addr���Ĵ�����ַ
*				val����Ҫ��ȡ�����ݻ���
*
*	���ز�����	0-�ɹ�		1-ʧ��
*
*	˵����		val��һ����������ĵ�ַ
************************************************************
*/
_Bool I2C_ReadByte(I2C_TypeDef *i2c_x, unsigned char slave_addr, unsigned char reg_addr, unsigned char *val)
{
	
	unsigned char addr = 0;

    addr = slave_addr << 1;														//IIC��ַ��7bit��������Ҫ����1λ��bit0��1-��	0-д

	IIC_Start(i2c_x);															//��ʼ�ź�
	
#if(HW_I2C == 1)
	I2C_Send7bitAddress(i2c_x, addr, I2C_Direction_Transmitter);
	IIC_CHECK_EVENT(i2c_x, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);			//�ȴ�EV6
	IIC_CHECK_EVENT(i2c_x, I2C_EVENT_MASTER_BYTE_TRANSMITTING);					//�ȴ�EV8
#else
	IIC_SendByte(i2c_x, addr);													//�����豸��ַ(д)
	if(IIC_WaitAck(i2c_x, 5000))												//�ȴ�Ӧ��
		return IIC_Err;
#endif
	
	IIC_SendByte(i2c_x, reg_addr);												//���ͼĴ�����ַ
	if(IIC_WaitAck(i2c_x, 5000))												//�ȴ�Ӧ��
		return IIC_Err;
	
	IIC_Start(i2c_x);															//�����ź�
	
#if(HW_I2C == 1)
	I2C_Send7bitAddress(i2c_x, addr, I2C_Direction_Receiver);
	IIC_CHECK_EVENT(i2c_x, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);			//�ȴ�EV6
#else
	IIC_SendByte(i2c_x, addr + 1);												//�����豸��ַ(��)
	if(IIC_WaitAck(i2c_x, 5000))												//�ȴ�Ӧ��
		return IIC_Err;
#endif
	
	*val = IIC_RecvByte(i2c_x);													//����
	IIC_NAck(i2c_x);															//����һ����Ӧ���źţ������ȡ����
	
	IIC_Stop(i2c_x);															//ֹͣ�ź�
	
	return IIC_OK;

}

/*
************************************************************
*	�������ƣ�	I2C_WriteBytes
*
*	�������ܣ�	���IICд�������
*
*	��ڲ�����	i2c_x��IIC��
*				slave_addr���ӻ���ַ
*				reg_addr���Ĵ�����ַ
*				buf����Ҫд������ݻ���
*				num�����ݳ���
*
*	���ز�����	0-д��ɹ�	1-д��ʧ��
*
*	˵����		*buf��һ�������ָ��һ����������ָ��
************************************************************
*/
_Bool I2C_WriteBytes(I2C_TypeDef *i2c_x, unsigned char slave_addr, unsigned char reg_addr, unsigned char *buf, unsigned char num)
{

	unsigned char addr = 0;

	addr = slave_addr << 1;														//IIC��ַ��7bit��������Ҫ����1λ��bit0��1-��	0-д
	
	IIC_Start(i2c_x);															//��ʼ�ź�
	
#if(HW_I2C == 1)
	I2C_Send7bitAddress(i2c_x, addr, I2C_Direction_Transmitter);
	IIC_CHECK_EVENT(i2c_x, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);			//�ȴ�EV6
	IIC_CHECK_EVENT(i2c_x, I2C_EVENT_MASTER_BYTE_TRANSMITTING);					//�ȴ�EV8
#else
	IIC_SendByte(i2c_x, addr);													//�����豸��ַ(д)
	if(IIC_WaitAck(i2c_x, 5000))												//�ȴ�Ӧ��
		return IIC_Err;
#endif
	
	IIC_SendByte(i2c_x, reg_addr);												//���ͼĴ�����ַ
	if(IIC_WaitAck(i2c_x, 5000))												//�ȴ�Ӧ��
		return IIC_Err;
	
	while(num--)																//ѭ��д������
	{
		IIC_SendByte(i2c_x, *buf);												//��������
		if(IIC_WaitAck(i2c_x, 5000))											//�ȴ�Ӧ��
			return IIC_Err;
		
		buf++;																	//����ָ��ƫ�Ƶ���һ��
		
		DelayUs(10);
	}
	
	IIC_Stop(i2c_x);															//ֹͣ�ź�
	
	return IIC_OK;

}

/*
************************************************************
*	�������ƣ�	I2C_ReadBytes
*
*	�������ܣ�	���IIC���������
*
*	��ڲ�����	i2c_x��IIC��
*				slave_addr���ӻ���ַ
*				reg_addr���Ĵ�����ַ
*				buf����Ҫ��ȡ�����ݻ���
*				num�����ݳ���
*
*	���ز�����	0-д��ɹ�	1-д��ʧ��
*
*	˵����		*buf��һ�������ָ��һ����������ָ��
************************************************************
*/
_Bool I2C_ReadBytes(I2C_TypeDef *i2c_x, unsigned char slave_addr, unsigned char reg_addr, unsigned char *buf, unsigned char num)
{
	
	unsigned short addr = 0;

    addr = slave_addr << 1;														//IIC��ַ��7bit��������Ҫ����1λ��bit0��1-��	0-д

	IIC_Start(i2c_x);															//��ʼ�ź�
	
#if(HW_I2C == 1)
	I2C_Send7bitAddress(i2c_x, addr, I2C_Direction_Transmitter);
	IIC_CHECK_EVENT(i2c_x, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);			//�ȴ�EV6
	IIC_CHECK_EVENT(i2c_x, I2C_EVENT_MASTER_BYTE_TRANSMITTING);					//�ȴ�EV8
#else
	IIC_SendByte(i2c_x, addr);													//�����豸��ַ(д)
	if(IIC_WaitAck(i2c_x, 5000))												//�ȴ�Ӧ��
		return IIC_Err;
#endif
	
	IIC_SendByte(i2c_x, reg_addr);												//���ͼĴ�����ַ
	if(IIC_WaitAck(i2c_x, 5000))												//�ȴ�Ӧ��
		return IIC_Err;
	
	IIC_Start(i2c_x);															//�����ź�
	
#if(HW_I2C == 1)
	I2C_Send7bitAddress(i2c_x, addr, I2C_Direction_Receiver);
	IIC_CHECK_EVENT(i2c_x, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);			//�ȴ�EV6
	
	IIC_Ack(i2c_x);																//�����Զ�Ӧ��
#else
	IIC_SendByte(i2c_x, addr + 1);												//�����豸��ַ(��)
	if(IIC_WaitAck(i2c_x, 5000))												//�ȴ�Ӧ��
		return IIC_Err;
#endif
	
	while(num--)
	{
		*buf = IIC_RecvByte(i2c_x);
		buf++;																	//ƫ�Ƶ���һ�����ݴ洢��ַ
		
		if(num == 0)
			IIC_NAck(i2c_x);													//���һ��������Ҫ��NOACK
#if(HW_I2C == 0)
        else
			IIC_Ack(i2c_x);														//��ӦACK
#endif
	}
	
	IIC_Stop(i2c_x);
	
	return IIC_OK;

}

#if(HW_I2C == 1)
#if(I2C_MASTER == 0)
/*
************************************************************
*	�������ƣ�	I2C1_EV_IRQHandler
*
*	�������ܣ�	I2C1�¼��ж�
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void I2C1_EV_IRQHandler(void)
{
	
	RTOS_EnterInt();

	switch(I2C_GetLastEvent(I2C1))
	{
		//�ӻ�����---------------------------------------------------------------------------------------------------------------

		case I2C_EVENT_SLAVE_TRANSMITTER_ADDRESS_MATCHED:	//EV1
		
			//��ַƥ��	EV1�� ADDR=1����SR1Ȼ���SR2��������¼�
			//�� I2C_GetLastEvent ���Ѿ���ȡ��SR1��SR2
			
			//��Ҫ�����Ƿ��͵�ַƥ�䣬���������ʹӻ���ַ+��ʱ���ӻ�ƥ��Ϊ���͵�ַ
			iic_info.i2c1_rw_flag = 2;
		
		break;
		
		case I2C_EVENT_SLAVE_BYTE_TRANSMITTED:				//EV3_1
		
			
		
		break;

		case I2C_EVENT_SLAVE_BYTE_TRANSMITTING:				//EV3
		
			iic_info.i2c1_slave_send_cnt %= sizeof(iic_info.i2c1_slave_send_buf);
			
			I2C1->DR = iic_info.i2c1_slave_send_buf[iic_info.i2c1_slave_send_cnt++];
		
		break;
		
		case I2C_EVENT_SLAVE_ACK_FAILURE:					//EV3_2
		
			//�ֲ���SR1-bit4-STOPF����ģʽ�£����յ�NACK�� STOPFλ������λ
		
		break;

		//�ӻ�����---------------------------------------------------------------------------------------------------------------

		case I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED:		//EV1
		
			//��ַƥ��	EV1�� ADDR=1����SR1Ȼ���SR2��������¼�
			//�� I2C_GetLastEvent ���Ѿ���ȡ��SR1��SR2
			
			//��Ҫ�����ǽ��յ�ַƥ�䣬���������ʹӻ���ַ+дʱ���ӻ�ƥ��Ϊ���յ�ַ
			iic_info.i2c1_rw_flag = 1;
		
		break;

		case I2C_EVENT_SLAVE_BYTE_RECEIVED:					//EV2
		
			//��������	EV2�� RxNE=1����DR��������¼���
			
			iic_info.i2c1_slave_recv_cnt %= sizeof(iic_info.i2c1_slave_recv_buf);
			
			iic_info.i2c1_slave_recv_buf[iic_info.i2c1_slave_recv_cnt++] = (unsigned char)I2C1->DR;
		
		break;

		case I2C_EVENT_SLAVE_STOP_DETECTED:					//EV4
		
			//ֹͣ���	EV4�� STOPF=1����SR1Ȼ��дCR1�Ĵ�����������¼�
			//�� I2C_GetLastEvent ���Ѿ���ȡ��SR1��SR2
			
			I2C1->CR1 |= 0x0001;							//����ʹ��I2C
			
			iic_info.i2c1_rw_flag = 0;
			iic_info.i2c1_stop_flag = 1;					//�յ���ֹͣ�źţ������Ϊ�յ�һ֡��������
															//�ֲ���SR1-bit4-STOPF����ģʽ�£����յ�NACK�� STOPFλ������λ
		
		break;

		default:
		break;
	}
	
	RTOS_ExitInt();

}

/*
************************************************************
*	�������ƣ�	I2C2_EV_IRQHandler
*
*	�������ܣ�	I2C2�¼��ж�
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void I2C2_EV_IRQHandler(void)
{
	
	RTOS_EnterInt();

	switch(I2C_GetLastEvent(I2C2))
	{
		//�ӻ�����---------------------------------------------------------------------------------------------------------------

		case I2C_EVENT_SLAVE_TRANSMITTER_ADDRESS_MATCHED:	//EV1
		
			//��ַƥ��	EV1�� ADDR=1����SR1Ȼ���SR2��������¼�
			//�� I2C_GetLastEvent ���Ѿ���ȡ��SR1��SR2
			
			//��Ҫ�����Ƿ��͵�ַƥ�䣬���������ʹӻ���ַ+��ʱ���ӻ�ƥ��Ϊ���͵�ַ
			iic_info.i2c2_rw_flag = 2;
		
		break;
		
		case I2C_EVENT_SLAVE_BYTE_TRANSMITTED:				//EV3_1
		
			
		
		break;

		case I2C_EVENT_SLAVE_BYTE_TRANSMITTING:				//EV3
		
			iic_info.i2c2_slave_send_cnt %= sizeof(iic_info.i2c2_slave_send_buf);
			
			I2C2->DR = iic_info.i2c2_slave_send_buf[iic_info.i2c2_slave_send_cnt++];
		
		break;
		
		case I2C_EVENT_SLAVE_ACK_FAILURE:					//EV3_2
		
			//�ֲ���SR1-bit4-STOPF����ģʽ�£����յ�NACK�� STOPFλ������λ
		
		break;

		//�ӻ�����---------------------------------------------------------------------------------------------------------------

		case I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED:		//EV1
		
			//��ַƥ��	EV1�� ADDR=1����SR1Ȼ���SR2��������¼�
			//�� I2C_GetLastEvent ���Ѿ���ȡ��SR1��SR2
			
			//��Ҫ�����ǽ��յ�ַƥ�䣬���������ʹӻ���ַ+дʱ���ӻ�ƥ��Ϊ���յ�ַ
			iic_info.i2c2_rw_flag = 1;
		
		break;

		case I2C_EVENT_SLAVE_BYTE_RECEIVED:					//EV2
		
			//��������	EV2�� RxNE=1����DR��������¼���
			
			iic_info.i2c2_slave_recv_cnt %= sizeof(iic_info.i2c2_slave_recv_buf);
			
			iic_info.i2c2_slave_recv_buf[iic_info.i2c2_slave_recv_cnt++] = (unsigned char)I2C2->DR;
		
		break;

		case I2C_EVENT_SLAVE_STOP_DETECTED:					//EV4
		
			//ֹͣ���	EV4�� STOPF=1����SR1Ȼ��дCR1�Ĵ�����������¼�
			//�� I2C_GetLastEvent ���Ѿ���ȡ��SR1��SR2
			
			I2C2->CR1 |= 0x0001;							//����ʹ��I2C
			
			iic_info.i2c2_rw_flag = 0;
			iic_info.i2c2_stop_flag = 1;					//�յ���ֹͣ�źţ������Ϊ�յ�һ֡��������
															//�ֲ���SR1-bit4-STOPF����ģʽ�£����յ�NACK�� STOPFλ������λ
		
		break;

		default:
		break;
	}
	
	RTOS_ExitInt();

}
#endif

/*
************************************************************
*	�������ƣ�	I2C1_ER_IRQHandler
*
*	�������ܣ�	I2C1�����ж�
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void I2C1_ER_IRQHandler(void)
{
	
	RTOS_EnterInt();

	if(I2C_GetITStatus(I2C1, I2C_IT_BERR) == SET)			//���ߴ���
	{
		I2C_ClearITPendingBit(I2C1, I2C_IT_BERR);
		
		UsartPrintf(USART_DEBUG, "I2C1 Bus Error\r\n");
		
		if(I2C1->SR2 & 0x01)								//����ģʽ����£�Ӳ�����ͷ����ߣ�ͬʱ��Ӱ�쵱ǰ�Ĵ���״̬����ʱ����������Ƿ�Ҫ��ֹ��ǰ�Ĵ���
			IIC_Stop(I2C1);
	}
	
	if(I2C_GetITStatus(I2C1, I2C_IT_ARLO) == SET)			//�ٲö�ʧ����
	{
		I2C_ClearITPendingBit(I2C1, I2C_IT_ARLO);
		
		UsartPrintf(USART_DEBUG, "I2C1 Arbitration lost Error\r\n");
		
		//I2C�ӿ��Զ��ص���ģʽ(M/SLλ�����).��I2C�ӿڶ�ʧ���ٲ�,�����޷���ͬһ����������Ӧ���Ĵӵ�ַ,����������Ӯ�����ߵ����豸��������ʼ����֮����Ӧ
		//Ӳ���ͷ�����
	}
	
	if(I2C_GetITStatus(I2C1, I2C_IT_AF) == SET)				//Ӧ�����
	{
		I2C_ClearITPendingBit(I2C1, I2C_IT_AF);
		
//		UsartPrintf(USART_DEBUG, "I2C1 Acknowledge Error\r\n");
		
		if(I2C1->SR2 & 0x01)								//����Ǵ�����ģʽ,�����������һ��ֹͣ����
			IIC_Stop(I2C1);
		else
		{
#if(HW_I2C == 1 && I2C_MASTER == 0)
			if(iic_info.i2c1_rw_flag == 2)					//�ֲ���SR1-bit4-STOPF����ģʽ�£����յ�NACK�� STOPFλ������λ
			{
				iic_info.i2c1_rw_flag = 0;
				iic_info.i2c1_slave_send_cnt = 0;
				iic_info.i2c1_stop_flag = 1;
			}
#endif
		}
	}
	
	if(I2C_GetITStatus(I2C1, I2C_IT_OVR) == SET)			//����/Ƿ�ش���
	{
		I2C_ClearITPendingBit(I2C1, I2C_IT_OVR);
		
		UsartPrintf(USART_DEBUG, "I2C1 Overrun/Underrun Error\r\n");
		
		I2C_ClearFlag(I2C2, I2C_FLAG_RXNE);					//�ڹ��ش���ʱ,���Ӧ���RxNEλ,������Ӧ�����·������һ�η��͵��ֽ�
	}
	
	if(I2C_GetITStatus(I2C1, I2C_IT_PECERR) == SET)			//PECУ�����
	{
		I2C_ClearITPendingBit(I2C1, I2C_IT_PECERR);
		
		UsartPrintf(USART_DEBUG, "I2C1 PEC Error\r\n");
		
		//У�����Ӱ��ͨ�Ź���
	}
	
	if(I2C_GetITStatus(I2C1, I2C_IT_TIMEOUT) == SET)		//���߳�ʱ����
	{
		I2C_ClearITPendingBit(I2C1, I2C_IT_TIMEOUT);
		
		UsartPrintf(USART_DEBUG, "I2C1 TimeOut Error\r\n");
		
		if(I2C1->SR2 & 0x01)								//�����ֹͣ����
			IIC_Stop(I2C1);
	}
	
	RTOS_ExitInt();

}

/*
************************************************************
*	�������ƣ�	I2C2_ER_IRQHandler
*
*	�������ܣ�	I2C2�����ж�
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void I2C2_ER_IRQHandler(void)
{
	
	RTOS_EnterInt();

	if(I2C_GetITStatus(I2C2, I2C_IT_BERR) == SET)			//���ߴ���
	{
		I2C_ClearITPendingBit(I2C2, I2C_IT_BERR);
		
		UsartPrintf(USART_DEBUG, "I2C2 Bus Error\r\n");
		
		if(I2C2->SR2 & 0x01)								//����ģʽ����£�Ӳ�����ͷ����ߣ�ͬʱ��Ӱ�쵱ǰ�Ĵ���״̬����ʱ����������Ƿ�Ҫ��ֹ��ǰ�Ĵ���
			IIC_Stop(I2C2);
	}
	
	if(I2C_GetITStatus(I2C2, I2C_IT_ARLO) == SET)			//�ٲö�ʧ����
	{
		I2C_ClearITPendingBit(I2C2, I2C_IT_ARLO);
		
		UsartPrintf(USART_DEBUG, "I2C2 Arbitration lost Error\r\n");
		
		//I2C�ӿ��Զ��ص���ģʽ(M/SLλ�����).��I2C�ӿڶ�ʧ���ٲ�,�����޷���ͬһ����������Ӧ���Ĵӵ�ַ,����������Ӯ�����ߵ����豸��������ʼ����֮����Ӧ
		//Ӳ���ͷ�����
	}
	
	if(I2C_GetITStatus(I2C2, I2C_IT_AF) == SET)				//Ӧ�����
	{
		I2C_ClearITPendingBit(I2C2, I2C_IT_AF);
		
//		UsartPrintf(USART_DEBUG, "I2C2 Acknowledge Error\r\n");
		
		if(I2C2->SR2 & 0x01)								//����Ǵ�����ģʽ,�����������һ��ֹͣ����
			IIC_Stop(I2C2);
		else
		{
#if(HW_I2C == 1 && I2C_MASTER == 0)
			if(iic_info.i2c2_rw_flag == 2)					//�ֲ���SR1-bit4-STOPF����ģʽ�£����յ�NACK�� STOPFλ������λ
			{
				iic_info.i2c2_rw_flag = 0;
				iic_info.i2c2_slave_send_cnt = 0;
				iic_info.i2c2_stop_flag = 1;
			}
#endif
		}
	}
	
	if(I2C_GetITStatus(I2C2, I2C_IT_OVR) == SET)			//����/Ƿ�ش���
	{
		I2C_ClearITPendingBit(I2C2, I2C_IT_OVR);
		
		UsartPrintf(USART_DEBUG, "I2C2 Overrun/Underrun Error\r\n");
		
		I2C_ClearFlag(I2C2, I2C_FLAG_RXNE);					//�ڹ��ش���ʱ,���Ӧ���RxNEλ,������Ӧ�����·������һ�η��͵��ֽ�
	}
	
	if(I2C_GetITStatus(I2C2, I2C_IT_PECERR) == SET)			//PECУ�����
	{
		I2C_ClearITPendingBit(I2C2, I2C_IT_PECERR);
		
		UsartPrintf(USART_DEBUG, "I2C2 PEC Error\r\n");
		
		//У�����Ӱ��ͨ�Ź���
	}
	
	if(I2C_GetITStatus(I2C2, I2C_IT_TIMEOUT) == SET)		//���߳�ʱ����
	{
		I2C_ClearITPendingBit(I2C2, I2C_IT_TIMEOUT);
		
		UsartPrintf(USART_DEBUG, "I2C2 TimeOut Error\r\n");
		
		if(I2C2->SR2 & 0x01)								//�����ֹͣ����
			IIC_Stop(I2C2);
	}
	
	RTOS_ExitInt();

}
#endif
