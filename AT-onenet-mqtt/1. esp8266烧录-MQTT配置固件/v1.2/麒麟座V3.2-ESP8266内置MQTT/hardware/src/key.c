/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	key.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2018-02-27
	*
	*	版本： 		V1.0
	*
	*	说明： 		按键IO初始化，按键功能判断
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f10x.h"

//单片机相关组件
#include "mcu_gpio.h"

//按键头文件
#include "key.h"

//硬件驱动
#include "delay.h"


unsigned char key_event[KEY_NUM][4];


const static GPIO_LIST key_gpio_list[KEY_NUM] = {
													{GPIOC, GPIO_Pin_13, "key1"},
													{GPIOC, GPIO_Pin_11, "key2"},
													{GPIOC, GPIO_Pin_12, "key3"},
													{GPIOD, GPIO_Pin_2, "key4"},
												};


/*
************************************************************
*	函数名称：	KEY_Init
*
*	函数功能：	按键IO初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		按下为低电平		释放为高电平
************************************************************
*/
void KEY_Init(void)
{
	
	unsigned char i = 0, j = 0, event_num = 0;
	
	for(; i < KEY_NUM; i++)
	{
		MCU_GPIO_Init(key_gpio_list[i].gpio_group, key_gpio_list[i].gpio_pin, GPIO_Mode_IPU, GPIO_Speed_50MHz, key_gpio_list[i].gpio_name);
		
		for(j = 0; j < 4; j++)
			key_event[i][j] = event_num++;
	}

}

/*
************************************************************
*	函数名称：	KEY_Scan
*
*	函数功能：	按键电平扫描
*
*	入口参数：	num：按键编号
*
*	返回参数：	按键状态
*
*	说明：		
************************************************************
*/
_Bool KEY_Scan(unsigned int num)
{
	
	unsigned char key_event_s = KEY_NONE;
	
	if(num >= KEY_NUM)
		return key_event_s;
	
	key_event_s = MCU_GPIO_Input_Read(key_gpio_list[num].gpio_name);
	
	if(key_event_s == 1)
		return KEY_UP;
	else if(key_event_s == 0)
		return KEY_DOWN;
	else
		return KEY_NONE;
	
}

/*
************************************************************
*	函数名称：	Keyboard
*
*	函数功能：	按键功能检测
*
*	入口参数：	无
*
*	返回参数：	按键键值
*
*	说明：		分单击、双击、长安
************************************************************
*/
unsigned char Keyboard(void)
{
	
	static unsigned int key_busyflag = 0;									//按键处于非释放状态
	static unsigned char key_count = 0;										//按键按下时间
	unsigned char time_out = 15, i = 0;										//判断双击动作所需要的延时间隔
	
	for(; i < KEY_NUM; i++)
	{
		if(KEY_Scan(i) == KEY_DOWN && !(key_busyflag & (~(1 << i))))		//如果按下 且其他按键未按下
		{
			key_busyflag |= 1 << i;											//此按键处于忙状态
			
			if(++key_count >= KEYDOWN_LONG_TIME)							//按下计时
				key_count = KEYDOWN_LONG_TIME;								//达到长按时长则不变
			
			return KEY_NONE;												//返回无动作状态
		}
		else if(KEY_Scan(i) == KEY_UP && key_busyflag & (1 << i))			//如果释放 且 按键之前是按下过的
		{
			key_busyflag &= ~(1 << i);										//此按键处于空闲状态
			
			if(key_count == KEYDOWN_LONG_TIME)								//如果是长按
			{
				key_count = 0;												//按下计时清零
				return key_event[i][KEY_X_DOWNLONG];						//返回长按动作
			}
			else
			{
				key_count = 0;												//按下计时清零
				while(--time_out)											//这里主要是等待约250ms，判断是否有第二次按下
				{
					RTOS_TimeDly(2);										//让此任务进入阻塞态，这不影响代码正常的运行
					
					if(KEY_Scan(i) == KEY_DOWN)								//有第二次按下，说明为双击
					{
						while(KEY_Scan(i) == KEY_DOWN)						//等待释放，无此句，双击后会跟一个单击动作
							RTOS_TimeDly(1);								//让此任务进入阻塞态，这不影响代码正常的运行
						
						return key_event[i][KEY_X_DOUBLE];					//返回双击动作
					}
					
				}
				
				return key_event[i][KEY_X_DOWN];							//以上判断均无效，则为单击动作
			}
		}
	}
	
	key_busyflag = 0;
	key_count = 0;
	
	return KEY_NONE;
	
}
