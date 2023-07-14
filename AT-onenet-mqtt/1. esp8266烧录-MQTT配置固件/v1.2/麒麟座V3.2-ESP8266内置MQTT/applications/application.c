/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	application.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2017-12-07
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		RT-Thread���ܲ���
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

/**
 * @addtogroup STM32
 */
/*@{*/

//RTOS
#include <board.h>
#include <rtthread.h>

//�����豸
#include "net_device.h"

//��������
#include "net_task.h"

//Э��
#include "onenet.h"

//����
#include "delay.h"
#include "usart.h"
#include "hwtimer.h"
#include "i2c.h"
#include "iwdg.h"
#include "rtc.h"

//Ӳ��
#include "led.h"
#include "key.h"
#include "lcd1602.h"
#include "adxl362.h"
#include "sht20.h"
#include "at24c02.h"
#include "beep.h"
#include "info.h"
#include "spilcd.h"
#include "light.h"
#include "ir.h"
#include "nec.h"

//����������
#include "dataStreamName.h"

//ͼƬ
#include "image_2k.h"

//�ֿ�
#include "font.h"

//C��
#include <string.h>
#include <time.h>
#include <stdio.h>


#define SPILCD_EN					1		//1-ʹ��SPILCD		0-ʹ��LCD1602


//���Ź�����
#define IWDG_TASK_PRIO				1
#define IWDG_TASK_TICK				1
#define IWDG_TASK_STK_SIZE			256
ALIGN(RT_ALIGN_SIZE) unsigned char IWDG_TASK_STK[IWDG_TASK_STK_SIZE];
struct rt_thread IWDG_Task_Handle;

//����������
#define SENSOR_TASK_PRIO			7
#define SENSOR_TASK_TICK			1
#define SENSOR_TASK_STK_SIZE		2048
ALIGN(RT_ALIGN_SIZE) unsigned char SENSOR_TASK_STK[SENSOR_TASK_STK_SIZE];
struct rt_thread SENSOR_Task_Handle;

//��������
#define KEY_TASK_PRIO				8
#define KEY_TASK_TICK				1
#define KEY_TASK_STK_SIZE			1024
ALIGN(RT_ALIGN_SIZE) unsigned char KEY_TASK_STK[KEY_TASK_STK_SIZE];
struct rt_thread KEY_Task_Handle;

//��Ϣ��������
#define ALTER_TASK_PRIO				9
#define ALTER_TASK_TICK				1
#define ALTER_TASK_STK_SIZE			512
ALIGN(RT_ALIGN_SIZE) unsigned char ALTER_TASK_STK[ALTER_TASK_STK_SIZE];
struct rt_thread ALTER_Task_Handle;

//ʱ������
#define CLOCK_TASK_PRIO				10
#define CLOCK_TASK_TICK				1
#define CLOCK_TASK_STK_SIZE			2048
ALIGN(RT_ALIGN_SIZE) unsigned char CLOCK_TASK_STK[CLOCK_TASK_STK_SIZE];
struct rt_thread CLOCK_Task_Handle;

//��ʼ������
#define INIT_TASK_PRIO				11
#define INIT_TASK_TICK				1
#define INIT_TASK_STK_SIZE			2048
ALIGN(RT_ALIGN_SIZE) unsigned char INIT_TASK_STK[INIT_TASK_STK_SIZE];
struct rt_thread INIT_Task_Handle;


char myTime[24];


//������
DATA_STREAM data_stream[] = {
								{ZW_TIME, myTime, TYPE_STRING, 0},
								
								{"GPS", &gps, TYPE_GPS, 0},
								
								{ZW_NETWORK_DELAY, &net_device_info.network_delay_time, TYPE_USHORT, 1},
								
								{ZW_REDLED, &led_status.led_status[0], TYPE_BOOL, 1},
								{ZW_GREENLED, &led_status.led_status[1], TYPE_BOOL, 1},
								{ZW_YELLOWLED, &led_status.led_status[2], TYPE_BOOL, 1},
								{ZW_BLUELED, &led_status.led_status[3], TYPE_BOOL, 1},
								
								{ZW_BEEP, &beep_info.beep_status[0], TYPE_BOOL, 1},
								
								{ZW_TEMPERATURE, &sht20_info.tempreture, TYPE_FLOAT, 1},
								{ZW_HUMIDITY, &sht20_info.humidity, TYPE_FLOAT, 1},
								
								{ZW_X, &adxl362_info.x, TYPE_FLOAT, 1},
								{ZW_Y, &adxl362_info.y, TYPE_FLOAT, 1},
								{ZW_Z, &adxl362_info.z, TYPE_FLOAT, 1},
								
								{ZW_BG, &spilcd_info.blSta, TYPE_UCHAR, 1},
							};
