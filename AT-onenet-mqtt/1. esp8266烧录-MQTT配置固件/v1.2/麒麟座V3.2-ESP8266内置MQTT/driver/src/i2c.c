/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	i2c.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2020-06-29
	*
	*	版本： 		V1.3
	*
	*	说明： 		IIC总线驱动
	*
	*	修改记录：	V1.1：软件IIC增加双总线机制
	*					  加入硬件IIC功能
	*				V1.2：增加I2C从模式功能
	*				V1.3：修复I2C总线在多线程调用下的bug
	************************************************************
	************************************************************
	************************************************************
**/

//驱动
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
*	函数名称：	IIC_SpeedCtl
*
*	函数功能：	IIC速度控制
*
*	入口参数：	i2c_x：IIC组
*				speed：延时参数
*
*	返回参数：	无
*
*	说明：		单位：微秒
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
*	函数名称：	IIC_DelayUs
*
*	函数功能：	IIC延时
*
*	入口参数：	i2c_x：I2C组
*
*	返回参数：	无
*
*	说明：		
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
*	函数名称：	IIC_IsBusReady
*
*	函数功能：	查询总线是否就绪
*
*	入口参数：	i2c_x：IIC组
*
*	返回参数：	0-就绪	1-未就绪
*
*	说明：		
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
*	函数名称：	IIC_FreeBus
*
*	函数功能：	释放总线
*
*	入口参数：	i2c_x：IIC组
*
*	返回参数：	无
*
*	说明：		
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
*	函数名称：	IIC_Init
*
*	函数功能：	IIC总线IO初始化
*
*	入口参数：	i2c_x：IIC组
*
*	返回参数：	0-成功	1-失败
*
*	说明：		使用开漏方式，这样可以不用切换IO口的输入输出方向
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
		
		I2C_ITConfig(i2c_x, I2C_IT_ERR, ENABLE);								//使能错误中断
		
		if(i2c_x == I2C1)
			MCU_NVIC_Init(I2C1_ER_IRQn, ENABLE, 1, 0);
		else if(i2c_x == I2C2)
			MCU_NVIC_Init(I2C2_ER_IRQn, ENABLE, 1, 0);
	#else
		UsartPrintf(USART_DEBUG, "Tips:	I2C%d is Slave Mode, OwnAddr: 0x%02X\r\n", i2c_x == I2C1 ? 1 : 2, SLAVE_ADDR);
		
		MCU_I2C_Init(i2c_x, I2C_Ack_Enable, I2C_AcknowledgedAddress_7bit, 350000, I2C_DutyCycle_2, I2C_Mode_I2C, SLAVE_ADDR << 1);
		I2C_Cmd(i2c_x, ENABLE);
		
		I2C_ITConfig(i2c_x, I2C_IT_EVT | I2C_IT_ERR |
										I2C_IT_BUF, ENABLE);					//使能事件、错误、缓冲器中断
		
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
	
	IIC_SDA_H(i2c_x);															//拉高SDA线，处于空闲状态
	IIC_SCL_H(i2c_x);															//拉高SCL线，处于空闲状态
#endif
	
	return result;

}

