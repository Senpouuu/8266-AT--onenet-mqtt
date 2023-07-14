/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	ir.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2017-09-11
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		���ⷢ�͹�����
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

#if(HW_IR_EN == 0)
#include "mcu_gpio.h"
#endif

//Ӳ������
#include "ir.h"
#include "hwtimer.h"
#include "delay.h"

static _Bool ir_busy = 0;


#if(HW_IR_EN == 0)
unsigned short freq = 0;

const static GPIO_LIST ir_gpio_list[] = {
											{GPIOC, GPIO_Pin_9, NULL},
										};
#endif


/*
************************************************************
*	�������ƣ�	IR_IsBusReady
*
*	�������ܣ�	��ѯ�����Ƿ����
*
*	��ڲ�����	��
*
*	���ز�����	0-����	1-δ����
*
*	˵����		
************************************************************
*/
__inline static _Bool IR_IsBusReady(void)
{
	
	_Bool result = 1;

	RTOS_ENTER_CRITICAL();
	
	if(ir_busy == 0)
	{
		ir_busy = 1;
		
		result = 0;
	}
	
	RTOS_EXIT_CRITICAL();
	
	return result;

}

/*
************************************************************
*	�������ƣ�	IR_FreeBus
*
*	�������ܣ�	�ͷ�����
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
__inline static void IR_FreeBus(void)
{

	RTOS_ENTER_CRITICAL();
	
	ir_busy = 0;
	
	RTOS_EXIT_CRITICAL();

}

/*
************************************************************
*	�������ƣ�	IR_Init
*
*	�������ܣ�	IR-IO��ʼ��
*
*	��ڲ�����	hz��Ƶ��(HZ)
*
*	���ز�����	0-�ɹ�	1-ʧ��
*
*	˵����		TIM8-CH4-GPIOC9
*				TIM8��APB2�£�������APB2δ��Ƶ��Ϊ72MHz
*				ռ�ձȹ̶�Ϊ50%
************************************************************
*/
_Bool IR_Init(unsigned int hz)
{
	
#if(HW_IR_EN == 1)
	
	unsigned short arr = 0, psc = 0;
	
	if(hz > 1 && hz < 1000)						//1Hz~1KHz
	{
		arr = (unsigned short)((20000.0 / (float)hz) + 0.5);
		psc = 3599;
	}
	else if(hz >= 1000 && hz <= 1000000)		//1KHz~1MHz
	{
		arr = (unsigned short)((1000000.0 / (float)hz) + 0.5);
		psc = 69;
	}
	else
		return 1;

	Timer_X_PWM_Init(IR_PWM, 8, TIM_OCMode_PWM1, GPIOC, GPIO_Pin_9, psc, arr);
	TIM_SetCompare4(IR_PWM, arr >> 1);
	
	TIM_Cmd(IR_PWM, DISABLE);
	TIM_SetCounter(IR_PWM, 0);
	
#else
	
	if(sem_ir == NULL)
		sem_ir = xSemaphoreCreateMutex();
	
	MCU_GPIO_Init(ir_gpio_list[0].gpio_group, ir_gpio_list[0].gpio_pin, GPIO_Mode_Out_PP, GPIO_Speed_50MHz, ir_gpio_list[0].gpio_name);
	
	if(hz > 1 && hz < 1000)						//1Hz~1KHz
		freq = (unsigned short)((10000.0 / (float)hz) + 0.5);
	else if(hz >= 1000 && hz <= 1000000)		//1KHz~1MHz
		freq = (unsigned short)((500000.0 / (float)hz) + 0.5);
	else
		return 1;
	
#endif
	
	return 0;

}

/*
************************************************************
*	�������ƣ�	IR_SendFreq
*
*	�������ܣ�	�����������巢��
*
*	��ڲ�����	time��ʱ��(US)
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void IR_SendFreq(unsigned short time)
{
	
	if(IR_IsBusReady() == 0)
	{
#if(HW_IR_EN == 1)
		
		IR_PWM->CNT = 0;
		IR_PWM->CR1 |= TIM_CR1_CEN;
		
		if(time)
			DelayUs(time);
		
#else
		
		unsigned short count = 0;
		
		sem_ir = 1;
		
		for(; count < time;)
		{
			ir_gpio_list[0].gpio_group->BSRR = 1UL << ir_gpio_list[0].gpio_pin;
			DelayUs(freq);
			ir_gpio_list[0].gpio_group->BSRR = 1UL << (ir_gpio_list[0].gpio_pin + 16);
			DelayUs(freq);
			
			count += freq << 1;
		}
		
	#endif
		
		IR_FreeBus();
	}

}

/*
************************************************************
*	�������ƣ�	IR_SendStop
*
*	�������ܣ�	ֹͣ����Ƶ�ʷ���
*
*	��ڲ�����	time��ʱ��(US)
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void IR_SendStop(unsigned short time)
{
	
	if(IR_IsBusReady() == 0)
	{
#if(HW_IR_EN == 1)
		IR_PWM->CR1 &= (uint16_t)(~((uint16_t)TIM_CR1_CEN));
		IR_PWM->CNT = 0;
		
#else
		
		ir_gpio_list[0].gpio_group->BSRR = 1UL << (ir_gpio_list[0].gpio_pin + 16);
		
#endif
		
		if(time)
			DelayUs(time);
		
		IR_FreeBus();
	}

}
