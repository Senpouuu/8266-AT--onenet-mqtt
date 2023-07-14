/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	key.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2018-02-27
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		����IO��ʼ�������������ж�
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

//����ͷ�ļ�
#include "key.h"

//Ӳ������
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
*	�������ƣ�	KEY_Init
*
*	�������ܣ�	����IO��ʼ��
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		����Ϊ�͵�ƽ		�ͷ�Ϊ�ߵ�ƽ
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
*	�������ƣ�	KEY_Scan
*
*	�������ܣ�	������ƽɨ��
*
*	��ڲ�����	num���������
*
*	���ز�����	����״̬
*
*	˵����		
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
*	�������ƣ�	Keyboard
*
*	�������ܣ�	�������ܼ��
*
*	��ڲ�����	��
*
*	���ز�����	������ֵ
*
*	˵����		�ֵ�����˫��������
************************************************************
*/
unsigned char Keyboard(void)
{
	
	static unsigned int key_busyflag = 0;									//�������ڷ��ͷ�״̬
	static unsigned char key_count = 0;										//��������ʱ��
	unsigned char time_out = 15, i = 0;										//�ж�˫����������Ҫ����ʱ���
	
	for(; i < KEY_NUM; i++)
	{
		if(KEY_Scan(i) == KEY_DOWN && !(key_busyflag & (~(1 << i))))		//������� ����������δ����
		{
			key_busyflag |= 1 << i;											//�˰�������æ״̬
			
			if(++key_count >= KEYDOWN_LONG_TIME)							//���¼�ʱ
				key_count = KEYDOWN_LONG_TIME;								//�ﵽ����ʱ���򲻱�
			
			return KEY_NONE;												//�����޶���״̬
		}
		else if(KEY_Scan(i) == KEY_UP && key_busyflag & (1 << i))			//����ͷ� �� ����֮ǰ�ǰ��¹���
		{
			key_busyflag &= ~(1 << i);										//�˰������ڿ���״̬
			
			if(key_count == KEYDOWN_LONG_TIME)								//����ǳ���
			{
				key_count = 0;												//���¼�ʱ����
				return key_event[i][KEY_X_DOWNLONG];						//���س�������
			}
			else
			{
				key_count = 0;												//���¼�ʱ����
				while(--time_out)											//������Ҫ�ǵȴ�Լ250ms���ж��Ƿ��еڶ��ΰ���
				{
					RTOS_TimeDly(2);										//�ô������������̬���ⲻӰ���������������
					
					if(KEY_Scan(i) == KEY_DOWN)								//�еڶ��ΰ��£�˵��Ϊ˫��
					{
						while(KEY_Scan(i) == KEY_DOWN)						//�ȴ��ͷţ��޴˾䣬˫������һ����������
							RTOS_TimeDly(1);								//�ô������������̬���ⲻӰ���������������
						
						return key_event[i][KEY_X_DOUBLE];					//����˫������
					}
					
				}
				
				return key_event[i][KEY_X_DOWN];							//�����жϾ���Ч����Ϊ��������
			}
		}
	}
	
	key_busyflag = 0;
	key_count = 0;
	
	return KEY_NONE;
	
}