/*
************************************************************
*	函数名称：	IIC_Start
*
*	函数功能：	软件IIC开始信号
*
*	入口参数：	i2c_x：IIC组
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void IIC_Start(I2C_TypeDef *i2c_x)
{
	
#if(HW_I2C == 1)
	I2C_GenerateSTART(i2c_x, ENABLE);
	while(I2C_CheckEvent(i2c_x, I2C_EVENT_MASTER_MODE_SELECT) == ERROR);		//等待EV5
#else
	IIC_SDA_H(i2c_x);															//拉高SDA线
	IIC_SCL_H(i2c_x);															//拉高SCL线
	IIC_DelayUs(i2c_x);															//延时，速度控制
	
	IIC_SDA_L(i2c_x);															//当SCL线为高时，SDA线一个下降沿代表开始信号
	IIC_DelayUs(i2c_x);															//延时，速度控制
	IIC_SCL_L(i2c_x);															//钳住SCL线，以便发送数据
#endif

}

/*
************************************************************
*	函数名称：	IIC_Stop
*
*	函数功能：	软件IIC停止信号
*
*	入口参数：	i2c_x：IIC组
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void IIC_Stop(I2C_TypeDef *i2c_x)
{

#if(HW_I2C == 1)
	I2C_GenerateSTOP(i2c_x, ENABLE);
#else
	IIC_SDA_L(i2c_x);															//拉低SDA线
	IIC_SCL_L(i2c_x);															//拉低SCL先
	IIC_DelayUs(i2c_x);															//延时，速度控制
	
	IIC_SCL_H(i2c_x);															//拉高SCL线
	IIC_SDA_H(i2c_x);															//拉高SDA线，当SCL线为高时，SDA线一个上升沿代表停止信号
	IIC_DelayUs(i2c_x);
#endif

}

/*
************************************************************
*	函数名称：	IIC_WaitAck
*
*	函数功能：	软件IIC等待应答
*
*	入口参数：	i2c_x：IIC组
*				time_out：超时时间
*
*	返回参数：	无
*
*	说明：		单位：微秒
************************************************************
*/
_Bool IIC_WaitAck(I2C_TypeDef *i2c_x, unsigned int time_out)
{
	
	
#if(HW_I2C == 1)
	
#else
	IIC_SDA_H(i2c_x);IIC_DelayUs(i2c_x);										//拉高SDA线
	IIC_SCL_H(i2c_x);IIC_DelayUs(i2c_x);										//拉高SCL线
	
	while(IIC_SDA_R(i2c_x))														//如果读到SDA线为1，则等待。应答信号应是0
	{
		if(--time_out == 0)
		{
			UsartPrintf(USART1, "WaitAck TimeOut\r\n");

			IIC_Stop(i2c_x);													//超时未收到应答，则停止总线
			
			return IIC_Err;														//返回失败
		}
		
		IIC_DelayUs(i2c_x);
	}
	
	IIC_SCL_L(i2c_x);															//拉低SCL线，以便继续收发数据
#endif
	
	return IIC_OK;																//返回成功
	
}

/*
************************************************************
*	函数名称：	IIC_Ack
*
*	函数功能：	软件IIC产生一个应答
*
*	入口参数：	i2c_x：IIC组
*
*	返回参数：	无
*
*	说明：		当SDA线为低时，SCL线一个正脉冲代表发送一个应答信号
************************************************************
*/
void IIC_Ack(I2C_TypeDef *i2c_x)
{
	
#if(HW_I2C == 1)
	I2C_AcknowledgeConfig(i2c_x, ENABLE);										//开启自动应答
#else
	IIC_SCL_L(i2c_x);															//拉低SCL线
	IIC_SDA_L(i2c_x);															//拉低SDA线
	IIC_DelayUs(i2c_x);
	IIC_SCL_H(i2c_x);															//拉高SCL线
	IIC_DelayUs(i2c_x);
	IIC_SCL_L(i2c_x);															//拉低SCL线
#endif
	
}

/*
************************************************************
*	函数名称：	IIC_NAck
*
*	函数功能：	软件IIC产生一非个应答
*
*	入口参数：	i2c_x：IIC组
*
*	返回参数：	无
*
*	说明：		当SDA线为高时，SCL线一个正脉冲代表发送一个非应答信号
************************************************************
*/
void IIC_NAck(I2C_TypeDef *i2c_x)
{
	
#if(HW_I2C == 1)
	I2C_AcknowledgeConfig(i2c_x, DISABLE);										//关闭自动应答
#else
	IIC_SCL_L(i2c_x);															//拉低SCL线
	IIC_SDA_H(i2c_x);															//拉高SDA线
	IIC_DelayUs(i2c_x);
	IIC_SCL_H(i2c_x);															//拉高SCL线
	IIC_DelayUs(i2c_x);
	IIC_SCL_L(i2c_x);															//拉低SCL线
#endif
	
}

