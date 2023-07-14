#ifndef _RTC_H_
#define _RTC_H_





typedef struct
{

	unsigned char rtc_sec_int : 1;		//���ж�
	
	unsigned char rtc_ow_int : 1;		//����ж�
	
	unsigned char rtc_alr_int : 1;		//�����ж�
	
	unsigned char rtc_alre_int : 1;		//����-�ⲿ�ж�
	
	unsigned char reverse : 4;

} RTC_INFO;

extern RTC_INFO rtc_info;


#define USE_EXT_RCC			0	//1-ʹ��LSE		0-ʹ��HSE��Ƶʱ��


#define RTC_SEC_INT_EN		0	//1-ʹ�����ж�			0-��ʹ��

#define RTC_OW_INT_EN		0	//1-ʹ������ж�			0-��ʹ��

#define RTC_ALR_INT_EN		0	//1-ʹ�������ж�			0-��ʹ��

#define RTC_ALRE_INT_EN		0	//1-ʹ������-�ⲿ�ж�	0-��ʹ��


_Bool RTC_Init(void);

void RTC_NVIC_Init(void);

void RTC_SetTime(unsigned int sec);

void RTC_AlarmSet(unsigned int sec);


#endif
