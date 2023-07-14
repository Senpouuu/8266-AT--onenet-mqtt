#ifndef _I2C_H_
#define _I2C_H_


//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"


/**********************************************************************

					I2C��ģʽ����˵��(ȫ�жϷ�ʽ)
					
Write��1.����Start���ӻ�׼��ƥ���ַ
		2.�������͵�ַ(д)���ӻ�ƥ���ַ
			ƥ��ɹ��򴥷� I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED �ж�
			ƥ��ʧ���򲻴����κ��ж�
		3.�����������ݣ��ӻ����� I2C_EVENT_SLAVE_BYTE_RECEIVED �жϣ�����
			���ж�����bufд����
		4.����Stop���ӻ����� I2C_EVENT_SLAVE_STOP_DETECTED �ж�

Read��1.����Start���ӻ�׼��ƥ���ַ
		2.�������͵�ַ(д)���ӻ�ƥ���ַ
			ƥ��ɹ��򴥷� I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED �ж�
			ƥ��ʧ���򲻴����κ��ж�
		3.�����������ݣ��ӻ����� I2C_EVENT_SLAVE_BYTE_RECEIVED �жϣ�����
			���ж�����bufд����
		4.����ReStart���ӻ�׼��ƥ���ַ
		5.�������͵�ַ(��)���ӻ�ƥ���ַ
			ƥ��ɹ��򴥷� I2C_EVENT_SLAVE_TRANSMITTER_ADDRESS_MATCHED �ж�
			ƥ��ʧ���򲻴����κ��ж�
		6.������ȡ���ݣ��ӻ����� I2C_EVENT_SLAVE_BYTE_TRANSMITTING �жϣ���
			�ڴ��ж��﷢������
		7.����ȡ���ֽڻ���ֽ�ʱ�������������NAcK�źţ���ʱ�ӻ����� Ӧ�����
			�ж�
		8.������Stop���ӻ��������κ��ж�
		ע��2��3��4�������û�У����������߼�����

					
					I2C��ģʽʹ��˵��(ȫ�жϷ�ʽ)

i2c1(2)_stop_flag������������stop�źŻ������ڶ����һ�����ݷ���NAcKʱ��1��

					�����յ�NACK�� SSR1-bit4-STOPFλ������λ


Write������������д������������ݻ����� i2c1(2)_slave_recv_buf �����

		���ȱ����� i2c1(2)_slave_send_cnt��Ȼ�� i2c1(2)_stop_flag ��1��
		
		��ʾһ֡���ݽ�����ɣ������������ѭ�����stopflag���������ݣ�Ȼ
		
		��� i2c1(2)_stop_flag �� i2c1(2)_slave_send_cnt ��0��


Read����������������������������߼���Ӧ�ò��в�ͬ�Ĵ���ʽ����AT24C02��

		���߼���������������ַ(д)+�洢��ַ+�����ź�+������ַ(��)+����+
		
		ֹͣ�źţ�stopflagֻ�������������֮�����1���ڵ�i2c1(2)_rw_flag
		
		Ϊ2��ʱ�򣬴��������Ƕ���������ʱ���Ի�ȡi2c1(2)_slave_recv_buf[0]
		
		��ֵ���жϴ洢��ַ��Ȼ��ᴥ�� I2C_EVENT_SLAVE_BYTE_TRANSMITTING
		
		�ж�����i2c1(2)_slave_send_buf��ߵ�����һ���ֽ�һ���ֽڵķ��͸���
		
		���������ǰ׼����i2c1(2)_slave_send_buf��ߵ����ݡ�
		
		��ô��û��������ַ(д)+�洢��ַ����������ʱ��������ֱ�Ӷ���ʱ��Ҳ
		
		����i2c1(2)_rw_flagΪ2������i2c1(2)_slave_recv_buf���û���κ�����
		
		��ֱ�Ӵ���I2C_EVENT_SLAVE_BYTE_TRANSMITTING�ж������͡�


ע����ñ�֤�ӻ����յ������ܹ���ʵʱ�����

**********************************************************************/

#define HW_I2C				1		//1-Ӳ��I2C		0-���I2C
#if(HW_I2C == 1)
	#define I2C_MASTER		1		//1-��ģʽ		0-��ģʽ
	#if(I2C_MASTER == 0)
		#define SLAVE_ADDR	0x05	//��ģʽ�±��� 7bit ��ַ
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
//��Ƭ��������
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
	
	unsigned char i2c1_stop_flag : 1;		//�յ�ֹͣ�ź�
	unsigned char i2c1_rw_flag : 2;			//�����˵Ķ�д��־	0-����	1-����Ϊд	2-����Ϊ��
	unsigned char i2c2_stop_flag : 1;		//�յ�ֹͣ�ź�
	unsigned char i2c2_rw_flag : 2;			//�����˵Ķ�д��־	0-����	1-����Ϊд	2-����Ϊ��
	unsigned char reverse : 2;				//����
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
