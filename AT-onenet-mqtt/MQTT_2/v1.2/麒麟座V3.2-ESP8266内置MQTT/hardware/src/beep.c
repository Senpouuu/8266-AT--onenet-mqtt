/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	beep.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2017-02-27
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		BEEP��ʼ��������
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"

//��Ƭ��������
#include "mcu_gpio.h"

//����
#include "delay.h"
#include "beep.h"


BEEP_INFO beep_info;


const static GPIO_LIST beep_gpio_list[BEEP_NUM] = {
														{GPIOB, GPIO_Pin_3, "beep"},
													};


/*
************************************************************
*	�������ƣ�	BEEP_Init
*
*	�������ܣ�	��������ʼ��
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void BEEP_Init(void)
{
	
	unsigned char i = 0;
	
	MCU_GPIO_SWJ_JTAGDisable();
	
	for(; i < BEEP_NUM; i++)
		MCU_GPIO_Init(beep_gpio_list[i].gpio_group, beep_gpio_list[i].gpio_pin, GPIO_Mode_Out_PP, GPIO_Speed_50MHz, beep_gpio_list[i].gpio_name);
	
	BEEP_Ctl(BEEP_ALL, BEEP_OFF);

}

/*
************************************************************
*	�������ƣ�	BEEP_Ctl
*
*	�������ܣ�	����������
*
*	��ڲ�����	num��BEEP���
*				status������״̬
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void BEEP_Ctl(unsigned char num, BEEP_ENUM status)
{
	
	unsigned char i = 0;
	
	RTOS_ENTER_CRITICAL();
	
	if(num < BEEP_NUM)
	{
		MCU_GPIO_Output_Ctl(beep_gpio_list[num].gpio_name, status);
		
		beep_info.beep_status[num] = status;
	}
	else if(num == BEEP_ALL)
	{
		for(; i < BEEP_NUM; i++)
		{
			MCU_GPIO_Output_Ctl(beep_gpio_list[i].gpio_name, status);
			
			beep_info.beep_status[i] = status;
		}
	}
	
	RTOS_EXIT_CRITICAL();

}
