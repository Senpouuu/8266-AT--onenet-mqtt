#ifndef _I2C_H_
#define _I2C_H_


//单片机头文件
#include "stm32f10x.h"


/**********************************************************************

					I2C从模式流程说明(全中断方式)
					
Write：1.主机Start，从机准备匹配地址
		2.主机发送地址(写)，从机匹配地址
			匹配成功则触发 I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED 中断
			匹配失败则不触发任何中断
		3.主机发送数据，从机触发 I2C_EVENT_SLAVE_BYTE_RECEIVED 中断，可在
			此中断里往buf写数据
		4.主机Stop，从机触发 I2C_EVENT_SLAVE_STOP_DETECTED 中断

Read：1.主机Start，从机准备匹配地址
		2.主机发送地址(写)，从机匹配地址
			匹配成功则触发 I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED 中断
			匹配失败则不触发任何中断
		3.主机发送数据，从机触发 I2C_EVENT_SLAVE_BYTE_RECEIVED 中断，可在
			此中断里往buf写数据
		4.主机ReStart，从机准备匹配地址
		5.主机发送地址(读)，从机匹配地址
			匹配成功则触发 I2C_EVENT_SLAVE_TRANSMITTER_ADDRESS_MATCHED 中断
			匹配失败则不触发任何中断
		6.主机读取数据，从机触发 I2C_EVENT_SLAVE_BYTE_TRANSMITTING 中断，可
			在此中断里发送数据
		7.当读取单字节或多字节时，最后主机发送NAcK信号，此时从机触发 应答错误
			中断
		8.主机发Stop，从机不触发任何中断
		注：2、3、4步骤可能没有，根据主机逻辑而定

					
					I2C从模式使用说明(全中断方式)

i2c1(2)_stop_flag：在主机发送stop信号或主机在读最后一个数据发送NAcK时置1，

					且在收到NACK后， SSR1-bit4-STOPF位不被置位


Write：当主机发起写操作，会把数据缓存在 i2c1(2)_slave_recv_buf 里，数据

		长度保存在 i2c1(2)_slave_send_cnt，然后 i2c1(2)_stop_flag 置1，
		
		表示一帧数据接收完成，在主任务里可循环检查stopflag来处理数据，然
		
		后把 i2c1(2)_stop_flag 和 i2c1(2)_slave_send_cnt 清0。


Read：当主机发起读操作，根据主机逻辑在应用层有不同的处理方式，如AT24C02的

		读逻辑，主机：器件地址(写)+存储地址+重启信号+器件地址(读)+接收+
		
		停止信号，stopflag只在整个流程完成之后才置1，在当i2c1(2)_rw_flag
		
		为2的时候，代表主机是读操作，此时可以获取i2c1(2)_slave_recv_buf[0]
		
		的值来判断存储地址，然后会触发 I2C_EVENT_SLAVE_BYTE_TRANSMITTING
		
		中断来将i2c1(2)_slave_send_buf里边的数据一个字节一个字节的发送给主
		
		机，最好提前准备好i2c1(2)_slave_send_buf里边的数据。
		
		那么当没有器件地址(写)+存储地址这两个步骤时，在主机直接读的时候，也
		
		会让i2c1(2)_rw_flag为2，但是i2c1(2)_slave_recv_buf里边没有任何数据
		
		会直接触发I2C_EVENT_SLAVE_BYTE_TRANSMITTING中断来发送。


注：最好保证从机接收的数据能够被实时处理掉

**********************************************************************/

#define HW_I2C				1		//1-硬件I2C		0-软件I2C
#if(HW_I2C == 1)
	#define I2C_MASTER		1		//1-主模式		0-从模式
	#if(I2C_MASTER == 0)
		#define SLAVE_ADDR	0x05	//从模式下本机 7bit 地址
	#endif
#endif


#define IIC_OK		0

#define IIC_Err		1


#if(HW_I2C == 1)
#define IIC_CHECK_EVENT(i2c_x, event)		{																		\
												unsigned short time_out = 5000;										\
												while((I2C_CheckEvent(i2c_x, event) == ERROR) && --time_out)		\
													DelayUs(1);														\
												if(!time_out)														\
													return IIC_Err;													\
											}
#else
//单片机相关组件
#include "mcu_gpio.h"