/*
************************************************************
*	函数名称：	IIC_SendByte
*
*	函数功能：	软件IIC发送一个字节
*
*	入口参数：	i2c_x：IIC组
*				byte：需要发送的字节
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
_Bool IIC_SendByte(I2C_TypeDef *i2c_x, unsigned char byte)
{

#if(HW_I2C == 1)
	I2C_SendData(i2c_x, byte);
	IIC_CHECK_EVENT(i2c_x, I2C_EVENT_MASTER_BYTE_TRANSMITTED);					//等待EV8_2
#else
	unsigned char count = 0;
	
    IIC_SCL_L(i2c_x);															//拉低时钟开始数据传输
	
    for(; count < 8; count++)													//循环8次，每次发送一个bit
    {
		if(byte & 0x80)															//发送最高位
			IIC_SDA_H(i2c_x);
		else
			IIC_SDA_L(i2c_x);
		
		byte <<= 1;																//byte左移1位
		
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
*	函数名称：	IIC_RecvByte
*
*	函数功能：	软件IIC接收一个字节
*
*	入口参数：	i2c_x：IIC组
*
*	返回参数：	接收到的字节数据
*
*	说明：		
************************************************************
*/
unsigned char IIC_RecvByte(I2C_TypeDef *i2c_x)
{
	
#if(HW_I2C == 1)
	IIC_CHECK_EVENT(i2c_x, I2C_EVENT_MASTER_BYTE_RECEIVED);						//等待EV7
	return I2C_ReceiveData(i2c_x);
#else
	unsigned char count = 0, receive = 0;
	
	IIC_SDA_H(i2c_x);															//拉高SDA线，开漏状态下，需线拉高以便读取数据
	
    for(; count < 8; count++ )													//循环8次，每次发送一个bit
	{
		IIC_SCL_L(i2c_x);
		IIC_DelayUs(i2c_x);
		IIC_SCL_H(i2c_x);
		
        receive <<= 1;															//左移一位
		
        if(IIC_SDA_R(i2c_x))													//如果SDA线为1，则receive变量自增，每次自增都是对bit0的+1，然后下一次循环会先左移一次
			receive++;
		
		IIC_DelayUs(i2c_x);
    }
	
    return receive;
#endif
	
}

/*
************************************************************
*	函数名称：	I2C_WriteByte
*
*	函数功能：	软件IIC写一个数据
*
*	入口参数：	i2c_x：IIC组
*				slave_addr：从机地址
*				reg_addr：寄存器地址
*				byte：需要写入的数据
*
*	返回参数：	0-写入成功	1-写入失败
*
*	说明：		*byte是缓存写入数据的变量的地址，因为有些寄存器只需要控制下寄存器，并不需要写入值
************************************************************
*/
_Bool I2C_WriteByte(I2C_TypeDef *i2c_x, unsigned char slave_addr, unsigned char reg_addr, unsigned char *byte)
{
	
	unsigned char addr = 0;

	addr = slave_addr << 1;														//IIC地址是7bit，这里需要左移1位，bit0：1-读	0-写

	IIC_Start(i2c_x);															//起始信号
	
#if(HW_I2C == 1)
	I2C_Send7bitAddress(i2c_x, addr, I2C_Direction_Transmitter);
	IIC_CHECK_EVENT(i2c_x, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);			//等待EV6
	IIC_CHECK_EVENT(i2c_x, I2C_EVENT_MASTER_BYTE_TRANSMITTING);					//等待EV8
#else
	IIC_SendByte(i2c_x, addr);													//发送设备地址(写)
	if(IIC_WaitAck(i2c_x, 5000))												//等待应答
		return IIC_Err;
#endif
	
	IIC_SendByte(i2c_x,reg_addr);												//发送寄存器地址
	if(IIC_WaitAck(i2c_x, 5000))												//等待应答
		return IIC_Err;
	
	if(byte)
	{
		IIC_SendByte(i2c_x, *byte);												//发送数据
		if(IIC_WaitAck(i2c_x, 5000))											//等待应答
			return IIC_Err;
	}
	
	IIC_Stop(i2c_x);															//停止信号
	
	return IIC_OK;

}

