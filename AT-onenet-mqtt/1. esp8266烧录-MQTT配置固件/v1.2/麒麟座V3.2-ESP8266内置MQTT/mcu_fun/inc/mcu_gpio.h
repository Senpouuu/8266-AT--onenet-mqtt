#ifndef _MCU_GPIO_H_
#define _MCU_GPIO_H_


//单片机头文件
#include "stm32f10x.h"


#define GPIO_GROUP_NUM			16


typedef struct
{

	char *name[GPIO_GROUP_NUM];
	GPIO_TypeDef *gpio_group[GPIO_GROUP_NUM];
	unsigned short gpio_pin[GPIO_GROUP_NUM];
	
	unsigned char count;
	

} GPIO_DEVICE;


typedef struct
{

	GPIO_TypeDef *gpio_group;
	
	unsigned short gpio_pin;
	
	char *gpio_name;

} GPIO_LIST;


unsigned char MCU_GPIO_Init(GPIO_TypeDef *gpio_group, unsigned short gpio_pin, GPIOMode_TypeDef gpio_mode, GPIOSpeed_TypeDef gpio_speed, char *gpio_name);

void MCU_GPIO_SWJ_JTAGDisable(void);

_Bool MCU_GPIO_Output_Ctl(char *name, _Bool status);

void MCU_GPIO_Output_Group_Ctl(GPIO_TypeDef *GPIO_Group, unsigned short port_value);

unsigned char MCU_GPIO_Input_Read(char *name);

unsigned short MCU_GPIO_Input_Group_Read(GPIO_TypeDef *GPIO_Group);


#endif