//I2C1	SDA		PB7
//I2C1	SCL		PB6
//I2C2	SDA		PB11
//I2C2	SCL		PB10
#define IIC_SDA_H(i2c_x)	i2c_x == I2C1 ? (i2c_gpio_list[1].gpio_group->BSRR = i2c_gpio_list[1].gpio_pin) : (i2c_gpio_list[3].gpio_group->BSRR = i2c_gpio_list[3].gpio_pin)
#define IIC_SDA_L(i2c_x)	i2c_x == I2C1 ? (i2c_gpio_list[1].gpio_group->BRR = i2c_gpio_list[1].gpio_pin) : (i2c_gpio_list[3].gpio_group->BRR = i2c_gpio_list[3].gpio_pin)
#define IIC_SDA_R(i2c_x)	MCU_GPIO_Input_Read(i2c_x == I2C1 ? (i2c_gpio_list[1].gpio_name) : (i2c_gpio_list[3].gpio_name))

#define IIC_SCL_H(i2c_x)	i2c_x == I2C1 ? (i2c_gpio_list[0].gpio_group->BSRR = i2c_gpio_list[0].gpio_pin) : (i2c_gpio_list[2].gpio_group->BSRR = i2c_gpio_list[2].gpio_pin)
#define IIC_SCL_L(i2c_x)	i2c_x == I2C1 ? (i2c_gpio_list[0].gpio_group->BRR = i2c_gpio_list[0].gpio_pin) : (i2c_gpio_list[2].gpio_group->BRR = i2c_gpio_list[2].gpio_pin)


extern const GPIO_LIST i2c_gpio_list[4];
#endif


typedef struct
{
	
	unsigned short i2c1_speed;
	unsigned short i2c2_speed;
	
#if(HW_I2C == 1 && I2C_MASTER == 0)
	unsigned char i2c1_slave_recv_buf[16];
	unsigned char i2c2_slave_recv_buf[16];
	
	unsigned char i2c1_slave_send_buf[16];
	unsigned char i2c2_slave_send_buf[16];
	
	unsigned short i2c1_slave_recv_cnt;
	unsigned short i2c2_slave_recv_cnt;
	
	unsigned short i2c1_slave_send_cnt;
	unsigned short i2c2_slave_send_cnt;
	
	unsigned char i2c1_stop_flag : 1;		//收到停止信号
	unsigned char i2c1_rw_flag : 2;			//主机端的读写标志	0-空闲	1-主机为写	2-主机为读
	unsigned char i2c2_stop_flag : 1;		//收到停止信号
	unsigned char i2c2_rw_flag : 2;			//主机端的读写标志	0-空闲	1-主机为写	2-主机为读
	unsigned char reverse : 2;				//保留
#endif

} IIC_INFO;

extern IIC_INFO iic_info;


_Bool IIC_Init(I2C_TypeDef *i2c_x);

void IIC_SpeedCtl(I2C_TypeDef *i2c_x, unsigned short speed);

void IIC_DelayUs(I2C_TypeDef *i2c_x);

_Bool IIC_IsBusReady(I2C_TypeDef *i2c_x);

void IIC_FreeBus(I2C_TypeDef *i2c_x);

_Bool I2C_WriteByte(I2C_TypeDef *i2c_x, unsigned char slave_addr, unsigned char reg_addr, unsigned char *byte);

_Bool I2C_ReadByte(I2C_TypeDef *i2c_x, unsigned char slave_addr, unsigned char reg_addr, unsigned char *val);

_Bool I2C_WriteBytes(I2C_TypeDef *i2c_x, unsigned char slave_addr, unsigned char reg_addr, unsigned char *buf, unsigned char num);

_Bool I2C_ReadBytes(I2C_TypeDef *i2c_x, unsigned char slave_addr, unsigned char reg_addr, unsigned char *buf, unsigned char num);

void IIC_Start(I2C_TypeDef *i2c_x);

void IIC_Stop(I2C_TypeDef *i2c_x);

_Bool IIC_WaitAck(I2C_TypeDef *i2c_x, unsigned int time_out);

void IIC_Ack(I2C_TypeDef *i2c_x);

void IIC_NAck(I2C_TypeDef *i2c_x);

_Bool IIC_SendByte(I2C_TypeDef *i2c_x, unsigned char byte);

unsigned char IIC_RecvByte(I2C_TypeDef *i2c_x);


#endif