unsigned char data_stream_cnt = sizeof(data_stream) / sizeof(data_stream[0]);


/*
************************************************************
*	�������ƣ�	Hardware_Init
*
*	�������ܣ�	Ӳ����ʼ��
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		��ʼ����Ƭ�������Լ�����豸
************************************************************
*/
void Hardware_Init(void)
{

	Delay_Init();																//Timer4��ʼ��
	
	Usart1_Init(115200); 														//��ʼ������   115200bps
#if(USART_DMA_RX_EN)
	USARTx_ResetMemoryBaseAddr(USART_DEBUG, (unsigned int)alter_info.alter_buf, sizeof(alter_info.alter_buf), USART_RX_TYPE);
#endif
	
	LED_Init();																	//LED��ʼ��
	
	KEY_Init();																	//������ʼ��
	
	BEEP_Init();																//��������ʼ��
	
	LIGHT_Init();																//���������ʼ��
	
	IR_Init(38000);																//���ⷢ��ܳ�ʼ��
	
	IIC_Init(I2C2);																//IIC���߳�ʼ��
	
#if(SPILCD_EN == 1)
	SPILCD_Init();																//SPILCD��ʼ��
#else
	LCD1602_Init();																//LCD1602��ʼ��
#endif
	
	RTC_Init();																	//��ʼ��RTC
	
	UsartPrintf(USART_DEBUG, "EEPROM: %s\r\n", AT24C02_Exist() ? "Ok" : "Err");	//EEPROM���
	
	UsartPrintf(USART_DEBUG, "SHT20: %s\r\n", SHT20_Exist() ? "Ok" : "Err");	//SHT20���
	
	UsartPrintf(USART_DEBUG, "ADXL362: %s\r\n", ADXL362_Init() ? "Ok" : "Err");	//ADXL362���

	if(RCC_GetFlagStatus(RCC_FLAG_IWDGRST) == SET) 								//����ǿ��Ź���λ����ʾ
	{
		UsartPrintf(USART_DEBUG, "WARN:	IWDG Reboot\r\n");
		
		RCC_ClearFlag();														//������Ź���λ��־λ
	}
	else
	{
		
	}
	
	Iwdg_Init(4, 1250); 														//64��Ƶ��ÿ��625�Σ�����1250�Σ�2s
	
	UsartPrintf(USART_DEBUG, "Hardware init OK\r\n");							//��ʾ��ʼ�����

}


