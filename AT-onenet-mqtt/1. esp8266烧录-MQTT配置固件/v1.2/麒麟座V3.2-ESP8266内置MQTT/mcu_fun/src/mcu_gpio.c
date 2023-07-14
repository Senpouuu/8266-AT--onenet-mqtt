/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	mcu_gpio.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2018-02-24
	*
	*	版本： 		V1.0
	*
	*	说明： 		单片机GPIO操作
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机相关组件
#include "mcu_rcc.h"
#include "mcu_gpio.h"


GPIO_DEVICE gpio_device;


/*
************************************************************
*	函数名称：	MCU_GPIO_Init
*
*	函数功能：	单片机GPIO初始化
*
*	入口参数：	GPIO_Group：GPIO组
*				GPIO_Pin：引脚
*				GPIO_Mode：io模式
*				GPIO_Speed：承载频率
*				GPIO_Name：io名称(自定义)
*
*	返回参数：	0-初始化ok，且加入设备列表
*				1-初始化ok，但未加入设备列表
*				255-初始化失败
*
*	说明：		当gpio_name为NULL的时候，不会加入gpio设备列表里
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
*	函数名称：	MCU_GPIO_SWJ_JTAGDisable
*
*	函数功能：	失能SWJ 和 JTAG功能
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
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
*	函数名称：	MCU_GPIO_FindIndex
*
*	函数功能：	根据名称在gpio设备列表里寻找索引
*
*	入口参数：	name：io名称(自定义)
*
*	返回参数：	255-失败
*
*	说明：		
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
*	函数名称：	MCU_GPIO_Output_Ctl
*
*	函数功能：	gpio输出控制
*
*	入口参数：	name：io名称(自定义)
*				status：1-输出高电平		0-输出低电平
*
*	返回参数：	0-成功	1-失败
*
*	说明：		
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
*	函数名称：	MCU_GPIO_Output_Group_Ctl
*
*	函数功能：	gpio组输出控制
*
*	入口参数：	GPIO_Group：GPIO组
*				port_value：端口值
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void MCU_GPIO_Output_Group_Ctl(GPIO_TypeDef *GPIO_Group, unsigned short port_value)
{
	
	GPIO_Group->ODR = port_value;

}

/*
************************************************************
*	函数名称：	MCU_GPIO_Input_Read
*
*	函数功能：	gpio读取输入
*
*	入口参数：	name：io名称(自定义)
*
*	返回参数：	0-低电平		1-高电平		255-读取失败
*
*	说明：		
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
*	函数名称：	MCU_GPIO_Input_Group_Read
*
*	函数功能：	gpio读取输入
*
*	入口参数：	GPIO_Group：GPIO组
*
*	返回参数：	端口值
*
*	说明：		
************************************************************
*/
unsigned short MCU_GPIO_Input_Group_Read(GPIO_TypeDef *GPIO_Group)
{

	return GPIO_Group->IDR;

}
