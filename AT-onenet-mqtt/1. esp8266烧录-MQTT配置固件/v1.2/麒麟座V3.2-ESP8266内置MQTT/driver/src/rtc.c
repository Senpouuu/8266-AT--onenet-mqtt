/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	rtc.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2017-11-09
	*
	*	�汾�� 		V1.1
	*
	*	˵���� 		RTC��ʼ���͹���
	*
	*	�޸ļ�¼��	V1.1���޸�RTC����ʹ�õ�BUG
						  ����RTC�жϿ��ƹ���
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"

//��Ƭ��������
#include "mcu_nvic.h"

//Ӳ������
#include "delay.h"
#include "rtc.h"


static _Bool rtc_busy = 0;


RTC_INFO rtc_info;


/*
************************************************************
*	�������ƣ�	RTC_IsBusReady
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
__inline static _Bool RTC_IsBusReady(void)
{
	
	_Bool result = 1;

	RTOS_ENTER_CRITICAL();
	
	if(rtc_busy == 0)
	{
		rtc_busy = 1;
		
		result = 0;
	}
	
	RTOS_EXIT_CRITICAL();
	
	return result;

}

/*
************************************************************
*	�������ƣ�	RTC_FreeBus
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
__inline static void RTC_FreeBus(void)
{

	RTOS_ENTER_CRITICAL();
	
	rtc_busy = 0;
	
	RTOS_EXIT_CRITICAL();

}

/*
************************************************************
*	�������ƣ�	RTC_Init
*
*	�������ܣ�	RTC��ʼ��
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
_Bool RTC_Init(void)
{
	
#if(USE_EXT_RCC == 1)
	unsigned char err_count = 0;
#endif

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//ʹ��PWR��BKP����ʱ��   
	PWR_BackupAccessCmd(ENABLE);												//ʹ�ܺ󱸼Ĵ�������
	
	BKP_DeInit();																//��λ��������
#if(USE_EXT_RCC == 1)
	RCC_LSEConfig(RCC_LSE_ON);													//�����ⲿ���پ���(LSE),ʹ��������پ���
	
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET && err_count < 250)		//���ָ����RCC��־λ�������,�ȴ����پ������
	{
		err_count++;
		RTOS_TimeDly(2);
	}
	
	if(err_count >= 250)
		return 1;																//��ʼ��ʱ��ʧ��,����������
#endif
	
#if(USE_EXT_RCC == 1)
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);										//����RTCʱ��(RTCCLK),ѡ��LSE��ΪRTCʱ��
#else
	RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);								//����RTCʱ��(HSE/128),ѡ��HES��ΪRTCʱ��
#endif
	RCC_RTCCLKCmd(ENABLE);														//ʹ��RTCʱ��
	RTC_WaitForLastTask();														//�ȴ����һ�ζ�RTC�Ĵ�����д�������
	RTC_WaitForSynchro();														//�ȴ�RTC�Ĵ���ͬ��
	
	RTC_ITConfig(RTC_IT_ALR, ENABLE);											//ʹ��RTC�����ж�
	RTC_WaitForLastTask();														//�ȴ����һ�ζ�RTC�Ĵ�����д�������
	
	RTC_EnterConfigMode();														//��������
#if(USE_EXT_RCC == 1)
	RTC_SetPrescaler(32767);													//����RTCԤ��Ƶ��ֵ
#else
	RTC_SetPrescaler(HSE_VALUE / 128 - 1);										//����RTCԤ��Ƶ��ֵ
#endif
	RTC_WaitForLastTask();														//�ȴ����һ�ζ�RTC�Ĵ�����д�������
	
	RTC_SetCounter(0);															//����RTC��������ֵ
	RTC_WaitForLastTask();														//�ȴ����һ�ζ�RTC�Ĵ�����д�������
	
	RTC_ExitConfigMode();														//�˳�����ģʽ
	
	RTC_NVIC_Init();
	
	return 0;

}

/*
************************************************************
*	�������ƣ�	RTC_NVIC_Init
*
*	�������ܣ�	RTC�жϳ�ʼ��
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void RTC_NVIC_Init(void)
{
	
#if(RTC_ALRE_INT_EN == 1)
	EXTI_InitTypeDef exti_initstruct;
#endif
	
#if(RTC_SEC_INT_EN == 1)
	RTC_ITConfig(RTC_IT_SEC, ENABLE);
#endif
	
#if(RTC_OW_INT_EN == 1)
	RTC_ITConfig(RTC_IT_OW, ENABLE);
#endif
	
#if(RTC_ALR_INT_EN == 1)
	RTC_ITConfig(RTC_IT_ALR, ENABLE);
#endif
	
#if(RTC_SEC_INT_EN == 1 || RTC_OW_INT_EN == 1 || RTC_ALR_INT_EN == 1)
	MCU_NVIC_Init(RTC_IRQn, ENABLE, 2, 1);
#endif
	
#if(RTC_ALRE_INT_EN == 1)
	RTC_ITConfig(RTC_IT_SEC, ENABLE);

	//ֹͣģʽ��RTC���������ⲿ�ж�17�Ͽɻ���
	exti_initstruct.EXTI_Line = EXTI_Line17;
	exti_initstruct.EXTI_LineCmd = ENABLE;
	exti_initstruct.EXTI_Mode = EXTI_Mode_Interrupt;
	exti_initstruct.EXTI_Trigger = EXTI_Trigger_Rising;	//������
	
	EXTI_Init(&exti_initstruct);
	
	MCU_NVIC_Init(RTCAlarm_IRQn, ENABLE, 2, 0);
#endif

}

/*
************************************************************
*	�������ƣ�	RTC_SetTime
*
*	�������ܣ�	RTCʱ������
*
*	��ڲ�����	sec����ֵ
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void RTC_SetTime(unsigned int sec)
{

	if(RTC_IsBusReady() == 0)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//ʹ��PWR��BKP����ʱ��  
		PWR_BackupAccessCmd(ENABLE);												//ʹ��RTC�ͺ󱸼Ĵ������� 
		RTC_SetCounter(sec);														//����RTC��������ֵ

		RTC_WaitForLastTask();														//�ȴ����һ�ζ�RTC�Ĵ�����д�������
		
		RTC_FreeBus();
	}

}

/*
************************************************************
*	�������ƣ�	RTC_AlarmSet
*
*	�������ܣ�	RTC���ӳ�ʼ��
*
*	��ڲ�����	sec����ֵ
*
*	���ز�����	��
*
*	˵����		д��һ��32bit��������������ʱ�䡣����RTC_CNT���ʱ���������ж�
************************************************************
*/
void RTC_AlarmSet(unsigned int sec)
{

	if(RTC_IsBusReady() == 0)
	{
		RTC_WaitForLastTask();
		
		RTC_SetAlarm(RTC_GetCounter() + sec);	//���ڵ�ǰʱ����������
		
		RTC_WaitForLastTask();
		RTC_WaitForSynchro();					//�ȴ�RTC�Ĵ���ͬ��
		
		RTC_FreeBus();
	}

}

/*
************************************************************
*	�������ƣ�	RTC_IRQHandler
*
*	�������ܣ�	RTCһ�㹦���ж�
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void RTC_IRQHandler(void)
{

	//���ж�
	if(RTC_GetITStatus(RTC_IT_SEC))
	{
		RTC_ClearITPendingBit(RTC_IT_SEC);	//�����ж�
		
		rtc_info.rtc_sec_int = 1;
	}
	//����ж�
	if(RTC_GetITStatus(RTC_IT_OW))
	{
		RTC_ClearITPendingBit(RTC_IT_OW);	//������ж�
		
		rtc_info.rtc_ow_int = 1;
	}
	//�����ж�
	if(RTC_GetITStatus(RTC_IT_ALR))
	{
		RTC_ClearITPendingBit(RTC_IT_ALR);	//�������ж�
		
		rtc_info.rtc_alr_int = 1;
	}

}

/*
************************************************************
*	�������ƣ�	RTCAlarm_IRQHandler
*
*	�������ܣ�	RTC����-�ⲿ�ж�
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void RTCAlarm_IRQHandler(void)
{
	
	rtc_info.rtc_alre_int = 1;
	
	EXTI_ClearITPendingBit(EXTI_Line17);

}