/*
************************************************************
*	�������ƣ�	NET_Event_CallBack
*
*	�������ܣ�	�����¼��ص�
*
*	��ڲ�����	net_event���¼�����
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void NET_Event_CallBack(NET_EVENT net_event)
{

	switch((unsigned char)net_event)
	{
		case NET_EVENT_Timer_Check_Err:		//���綨ʱ��鳬ʱ����
			UsartPrintf(USART_DEBUG, "Tips:	Timer Check Err\r\n");
		break;
		
		case NET_EVENT_Timer_Send_Err:		//���緢��ʧ�ܴ���
			UsartPrintf(USART_DEBUG, "Tips:	Timer Check Err-Send\r\n");
		break;
		
		case NET_EVENT_Send_HeartBeat:		//��������������
		break;
		
		case NET_EVENT_Send_Data:			//�����������ݵ�
		break;
		
		case NET_EVENT_Send_Subscribe:		//�������Ͷ�������
		break;
		
		case NET_EVENT_Send_UnSubscribe:	//��������ȡ����������
		break;
		
		case NET_EVENT_Send_Publish:		//����������������
		break;
		
		case NET_EVENT_Send:				//��ʼ��������
		break;
		
		case NET_EVENT_Recv:				//Modbus��-�յ����ݲ�ѯָ��
		break;
		
		case NET_EVENT_Check_Status:		//��������ģ��״̬���
		break;
		
		case NET_EVENT_Device_Ok:			//����ģ����Ok
			UsartPrintf(USART_DEBUG, "NET Device :Ok\r\n");
		break;
		case NET_EVENT_Device_Err:			//����ģ�������
			UsartPrintf(USART_DEBUG, "NET Device :Error\r\n");
		break;
		
		case NET_EVENT_Initialize:			//���ڳ�ʼ������ģ��
#if(SPILCD_EN == 1)
			SPILCD_DisZW(0, 80, BLUE, lian);SPILCD_DisZW(16, 80, BLUE, jie);SPILCD_DisZW(32, 80, BLUE, zhong);
#endif
		break;
		
		case NET_EVENT_Init_Ok:				//����ģ���ʼ���ɹ�
		break;
		
		case NET_EVENT_Auto_Create_Ok:		//�Զ������豸�ɹ�
			UsartPrintf(USART_DEBUG, "Tips:	Auto Create Device Ok\r\n");
		break;
		
		case NET_EVENT_Auto_Create_Err:		//�Զ������豸ʧ��
			UsartPrintf(USART_DEBUG, "WARN:	Auto Create Device Err\r\n");
		break;
		
		case NET_EVENT_Connect:				//�������ӡ���¼OneNET
		break;
		
		case NET_EVENT_Connect_Ok:			//���ӡ���¼�ɹ�
			
			BEEP_Ctl(0, BEEP_ON);
			RTOS_TimeDly(40);
			BEEP_Ctl(0, BEEP_OFF);
			
#if(SPILCD_EN == 1)
			SPILCD_DisZW(0, 80, BLUE, yi);SPILCD_DisZW(16, 80, BLUE, lian);SPILCD_DisZW(32, 80, BLUE, jie);
#endif
			
			if(gps.flag == 1)
				data_stream[1].flag = 1;	//GPS������׼���ϴ�
		break;
		
		case NET_EVENT_Connect_Err:			//���ӡ���¼����
			
			BEEP_Ctl(0, BEEP_ON);
			RTOS_TimeDly(100);
			BEEP_Ctl(0, BEEP_OFF);
			
#if(SPILCD_EN == 1)
			SPILCD_DisZW(0, 80, BLUE, wei);SPILCD_DisZW(16, 80, BLUE, lian);SPILCD_DisZW(32, 80, BLUE, jie);
#endif
		break;
		
		case NET_EVENT_Fault_Process:		//������
#if(SPILCD_EN == 1)
			SPILCD_DisZW(16, 80, BLUE, duan);SPILCD_DisZW(32, 80, BLUE, kai);
#endif
			UsartPrintf(USART_DEBUG, "WARN:	NET Fault Process\r\n");
		break;
		
		default:							//��
		break;
	}

}

/*
************************************************************
*	�������ƣ�	IWDG_Task
*
*	�������ܣ�	������Ź�
*
*	��ڲ�����	void���͵Ĳ���ָ��
*
*	���ز�����	��
*
*	˵����		���Ź�����
************************************************************
*/
void IWDG_Task(void *pvParameters)
{

	while(1)
	{
	
		Iwdg_Feed(); 		//ι��
		
		RTOS_TimeDly(50); 	//��������250ms
	
	}

}