/*
************************************************************
*	函数名称：	I2C_ReadByte
*
*	函数功能：	软件IIC读取一个字节
*
*	入口参数：	i2c_x：IIC组
*				slave_addr：从机地址
*				reg_addr：寄存器地址
*				val：需要读取的数据缓存
*
*	返回参数：	0-成功		1-失败
*
*	说明：		val是一个缓存变量的地址
************************************************************
*/
_Bool I2C_ReadByte(I2C_TypeDef *i2c_x, unsigned char slave_addr, unsigned char reg_addr, unsigned char *val)
{
	
	unsigned char addr = 0;

    addr = slave_addr << 1;														//IIC地址是7bit，这里需要左移1位，bit0：1-读	0-写

	IIC_Start(i2c_x);															//起始信号
	
#if(HW_I2C == 1)
	I2C_Send7bitAddress(i2c_x, addr, I2C_Direction_Transmitter);
	IIC_CHECK_EVENT(i2c_x, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);			//等待EV6
	IIC_CHECK_EVENT(i2c_x, I2C_EVENT_MASTER_BYTE_TRANSMITTING);					//等待EV8
#else
	IIC_SendByte(i2c_x, addr);													//发送设备地址(写)
	if(IIC_WaitAck(i2c_x, 5000))												//等待应答
		return IIC_Err;
#endif
	
	IIC_SendByte(i2c_x, reg_addr);												//发送寄存器地址
	if(IIC_WaitAck(i2c_x, 5000))												//等待应答
		return IIC_Err;
	
	IIC_Start(i2c_x);															//重启信号
	
#if(HW_I2C == 1)
	I2C_Send7bitAddress(i2c_x, addr, I2C_Direction_Receiver);
	IIC_CHECK_EVENT(i2c_x, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);			//等待EV6
#else
	IIC_SendByte(i2c_x, addr + 1);												//发送设备地址(读)
	if(IIC_WaitAck(i2c_x, 5000))												//等待应答
		return IIC_Err;
#endif
	
	*val = IIC_RecvByte(i2c_x);													//接收
	IIC_NAck(i2c_x);															//产生一个非应答信号，代表读取接收
	
	IIC_Stop(i2c_x);															//停止信号
	
	return IIC_OK;

}

/*
************************************************************
*	函数名称：	I2C_WriteBytes
*
*	函数功能：	软件IIC写多个数据
*
*	入口参数：	i2c_x：IIC组
*				slave_addr：从机地址
*				reg_addr：寄存器地址
*				buf：需要写入的数据缓存
*				num：数据长度
*
*	返回参数：	0-写入成功	1-写入失败
*
*	说明：		*buf是一个数组或指向一个缓存区的指针
************************************************************
*/
_Bool I2C_WriteBytes(I2C_TypeDef *i2c_x, unsigned char slave_addr, unsigned char reg_addr, unsigned char *buf, unsigned char num)
{

	unsigned char addr = 0;

	addr = slave_addr << 1;														//IIC地址是7bit，这里需要左移1位，bit0：1-读	0-写
	
	IIC_Start(i2c_x);															//起始信号
	
#if(HW_I2C == 1)
	I2C_Send7bitAddress(i2c_x, addr, I2C_Direction_Transmitter);
	IIC_CHECK_EVENT(i2c_x, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);			//等待EV6
	IIC_CHECK_EVENT(i2c_x, I2C_EVENT_MASTER_BYTE_TRANSMITTING);					//等待EV8
#else
	IIC_SendByte(i2c_x, addr);													//发送设备地址(写)
	if(IIC_WaitAck(i2c_x, 5000))												//等待应答
		return IIC_Err;
#endif
	
	IIC_SendByte(i2c_x, reg_addr);												//发送寄存器地址
	if(IIC_WaitAck(i2c_x, 5000))												//等待应答
		return IIC_Err;
	
	while(num--)																//循环写入数据
	{
		IIC_SendByte(i2c_x, *buf);												//发送数据
		if(IIC_WaitAck(i2c_x, 5000))											//等待应答
			return IIC_Err;
		
		buf++;																	//数据指针偏移到下一个
		
		DelayUs(10);
	}
	
	IIC_Stop(i2c_x);															//停止信号
	
	return IIC_OK;

}

