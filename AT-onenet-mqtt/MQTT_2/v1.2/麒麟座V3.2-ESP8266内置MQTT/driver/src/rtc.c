/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	rtc.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-11-09
	*
	*	版本： 		V1.1
	*
	*	说明： 		RTC初始化和功能
	*
	*	修改记录：	V1.1：修复RTC闹钟使用的BUG
						  增加RTC中断控制功能
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f10x.h"

//单片机相关组件
#include "mcu_nvic.h"

//硬件驱动
#include "delay.h"
#include "rtc.h"


static _Bool rtc_busy = 0;


RTC_INFO rtc_info;


/*
************************************************************
*	函数名称：	RTC_IsBusReady
*
*	函数功能：	查询总线是否就绪
*
*	入口参数：	无
*
*	返回参数：	0-就绪	1-未就绪
*
*	说明：		
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
*	函数名称：	RTC_FreeBus
*
*	函数功能：	释放总线
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
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
*	函数名称：	RTC_Init
*
*	函数功能：	RTC初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
_Bool RTC_Init(void)
{
	
#if(USE_EXT_RCC == 1)
	unsigned char err_count = 0;
#endif

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//使能PWR和BKP外设时钟   
	PWR_BackupAccessCmd(ENABLE);												//使能后备寄存器访问
	
	BKP_DeInit();																//复位备份区域
#if(USE_EXT_RCC == 1)
	RCC_LSEConfig(RCC_LSE_ON);													//设置外部低速晶振(LSE),使用外设低速晶振
	
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET && err_count < 250)		//检查指定的RCC标志位设置与否,等待低速晶振就绪
	{
		err_count++;
		RTOS_TimeDly(2);
	}
	
	if(err_count >= 250)
		return 1;																//初始化时钟失败,晶振有问题
#endif
	
#if(USE_EXT_RCC == 1)
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);										//设置RTC时钟(RTCCLK),选择LSE作为RTC时钟
#else
	RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);								//设置RTC时钟(HSE/128),选择HES作为RTC时钟
#endif
	RCC_RTCCLKCmd(ENABLE);														//使能RTC时钟
	RTC_WaitForLastTask();														//等待最近一次对RTC寄存器的写操作完成
	RTC_WaitForSynchro();														//等待RTC寄存器同步
	
	RTC_ITConfig(RTC_IT_ALR, ENABLE);											//使能RTC闹钟中断
	RTC_WaitForLastTask();														//等待最近一次对RTC寄存器的写操作完成
	
	RTC_EnterConfigMode();														//允许配置
#if(USE_EXT_RCC == 1)
	RTC_SetPrescaler(32767);													//设置RTC预分频的值
#else
	RTC_SetPrescaler(HSE_VALUE / 128 - 1);										//设置RTC预分频的值
#endif
	RTC_WaitForLastTask();														//等待最近一次对RTC寄存器的写操作完成
	
	RTC_SetCounter(0);															//设置RTC计数器的值
	RTC_WaitForLastTask();														//等待最近一次对RTC寄存器的写操作完成
	
	RTC_ExitConfigMode();														//退出配置模式
	
	RTC_NVIC_Init();
	
	return 0;

}

/*
************************************************************
*	函数名称：	RTC_NVIC_Init
*
*	函数功能：	RTC中断初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
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

	//停止模式下RTC闹钟连到外部中断17上可唤醒
	exti_initstruct.EXTI_Line = EXTI_Line17;
	exti_initstruct.EXTI_LineCmd = ENABLE;
	exti_initstruct.EXTI_Mode = EXTI_Mode_Interrupt;
	exti_initstruct.EXTI_Trigger = EXTI_Trigger_Rising;	//上升沿
	
	EXTI_Init(&exti_initstruct);
	
	MCU_NVIC_Init(RTCAlarm_IRQn, ENABLE, 2, 0);
#endif

}

/*
************************************************************
*	函数名称：	RTC_SetTime
*
*	函数功能：	RTC时间设置
*
*	入口参数：	sec：秒值
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void RTC_SetTime(unsigned int sec)
{

	if(RTC_IsBusReady() == 0)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//使能PWR和BKP外设时钟  
		PWR_BackupAccessCmd(ENABLE);												//使能RTC和后备寄存器访问 
		RTC_SetCounter(sec);														//设置RTC计数器的值

		RTC_WaitForLastTask();														//等待最近一次对RTC寄存器的写操作完成
		
		RTC_FreeBus();
	}

}

/*
************************************************************
*	函数名称：	RTC_AlarmSet
*
*	函数功能：	RTC闹钟初始化
*
*	入口参数：	sec：秒值
*
*	返回参数：	无
*
*	说明：		写入一个32bit的秒数，来代表时间。当与RTC_CNT相等时产生闹钟中断
************************************************************
*/
void RTC_AlarmSet(unsigned int sec)
{

	if(RTC_IsBusReady() == 0)
	{
		RTC_WaitForLastTask();
		
		RTC_SetAlarm(RTC_GetCounter() + sec);	//基于当前时间设置闹钟
		
		RTC_WaitForLastTask();
		RTC_WaitForSynchro();					//等待RTC寄存器同步
		
		RTC_FreeBus();
	}

}

/*
************************************************************
*	函数名称：	RTC_IRQHandler
*
*	函数功能：	RTC一般功能中断
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void RTC_IRQHandler(void)
{

	//秒中断
	if(RTC_GetITStatus(RTC_IT_SEC))
	{
		RTC_ClearITPendingBit(RTC_IT_SEC);	//清秒中断
		
		rtc_info.rtc_sec_int = 1;
	}
	//溢出中断
	if(RTC_GetITStatus(RTC_IT_OW))
	{
		RTC_ClearITPendingBit(RTC_IT_OW);	//清溢出中断
		
		rtc_info.rtc_ow_int = 1;
	}
	//闹钟中断
	if(RTC_GetITStatus(RTC_IT_ALR))
	{
		RTC_ClearITPendingBit(RTC_IT_ALR);	//清闹钟中断
		
		rtc_info.rtc_alr_int = 1;
	}

}

/*
************************************************************
*	函数名称：	RTCAlarm_IRQHandler
*
*	函数功能：	RTC闹钟-外部中断
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void RTCAlarm_IRQHandler(void)
{
	
	rtc_info.rtc_alre_int = 1;
	
	EXTI_ClearITPendingBit(EXTI_Line17);

}