/*
************************************************************
*	�������ƣ�	KEY_Task
*
*	�������ܣ�	ɨ�谴���Ƿ��£�����а��£����ж�Ӧ�Ĵ���
*
*	��ڲ�����	void���͵Ĳ���ָ��
*
*	���ز�����	��
*
*	˵����		��������
************************************************************
*/
void KEY_Task(void *pvParameters)
{

	unsigned char key_event_r = KEY_NONE;

	while(1)
	{
		
		key_event_r = Keyboard();
		
		//�����ж�-------------------------------------------------------
		if(key_event_r == key_event[0][KEY_X_DOWN])
		{
			if(led_status.led_status[0] == LED_OFF)
				LED_Ctl(0, LED_ON);
			else
				LED_Ctl(0, LED_OFF);
			
			onenet_info.send_data |= SEND_TYPE_DATA;
		}
		else if(key_event_r == key_event[1][KEY_X_DOWN])
		{
			if(led_status.led_status[1] == LED_OFF)
				LED_Ctl(1, LED_ON);
			else
				LED_Ctl(1, LED_OFF);
			
			onenet_info.send_data |= SEND_TYPE_DATA;
		}
		else if(key_event_r == key_event[2][KEY_X_DOWN])
		{
			if(led_status.led_status[2] == LED_OFF)
				LED_Ctl(2, LED_ON);
			else
				LED_Ctl(2, LED_OFF);
			
			onenet_info.send_data |= SEND_TYPE_DATA;
		}
		else if(key_event_r == key_event[3][KEY_X_DOWN])
		{
			if(led_status.led_status[3] == LED_OFF)
				LED_Ctl(3, LED_ON);
			else
				LED_Ctl(3, LED_OFF);
			
			onenet_info.send_data |= SEND_TYPE_DATA;
		}
		
		//˫���ж�-------------------------------------------------------
		if(key_event_r == key_event[0][KEY_X_DOUBLE])
		{
			onenet_info.send_data |= SEND_TYPE_DATA;
			onenet_info.send_data |= SEND_TYPE_HEART;
			onenet_info.send_data |= SEND_TYPE_SUBSCRIBE;
			onenet_info.send_data |= SEND_TYPE_PUBLISH;
		}
		
		//�����ж�-------------------------------------------------------
		if(key_event_r == key_event[0][KEY_X_DOWNLONG])
		{
			
		}
	
		RTOS_TimeDly(10); 								//��������50ms
	
	}

}

/*
************************************************************
*	�������ƣ�	SENSOR_Task
*
*	�������ܣ�	���������ݲɼ�����ʾ
*
*	��ڲ�����	void���͵Ĳ���ָ��
*
*	���ز�����	��
*
*	˵����		���������ݲɼ����񡣽�����Ӵ����������ݲɼ�����ȡ����ʾ
************************************************************
*/
void SENSOR_Task(void *pvParameters)
{
	
	unsigned char count = 0;
	
	if(!Info_Check() && eeprom_info.device_ok)
	{
		//AT24C02_Clear(0, 255, 256);
		UsartPrintf(USART_DEBUG, "ssid_pswd in EEPROM\r\n");
		Info_Read();
	}
	else
	{
		UsartPrintf(USART_DEBUG, "ssid_pswd in ROM\r\n");
	}
	
	UsartPrintf(USART_DEBUG, "DEVID: %s,	APIKEY: %s\r\nPROID:%s,	AUIF:%s\r\n"
							, onenet_info.dev_id, onenet_info.api_key, onenet_info.pro_id, onenet_info.auth_info);
	
#if(SPILCD_EN == 1)
	SPILCD_Clear(BGC);									//����
	
	//������ʾ
	SPILCD_DisZW(0, 0, RED, san);						//��ʾ������
	SPILCD_DisZW(16, 0, RED, zhou);						//��ʾ���ᡱ
	
	SPILCD_DisZW(0, 32, RED, wen);						//��ʾ���¡�
	SPILCD_DisZW(16, 32, RED, shi);						//��ʾ��ʪ��
	SPILCD_DisZW(32, 32, RED, du);						//��ʾ���ȡ�
	
	SPILCD_DisZW(96, 64, RED, guang);					//��ʾ���⡱
	SPILCD_DisZW(112, 64, RED, min);					//��ʾ������
	
	SPILCD_DisZW(0, 64, RED, zhuang);					//��ʾ��״��
	SPILCD_DisZW(16, 64, RED, tai);						//��ʾ��̬��
#else
	LCD1602_Clear(0xff);								//����
#endif

	while(1)
	{
		
		if(adxl362_info.device_ok) 						//ֻ���豸����ʱ���Ż��ȡֵ����ʾ
		{
			ADXL362_GetValue();							//�ɼ�����������
				
#if(SPILCD_EN == 1)
			SPILCD_DisString(0, 16, 16, BLUE, 1, "X%0.1f,Y%0.1f,Z%0.1f    ", adxl362_info.x, adxl362_info.y, adxl362_info.z);
#else
			LCD1602_DisString(0x80, "X%0.1f,Y%0.1f,Z%0.1f", adxl362_info.x, adxl362_info.y, adxl362_info.z);
#endif
		}
		
		RTOS_TimeDly(1);
			
		if(sht20_info.device_ok) 						//ֻ���豸����ʱ���Ż��ȡֵ����ʾ
		{
			SHT20_GetValue();							//�ɼ�����������
				
#if(SPILCD_EN == 1)
			SPILCD_DisString(0, 48, 16, BLUE, 1, "%0.1fC,%0.1f%%    ", sht20_info.tempreture, sht20_info.humidity);
#else
			LCD1602_DisString(0xC0, "%0.1fC,%0.1f%%", sht20_info.tempreture, sht20_info.humidity);
#endif
		}
		
		RTOS_TimeDly(1);
			
		LIGHT_GetVoltag();
			
#if(SPILCD_EN == 1)
		SPILCD_DisString(95, 80, 16, BLUE, 1, "%0.2f%", light_info.voltag);
#else
		LCD1602_DisString(0xCC, "%0.2f%", light_info.voltag);
#endif
		
#if(SPILCD_EN == 1)
		SPILCD_BL_Ctl_Auto();							//�Զ�����SPILCD��������
#endif
		
		if(++count >= 20)								//ÿ��һ��ʱ�䷢��һ�κ�������
		{
			count = 0;
			
			NET_DEVICE_GetNWD();
		}
		
		RTOS_TimeDly(100); 								//��������500ms
	
	}

}