/*
************************************************************
*	函数名称：	I2C_ReadBytes
*
*	函数功能：	软件IIC读多个数据
*
*	入口参数：	i2c_x：IIC组
*				slave_addr：从机地址
*				reg_addr：寄存器地址
*				buf：需要读取的数据缓存
*				num：数据长度
*
*	返回参数：	0-写入成功	1-写入失败
*
*	说明：		*buf是一个数组或指向一个缓存区的指针
************************************************************
*/
_Bool I2C_ReadBytes(I2C_TypeDef *i2c_x, unsigned char slave_addr, unsigned char reg_addr, unsigned char *buf, unsigned char num)
{
	
	unsigned short addr = 0;

    addr = slave_addr << 1;														//IIC地址是7bit，这里需要左移1位，bit0：1-读	0-写

	IIC_Start(i2c_x);															//起始信号
	
#if(HW_I2C == 1)
	I2C_Send7bitAddress(i2c_x, addr, I2C_Direction_Transmitter);
	IIC_CHECK_EVENT(i2c_x, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);			//等待EV6
	IIC_CHECK_EVENT(i2c_x, I2C_EVENT_MASTER_BYTE_TRANSMITTING);					//等待EV8
#else
	IIC_SendByte(i2c_x, addr);													//发送设备地址(写)
	if(IIC_WaitAck(i2c_x, 5000))												//等待应答
		return IIC_Err;
#endif
	
	IIC_SendByte(i2c_x, reg_addr);												//发送寄存器地址
	if(IIC_WaitAck(i2c_x, 5000))												//等待应答
		return IIC_Err;
	
	IIC_Start(i2c_x);															//重启信号
	
#if(HW_I2C == 1)
	I2C_Send7bitAddress(i2c_x, addr, I2C_Direction_Receiver);
	IIC_CHECK_EVENT(i2c_x, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);			//等待EV6
	
	IIC_Ack(i2c_x);																//开启自动应答
#else
	IIC_SendByte(i2c_x, addr + 1);												//发送设备地址(读)
	if(IIC_WaitAck(i2c_x, 5000))												//等待应答
		return IIC_Err;
#endif
	
	while(num--)
	{
		*buf = IIC_RecvByte(i2c_x);
		buf++;																	//偏移到下一个数据存储地址
		
		if(num == 0)
			IIC_NAck(i2c_x);													//最后一个数据需要回NOACK
#if(HW_I2C == 0)
        else
			IIC_Ack(i2c_x);														//回应ACK
#endif
	}
	
	IIC_Stop(i2c_x);
	
	return IIC_OK;

}

