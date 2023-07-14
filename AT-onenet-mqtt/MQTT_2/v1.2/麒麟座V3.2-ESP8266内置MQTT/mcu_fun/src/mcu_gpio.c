/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	mcu_gpio.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2018-02-24
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		��Ƭ��GPIO����
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��������
#include "mcu_rcc.h"
#include "mcu_gpio.h"


GPIO_DEVICE gpio_device;


/*
************************************************************
*	�������ƣ�	MCU_GPIO_Init
*
*	�������ܣ�	��Ƭ��GPIO��ʼ��
*
*	��ڲ�����	GPIO_Group��GPIO��
*				GPIO_Pin������
*				GPIO_Mode��ioģʽ
*				GPIO_Speed������Ƶ��
*				GPIO_Name��io����(�Զ���)
*
*	���ز�����	0-��ʼ��ok���Ҽ����豸�б�
*				1-��ʼ��ok����δ�����豸�б�
*				255-��ʼ��ʧ��
*
*	˵����		��gpio_nameΪNULL��ʱ�򣬲������gpio�豸�б���
************************************************************
*/
unsigned char MCU_GPIO_Init(GPIO_TypeDef *GPIO_Group, unsigned short GPIO_Pin, GPIOMode_TypeDef GPIO_Mode,
								GPIOSpeed_TypeDef GPIO_Speed, char *GPIO_Name)
{

	unsigned char result = 255;
	GPIO_InitTypeDef gpio_initstruct;
	
	if(MCU_RCC(GPIO_Group, ENABLE))
		return result;
	
	gpio_initstruct.GPIO_Mode = GPIO_Mode;
	gpio_initstruct.GPIO_Pin = GPIO_Pin;
	gpio_initstruct.GPIO_Speed = GPIO_Speed;
	
	GPIO_Init(GPIO_Group, &gpio_initstruct);
	
	if(GPIO_Name && gpio_device.count < GPIO_GROUP_NUM)
	{
		gpio_device.name[gpio_device.count] = GPIO_Name;
		gpio_device.gpio_group[gpio_device.count] = GPIO_Group;
		gpio_device.gpio_pin[gpio_device.count] = GPIO_Pin;
		gpio_device.count++;
		
		result = 0;
	}
	else
		result = 1;
	
	return result;

}

/*
************************************************************
*	�������ƣ�	MCU_GPIO_SWJ_JTAGDisable
*
*	�������ܣ�	ʧ��SWJ �� JTAG����
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void MCU_GPIO_SWJ_JTAGDisable(void)
{

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, DISABLE);

}

/*
************************************************************
*	�������ƣ�	MCU_GPIO_FindIndex
*
*	�������ܣ�	����������gpio�豸�б���Ѱ������
*
*	��ڲ�����	name��io����(�Զ���)
*
*	���ز�����	255-ʧ��
*
*	˵����		
************************************************************
*/
unsigned char MCU_GPIO_FindIndex(char *name)
{

	unsigned char i = 0;
	char *name_ptr = gpio_device.name[0];
	char *name_ptr_t = name;
	
	for(; i < gpio_device.count;)
	{
		while(1)
		{
			if(*name_ptr == '\0')
				return i;
			
			if(*name_ptr++ != *name_ptr_t++)
			{
				name_ptr = gpio_device.name[++i];
				name_ptr_t = name;
				
				break;
			}
		}
	}
	
	return 255;

}

/*
************************************************************
*	�������ƣ�	MCU_GPIO_Output_Ctl
*
*	�������ܣ�	gpio�������
*
*	��ڲ�����	name��io����(�Զ���)
*				status��1-����ߵ�ƽ		0-����͵�ƽ
*
*	���ز�����	0-�ɹ�	1-ʧ��
*
*	˵����		
************************************************************
*/
_Bool MCU_GPIO_Output_Ctl(char *name, _Bool status)
{
	
	unsigned char i = 255;
	_Bool result = 1;

	if(!name)
		return result;
	
	i = MCU_GPIO_FindIndex(name);
	
	if(i < gpio_device.count)
	{
		if(status)
			gpio_device.gpio_group[i]->BSRR = gpio_device.gpio_pin[i];
		else
			gpio_device.gpio_group[i]->BRR = gpio_device.gpio_pin[i];
		
		result = 0;
	}
	
	return result;

}

/*
************************************************************
*	�������ƣ�	MCU_GPIO_Output_Group_Ctl
*
*	�������ܣ�	gpio���������
*
*	��ڲ�����	GPIO_Group��GPIO��
*				port_value���˿�ֵ
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void MCU_GPIO_Output_Group_Ctl(GPIO_TypeDef *GPIO_Group, unsigned short port_value)
{
	
	GPIO_Group->ODR = port_value;

}

/*
************************************************************
*	�������ƣ�	MCU_GPIO_Input_Read
*
*	�������ܣ�	gpio��ȡ����
*
*	��ڲ�����	name��io����(�Զ���)
*
*	���ز�����	0-�͵�ƽ		1-�ߵ�ƽ		255-��ȡʧ��
*
*	˵����		
************************************************************
*/
unsigned char MCU_GPIO_Input_Read(char *name)
{

	unsigned char i = 255, result = 255;

	if(!name)
		return result;
	
	i = MCU_GPIO_FindIndex(name);
	
	if(i < gpio_device.count)
	{
		if(gpio_device.gpio_group[i]->IDR & gpio_device.gpio_pin[i])
			result = 1;
		else
			result = 0;
	}
	
	return result;

}

/*
************************************************************
*	�������ƣ�	MCU_GPIO_Input_Group_Read
*
*	�������ܣ�	gpio��ȡ����
*
*	��ڲ�����	GPIO_Group��GPIO��
*
*	���ز�����	�˿�ֵ
*
*	˵����		
************************************************************
*/
unsigned short MCU_GPIO_Input_Group_Read(GPIO_TypeDef *GPIO_Group)
{

	return GPIO_Group->IDR;

}