/*
************************************************************
*	�������ƣ�	ALTER_Task
*
*	�������ܣ�	ͨ�����ڸ���SSID��PSWD��DEVID��APIKEY
*
*	��ڲ�����	void���͵Ĳ���ָ��
*
*	���ز�����	��
*
*	˵����		���ĺ�ᱣ�浽EEPROM��
************************************************************
*/
void ALTER_Task(void *pvParameters)
{

    while(1)
    {
    
		memset(alter_info.alter_buf, 0, sizeof(alter_info.alter_buf));
		
		while(!alter_info.rev_idle)
			RTOS_TimeDly(20);														//ÿ100ms���һ��
		
		alter_info.rev_idle = 0;
		
		UsartPrintf(USART_DEBUG, "\r\nAlter Rev\r\n%s\r\n", alter_info.alter_buf);
        
		if(eeprom_info.device_ok)													//���EEPROM����
		{
			if(Info_Alter(alter_info.alter_buf))									//������Ϣ
			{
				onenet_info.status = ONENET_STATUS_DIS;
			}
		}
    
    }

}

/*
************************************************************
*	�������ƣ�	CLOCK_Task
*
*	�������ܣ�	����Уʱ��ʱ����ʾ
*
*	��ڲ�����	void���͵Ĳ���ָ��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void CLOCK_Task(void *pvParameter)
{
	
#if(NET_TIME_EN == 1)
	unsigned int second = 0, second_pre = 0;							//second��ʵʱʱ�䣬second_pre��ֵ�Ƚ�
	struct tm *time;
	_Bool get_net_time = 0;
	
	while(!net_device_info.net_time)
		RTOS_TimeDly(20);
	
	RTC_SetTime(net_device_info.net_time + 4);
	
	data_stream[0].flag = 1;
#endif

	while(1)
	{
	
#if(NET_TIME_EN == 1)
		if(get_net_time)												//��Ҫ��ȡʱ��
		{
			data_stream[0].flag = 0;									//���ϴ�ʱ��
			
			net_device_info.net_time = NET_DEVICE_GetTime();
			
			if(net_device_info.net_time)
			{
				second = RTC_GetCounter();
				
				if(((net_device_info.net_time <= second + 300) && (net_device_info.net_time >= second - 300)) || (second <= 100))
				{														//����ڡ�5�����ڣ�����Ϊʱ����ȷ
					RTC_SetTime(net_device_info.net_time + 4);			//����RTCʱ�䣬��4�ǲ��ϴ�ŵ�ʱ���
					
					get_net_time = 0;
					
					data_stream[0].flag = 1;							//�ϴ�ʱ��
				}
			}
		}

		second = RTC_GetCounter();										//��ȡ��ֵ
		
		if(second > second_pre)
		{
			second_pre = second;
			time = localtime((const time_t *)&second);					//����ֵתΪtm�ṹ����ʾ��ʱ��
			
			memset(myTime, 0, sizeof(myTime));
			snprintf(myTime, sizeof(myTime), "%d-%d-%d %d:%d:%d",
							time->tm_year + 1900, time->tm_mon + 1, time->tm_mday,
							time->tm_hour, time->tm_min, time->tm_sec);
#if(SPILCD_EN == 1)
			SPILCD_DisString(0, 115, 12, RED, 1, "%s     ", myTime);	//��ʾ
#endif
		
			if(time->tm_hour == 0 && time->tm_min == 0 && time->tm_sec == 0)//ÿ��0��ʱ������һ��ʱ��
			{
				get_net_time = 1;
				net_device_info.net_time = 0;
			}
		}
#endif
		
		RTOS_TimeDly(20);													//��������100ms
	
	}

}

/*
************************************************************
*	�������ƣ�	INIT_Task
*
*	�������ܣ�	��ʼ��Ӳ��������Ӧ������
*
*	��ڲ�����	void���͵Ĳ���ָ��
*
*	���ز�����	��
*
*	˵����		��ʼ������
************************************************************
*/
void INIT_Task(void *pvParameters)
{

	Hardware_Init();								//Ӳ����ʼ��
	
	RTOS_ENTER_CRITICAL();
	
	if(rt_thread_init(&IWDG_Task_Handle, "IWDG", IWDG_Task, RT_NULL,
						(unsigned char *)&IWDG_TASK_STK[0], IWDG_TASK_STK_SIZE, IWDG_TASK_PRIO, IWDG_TASK_TICK) == RT_EOK)
		rt_thread_startup(&IWDG_Task_Handle);
	
	if(rt_thread_init(&SENSOR_Task_Handle, "SENSOR", SENSOR_Task, RT_NULL,
						(unsigned char *)&SENSOR_TASK_STK[0], SENSOR_TASK_STK_SIZE, SENSOR_TASK_PRIO, SENSOR_TASK_TICK) == RT_EOK)
		rt_thread_startup(&SENSOR_Task_Handle);
	
	if(rt_thread_init(&KEY_Task_Handle, "KEY", KEY_Task, RT_NULL,
						(unsigned char *)&KEY_TASK_STK[0], KEY_TASK_STK_SIZE, KEY_TASK_PRIO, KEY_TASK_TICK) == RT_EOK)
		rt_thread_startup(&KEY_Task_Handle);
	
	if(rt_thread_init(&ALTER_Task_Handle, "ALTER", ALTER_Task, RT_NULL,
						(unsigned char *)&ALTER_TASK_STK[0], ALTER_TASK_STK_SIZE, ALTER_TASK_PRIO, ALTER_TASK_TICK) == RT_EOK)
		rt_thread_startup(&ALTER_Task_Handle);
	
	if(rt_thread_init(&CLOCK_Task_Handle, "CLOCK", CLOCK_Task, RT_NULL,
						(unsigned char *)&CLOCK_TASK_STK[0], CLOCK_TASK_STK_SIZE, CLOCK_TASK_PRIO, CLOCK_TASK_TICK) == RT_EOK)
		rt_thread_startup(&CLOCK_Task_Handle);
	
	NET_Task_Init();
	
	RTOS_EXIT_CRITICAL();
	
	UsartPrintf(USART_DEBUG, "OneNET-IOT Start\r\n");
	
	while(1)
	{
		RTOS_TimeDly(200);
	}

}





/*
************************************************************
*	�������ƣ�	rt_application_init
*
*	�������ܣ�	���񴴽�
*
*	��ڲ�����	��
*
*	���ز�����	0
*
*	˵����		
************************************************************
*/
int rt_application_init(void)
{
	
	if(rt_thread_init(&INIT_Task_Handle, "INIT", INIT_Task, RT_NULL,
						(unsigned char *)&INIT_TASK_STK[0], INIT_TASK_STK_SIZE, INIT_TASK_PRIO, INIT_TASK_TICK) == RT_EOK)
		rt_thread_startup(&INIT_Task_Handle);

	return 0;
	
}

/*@}*/
