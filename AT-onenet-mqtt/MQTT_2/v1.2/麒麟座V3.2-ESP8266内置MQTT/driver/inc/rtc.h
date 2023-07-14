#ifndef _RTC_H_
#define _RTC_H_





typedef struct
{

	unsigned char rtc_sec_int : 1;		//秒中断
	
	unsigned char rtc_ow_int : 1;		//溢出中断
	
	unsigned char rtc_alr_int : 1;		//闹钟中断
	
	unsigned char rtc_alre_int : 1;		//闹钟-外部中断
	
	unsigned char reverse : 4;

} RTC_INFO;

extern RTC_INFO rtc_info;


#define USE_EXT_RCC			0	//1-使用LSE		0-使用HSE分频时钟


#define RTC_SEC_INT_EN		0	//1-使能秒中断			0-不使能

#define RTC_OW_INT_EN		0	//1-使能溢出中断			0-不使能

#define RTC_ALR_INT_EN		0	//1-使能闹钟中断			0-不使能

#define RTC_ALRE_INT_EN		0	//1-使能闹钟-外部中断	0-不使能


_Bool RTC_Init(void);

void RTC_NVIC_Init(void);

void RTC_SetTime(unsigned int sec);

void RTC_AlarmSet(unsigned int sec);


#endif