#if(HW_I2C == 1)
#if(I2C_MASTER == 0)
/*
************************************************************
*	函数名称：	I2C1_EV_IRQHandler
*
*	函数功能：	I2C1事件中断
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void I2C1_EV_IRQHandler(void)
{
	
	RTOS_EnterInt();

	switch(I2C_GetLastEvent(I2C1))
	{
		//从机发送---------------------------------------------------------------------------------------------------------------

		case I2C_EVENT_SLAVE_TRANSMITTER_ADDRESS_MATCHED:	//EV1
		
			//地址匹配	EV1： ADDR=1，读SR1然后读SR2将清除该事件
			//在 I2C_GetLastEvent 里已经读取了SR1和SR2
			
			//重要：这是发送地址匹配，当主机发送从机地址+读时，从机匹配为发送地址
			iic_info.i2c1_rw_flag = 2;
		
		break;
		
		case I2C_EVENT_SLAVE_BYTE_TRANSMITTED:				//EV3_1
		
			
		
		break;

		case I2C_EVENT_SLAVE_BYTE_TRANSMITTING:				//EV3
		
			iic_info.i2c1_slave_send_cnt %= sizeof(iic_info.i2c1_slave_send_buf);
			
			I2C1->DR = iic_info.i2c1_slave_send_buf[iic_info.i2c1_slave_send_cnt++];
		
		break;
		
		case I2C_EVENT_SLAVE_ACK_FAILURE:					//EV3_2
		
			//手册中SR1-bit4-STOPF：从模式下，在收到NACK后， STOPF位不被置位
		
		break;

		//从机接收---------------------------------------------------------------------------------------------------------------

		case I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED:		//EV1
		
			//地址匹配	EV1： ADDR=1，读SR1然后读SR2将清除该事件
			//在 I2C_GetLastEvent 里已经读取了SR1和SR2
			
			//重要：这是接收地址匹配，当主机发送从机地址+写时，从机匹配为接收地址
			iic_info.i2c1_rw_flag = 1;
		
		break;

		case I2C_EVENT_SLAVE_BYTE_RECEIVED:					//EV2
		
			//接收数据	EV2： RxNE=1，读DR将清除该事件。
			
			iic_info.i2c1_slave_recv_cnt %= sizeof(iic_info.i2c1_slave_recv_buf);
			
			iic_info.i2c1_slave_recv_buf[iic_info.i2c1_slave_recv_cnt++] = (unsigned char)I2C1->DR;
		
		break;

		case I2C_EVENT_SLAVE_STOP_DETECTED:					//EV4
		
			//停止检测	EV4： STOPF=1，读SR1然后写CR1寄存器将清除该事件
			//在 I2C_GetLastEvent 里已经读取了SR1和SR2
			
			I2C1->CR1 |= 0x0001;							//重新使能I2C
			
			iic_info.i2c1_rw_flag = 0;
			iic_info.i2c1_stop_flag = 1;					//收到了停止信号，则可认为收到一帧完整数据
															//手册中SR1-bit4-STOPF：从模式下，在收到NACK后， STOPF位不被置位
		
		break;

		default:
		break;
	}
	
	RTOS_ExitInt();

}

/*
************************************************************
*	函数名称：	I2C2_EV_IRQHandler
*
*	函数功能：	I2C2事件中断
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void I2C2_EV_IRQHandler(void)
{
	
	RTOS_EnterInt();

	switch(I2C_GetLastEvent(I2C2))
	{
		//从机发送---------------------------------------------------------------------------------------------------------------

		case I2C_EVENT_SLAVE_TRANSMITTER_ADDRESS_MATCHED:	//EV1
		
			//地址匹配	EV1： ADDR=1，读SR1然后读SR2将清除该事件
			//在 I2C_GetLastEvent 里已经读取了SR1和SR2
			
			//重要：这是发送地址匹配，当主机发送从机地址+读时，从机匹配为发送地址
			iic_info.i2c2_rw_flag = 2;
		
		break;
		
		case I2C_EVENT_SLAVE_BYTE_TRANSMITTED:				//EV3_1
		
			
		
		break;

		case I2C_EVENT_SLAVE_BYTE_TRANSMITTING:				//EV3
		
			iic_info.i2c2_slave_send_cnt %= sizeof(iic_info.i2c2_slave_send_buf);
			
			I2C2->DR = iic_info.i2c2_slave_send_buf[iic_info.i2c2_slave_send_cnt++];
		
		break;
		
		case I2C_EVENT_SLAVE_ACK_FAILURE:					//EV3_2
		
			//手册中SR1-bit4-STOPF：从模式下，在收到NACK后， STOPF位不被置位
		
		break;

		//从机接收---------------------------------------------------------------------------------------------------------------

		case I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED:		//EV1
		
			//地址匹配	EV1： ADDR=1，读SR1然后读SR2将清除该事件
			//在 I2C_GetLastEvent 里已经读取了SR1和SR2
			
			//重要：这是接收地址匹配，当主机发送从机地址+写时，从机匹配为接收地址
			iic_info.i2c2_rw_flag = 1;
		
		break;

		case I2C_EVENT_SLAVE_BYTE_RECEIVED:					//EV2
		
			//接收数据	EV2： RxNE=1，读DR将清除该事件。
			
			iic_info.i2c2_slave_recv_cnt %= sizeof(iic_info.i2c2_slave_recv_buf);
			
			iic_info.i2c2_slave_recv_buf[iic_info.i2c2_slave_recv_cnt++] = (unsigned char)I2C2->DR;
		
		break;

		case I2C_EVENT_SLAVE_STOP_DETECTED:					//EV4
		
			//停止检测	EV4： STOPF=1，读SR1然后写CR1寄存器将清除该事件
			//在 I2C_GetLastEvent 里已经读取了SR1和SR2
			
			I2C2->CR1 |= 0x0001;							//重新使能I2C
			
			iic_info.i2c2_rw_flag = 0;
			iic_info.i2c2_stop_flag = 1;					//收到了停止信号，则可认为收到一帧完整数据
															//手册中SR1-bit4-STOPF：从模式下，在收到NACK后， STOPF位不被置位
		
		break;

		default:
		break;
	}
	
	RTOS_ExitInt();

}
#endif

/*
************************************************************
*	函数名称：	I2C1_ER_IRQHandler
*
*	函数功能：	I2C1错误中断
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void I2C1_ER_IRQHandler(void)
{
	
	RTOS_EnterInt();

	if(I2C_GetITStatus(I2C1, I2C_IT_BERR) == SET)			//总线错误
	{
		I2C_ClearITPendingBit(I2C1, I2C_IT_BERR);
		
		UsartPrintf(USART_DEBUG, "I2C1 Bus Error\r\n");
		
		if(I2C1->SR2 & 0x01)								//在主模式情况下，硬件不释放总线，同时不影响当前的传输状态。此时由软件决定是否要中止当前的传输
			IIC_Stop(I2C1);
	}
	
	if(I2C_GetITStatus(I2C1, I2C_IT_ARLO) == SET)			//仲裁丢失错误
	{
		I2C_ClearITPendingBit(I2C1, I2C_IT_ARLO);
		
		UsartPrintf(USART_DEBUG, "I2C1 Arbitration lost Error\r\n");
		
		//I2C接口自动回到从模式(M/SL位被清除).当I2C接口丢失了仲裁,则它无法在同一个传输中响应它的从地址,但它可以在赢得总线的主设备发送重起始条件之后响应
		//硬件释放总线
	}
	
	if(I2C_GetITStatus(I2C1, I2C_IT_AF) == SET)				//应答错误
	{
		I2C_ClearITPendingBit(I2C1, I2C_IT_AF);
		
//		UsartPrintf(USART_DEBUG, "I2C1 Acknowledge Error\r\n");
		
		if(I2C1->SR2 & 0x01)								//如果是处于主模式,软件必须生成一个停止条件
			IIC_Stop(I2C1);
		else
		{
#if(HW_I2C == 1 && I2C_MASTER == 0)
			if(iic_info.i2c1_rw_flag == 2)					//手册中SR1-bit4-STOPF：从模式下，在收到NACK后， STOPF位不被置位
			{
				iic_info.i2c1_rw_flag = 0;
				iic_info.i2c1_slave_send_cnt = 0;
				iic_info.i2c1_stop_flag = 1;
			}
#endif
		}
	}
	
	if(I2C_GetITStatus(I2C1, I2C_IT_OVR) == SET)			//过载/欠载错误
	{
		I2C_ClearITPendingBit(I2C1, I2C_IT_OVR);
		
		UsartPrintf(USART_DEBUG, "I2C1 Overrun/Underrun Error\r\n");
		
		I2C_ClearFlag(I2C2, I2C_FLAG_RXNE);					//在过载错误时,软件应清除RxNE位,发送器应该重新发送最后一次发送的字节
	}
	
	if(I2C_GetITStatus(I2C1, I2C_IT_PECERR) == SET)			//PEC校验错误
	{
		I2C_ClearITPendingBit(I2C1, I2C_IT_PECERR);
		
		UsartPrintf(USART_DEBUG, "I2C1 PEC Error\r\n");
		
		//校验错误不影响通信过程
	}
	
	if(I2C_GetITStatus(I2C1, I2C_IT_TIMEOUT) == SET)		//总线超时错误
	{
		I2C_ClearITPendingBit(I2C1, I2C_IT_TIMEOUT);
		
		UsartPrintf(USART_DEBUG, "I2C1 TimeOut Error\r\n");
		
		if(I2C1->SR2 & 0x01)								//由软件停止总线
			IIC_Stop(I2C1);
	}
	
	RTOS_ExitInt();

}

/*
************************************************************
*	函数名称：	I2C2_ER_IRQHandler
*
*	函数功能：	I2C2错误中断
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void I2C2_ER_IRQHandler(void)
{
	
	RTOS_EnterInt();

	if(I2C_GetITStatus(I2C2, I2C_IT_BERR) == SET)			//总线错误
	{
		I2C_ClearITPendingBit(I2C2, I2C_IT_BERR);
		
		UsartPrintf(USART_DEBUG, "I2C2 Bus Error\r\n");
		
		if(I2C2->SR2 & 0x01)								//在主模式情况下，硬件不释放总线，同时不影响当前的传输状态。此时由软件决定是否要中止当前的传输
			IIC_Stop(I2C2);
	}
	
	if(I2C_GetITStatus(I2C2, I2C_IT_ARLO) == SET)			//仲裁丢失错误
	{
		I2C_ClearITPendingBit(I2C2, I2C_IT_ARLO);
		
		UsartPrintf(USART_DEBUG, "I2C2 Arbitration lost Error\r\n");
		
		//I2C接口自动回到从模式(M/SL位被清除).当I2C接口丢失了仲裁,则它无法在同一个传输中响应它的从地址,但它可以在赢得总线的主设备发送重起始条件之后响应
		//硬件释放总线
	}
	
	if(I2C_GetITStatus(I2C2, I2C_IT_AF) == SET)				//应答错误
	{
		I2C_ClearITPendingBit(I2C2, I2C_IT_AF);
		
//		UsartPrintf(USART_DEBUG, "I2C2 Acknowledge Error\r\n");
		
		if(I2C2->SR2 & 0x01)								//如果是处于主模式,软件必须生成一个停止条件
			IIC_Stop(I2C2);
		else
		{
#if(HW_I2C == 1 && I2C_MASTER == 0)
			if(iic_info.i2c2_rw_flag == 2)					//手册中SR1-bit4-STOPF：从模式下，在收到NACK后， STOPF位不被置位
			{
				iic_info.i2c2_rw_flag = 0;
				iic_info.i2c2_slave_send_cnt = 0;
				iic_info.i2c2_stop_flag = 1;
			}
#endif
		}
	}
	
	if(I2C_GetITStatus(I2C2, I2C_IT_OVR) == SET)			//过载/欠载错误
	{
		I2C_ClearITPendingBit(I2C2, I2C_IT_OVR);
		
		UsartPrintf(USART_DEBUG, "I2C2 Overrun/Underrun Error\r\n");
		
		I2C_ClearFlag(I2C2, I2C_FLAG_RXNE);					//在过载错误时,软件应清除RxNE位,发送器应该重新发送最后一次发送的字节
	}
	
	if(I2C_GetITStatus(I2C2, I2C_IT_PECERR) == SET)			//PEC校验错误
	{
		I2C_ClearITPendingBit(I2C2, I2C_IT_PECERR);
		
		UsartPrintf(USART_DEBUG, "I2C2 PEC Error\r\n");
		
		//校验错误不影响通信过程
	}
	
	if(I2C_GetITStatus(I2C2, I2C_IT_TIMEOUT) == SET)		//总线超时错误
	{
		I2C_ClearITPendingBit(I2C2, I2C_IT_TIMEOUT);
		
		UsartPrintf(USART_DEBUG, "I2C2 TimeOut Error\r\n");
		
		if(I2C2->SR2 & 0x01)								//由软件停止总线
			IIC_Stop(I2C2);
	}
	
	RTOS_ExitInt();

}
#endif
