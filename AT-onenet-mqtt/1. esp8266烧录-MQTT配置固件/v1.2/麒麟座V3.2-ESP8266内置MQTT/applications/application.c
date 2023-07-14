/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	application.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-12-07
	*
	*	版本： 		V1.0
	*
	*	说明： 		RT-Thread功能测试
	*
	*	修改记录：	
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

//网络设备
#include "net_device.h"

//网络任务
#include "net_task.h"

//协议
#include "onenet.h"

//驱动
#include "delay.h"
#include "usart.h"
#include "hwtimer.h"
#include "i2c.h"
#include "iwdg.h"
#include "rtc.h"

//硬件
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

//中文数据流
#include "dataStreamName.h"

//图片
#include "image_2k.h"

//字库
#include "font.h"

//C库
#include <string.h>
#include <time.h>
#include <stdio.h>


#define SPILCD_EN					1		//1-使用SPILCD		0-使用LCD1602


//看门狗任务
#define IWDG_TASK_PRIO				1
#define IWDG_TASK_TICK				1
#define IWDG_TASK_STK_SIZE			256
ALIGN(RT_ALIGN_SIZE) unsigned char IWDG_TASK_STK[IWDG_TASK_STK_SIZE];
struct rt_thread IWDG_Task_Handle;

//传感器任务
#define SENSOR_TASK_PRIO			7
#define SENSOR_TASK_TICK			1
#define SENSOR_TASK_STK_SIZE		2048
ALIGN(RT_ALIGN_SIZE) unsigned char SENSOR_TASK_STK[SENSOR_TASK_STK_SIZE];
struct rt_thread SENSOR_Task_Handle;

//按键任务
#define KEY_TASK_PRIO				8
#define KEY_TASK_TICK				1
#define KEY_TASK_STK_SIZE			1024
ALIGN(RT_ALIGN_SIZE) unsigned char KEY_TASK_STK[KEY_TASK_STK_SIZE];
struct rt_thread KEY_Task_Handle;

//信息更改任务
#define ALTER_TASK_PRIO				9
#define ALTER_TASK_TICK				1
#define ALTER_TASK_STK_SIZE			512
ALIGN(RT_ALIGN_SIZE) unsigned char ALTER_TASK_STK[ALTER_TASK_STK_SIZE];
struct rt_thread ALTER_Task_Handle;

//时钟任务
#define CLOCK_TASK_PRIO				10
#define CLOCK_TASK_TICK				1
#define CLOCK_TASK_STK_SIZE			2048
ALIGN(RT_ALIGN_SIZE) unsigned char CLOCK_TASK_STK[CLOCK_TASK_STK_SIZE];
struct rt_thread CLOCK_Task_Handle;

//初始化任务
#define INIT_TASK_PRIO				11
#define INIT_TASK_TICK				1
#define INIT_TASK_STK_SIZE			2048
ALIGN(RT_ALIGN_SIZE) unsigned char INIT_TASK_STK[INIT_TASK_STK_SIZE];
struct rt_thread INIT_Task_Handle;


char myTime[24];


//数据流
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
*	函数名称：	Hardware_Init
*
*	函数功能：	硬件初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		初始化单片机功能以及外接设备
************************************************************
*/
void Hardware_Init(void)
{

	Delay_Init();																//Timer4初始化
	
	Usart1_Init(115200); 														//初始化串口   115200bps
#if(USART_DMA_RX_EN)
	USARTx_ResetMemoryBaseAddr(USART_DEBUG, (unsigned int)alter_info.alter_buf, sizeof(alter_info.alter_buf), USART_RX_TYPE);
#endif
	
	LED_Init();																	//LED初始化
	
	KEY_Init();																	//按键初始化
	
	BEEP_Init();																//蜂鸣器初始化
	
	LIGHT_Init();																//光敏电阻初始化
	
	IR_Init(38000);																//红外发射管初始化
	
	IIC_Init(I2C2);																//IIC总线初始化
	
#if(SPILCD_EN == 1)
	SPILCD_Init();																//SPILCD初始化
#else
	LCD1602_Init();																//LCD1602初始化
#endif
	
	RTC_Init();																	//初始化RTC
	
	UsartPrintf(USART_DEBUG, "EEPROM: %s\r\n", AT24C02_Exist() ? "Ok" : "Err");	//EEPROM检测
	
	UsartPrintf(USART_DEBUG, "SHT20: %s\r\n", SHT20_Exist() ? "Ok" : "Err");	//SHT20检测
	
	UsartPrintf(USART_DEBUG, "ADXL362: %s\r\n", ADXL362_Init() ? "Ok" : "Err");	//ADXL362检测

	if(RCC_GetFlagStatus(RCC_FLAG_IWDGRST) == SET) 								//如果是看门狗复位则提示
	{
		UsartPrintf(USART_DEBUG, "WARN:	IWDG Reboot\r\n");
		
		RCC_ClearFlag();														//清除看门狗复位标志位
	}
	else
	{
		
	}
	
	Iwdg_Init(4, 1250); 														//64分频，每秒625次，重载1250次，2s
	
	UsartPrintf(USART_DEBUG, "Hardware init OK\r\n");							//提示初始化完成

}


/*
************************************************************
*	函数名称：	NET_Event_CallBack
*
*	函数功能：	网络事件回调
*
*	入口参数：	net_event：事件类型
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void NET_Event_CallBack(NET_EVENT net_event)
{

	switch((unsigned char)net_event)
	{
		case NET_EVENT_Timer_Check_Err:		//网络定时检查超时错误
			UsartPrintf(USART_DEBUG, "Tips:	Timer Check Err\r\n");
		break;
		
		case NET_EVENT_Timer_Send_Err:		//网络发送失败错误
			UsartPrintf(USART_DEBUG, "Tips:	Timer Check Err-Send\r\n");
		break;
		
		case NET_EVENT_Send_HeartBeat:		//即将发送心跳包
		break;
		
		case NET_EVENT_Send_Data:			//即将发送数据点
		break;
		
		case NET_EVENT_Send_Subscribe:		//即将发送订阅数据
		break;
		
		case NET_EVENT_Send_UnSubscribe:	//即将发送取消订阅数据
		break;
		
		case NET_EVENT_Send_Publish:		//即将发送推送数据
		break;
		
		case NET_EVENT_Send:				//开始发送数据
		break;
		
		case NET_EVENT_Recv:				//Modbus用-收到数据查询指令
		break;
		
		case NET_EVENT_Check_Status:		//进入网络模组状态检查
		break;
		
		case NET_EVENT_Device_Ok:			//网络模组检测Ok
			UsartPrintf(USART_DEBUG, "NET Device :Ok\r\n");
		break;
		case NET_EVENT_Device_Err:			//网络模组检测错误
			UsartPrintf(USART_DEBUG, "NET Device :Error\r\n");
		break;
		
		case NET_EVENT_Initialize:			//正在初始化网络模组
#if(SPILCD_EN == 1)
			SPILCD_DisZW(0, 80, BLUE, lian);SPILCD_DisZW(16, 80, BLUE, jie);SPILCD_DisZW(32, 80, BLUE, zhong);
#endif
		break;
		
		case NET_EVENT_Init_Ok:				//网络模组初始化成功
		break;
		
		case NET_EVENT_Auto_Create_Ok:		//自动创建设备成功
			UsartPrintf(USART_DEBUG, "Tips:	Auto Create Device Ok\r\n");
		break;
		
		case NET_EVENT_Auto_Create_Err:		//自动创建设备失败
			UsartPrintf(USART_DEBUG, "WARN:	Auto Create Device Err\r\n");
		break;
		
		case NET_EVENT_Connect:				//正在连接、登录OneNET
		break;
		
		case NET_EVENT_Connect_Ok:			//连接、登录成功
			
			BEEP_Ctl(0, BEEP_ON);
			RTOS_TimeDly(40);
			BEEP_Ctl(0, BEEP_OFF);
			
#if(SPILCD_EN == 1)
			SPILCD_DisZW(0, 80, BLUE, yi);SPILCD_DisZW(16, 80, BLUE, lian);SPILCD_DisZW(32, 80, BLUE, jie);
#endif
			
			if(gps.flag == 1)
				data_stream[1].flag = 1;	//GPS就绪，准备上传
		break;
		
		case NET_EVENT_Connect_Err:			//连接、登录错误
			
			BEEP_Ctl(0, BEEP_ON);
			RTOS_TimeDly(100);
			BEEP_Ctl(0, BEEP_OFF);
			
#if(SPILCD_EN == 1)
			SPILCD_DisZW(0, 80, BLUE, wei);SPILCD_DisZW(16, 80, BLUE, lian);SPILCD_DisZW(32, 80, BLUE, jie);
#endif
		break;
		
		case NET_EVENT_Fault_Process:		//错误处理
#if(SPILCD_EN == 1)
			SPILCD_DisZW(16, 80, BLUE, duan);SPILCD_DisZW(32, 80, BLUE, kai);
#endif
			UsartPrintf(USART_DEBUG, "WARN:	NET Fault Process\r\n");
		break;
		
		default:							//无
		break;
	}

}

/*
************************************************************
*	函数名称：	IWDG_Task
*
*	函数功能：	清除看门狗
*
*	入口参数：	void类型的参数指针
*
*	返回参数：	无
*
*	说明：		看门狗任务
************************************************************
*/
void IWDG_Task(void *pvParameters)
{

	while(1)
	{
	
		Iwdg_Feed(); 		//喂狗
		
		RTOS_TimeDly(50); 	//挂起任务250ms
	
	}

}

/*
************************************************************
*	函数名称：	KEY_Task
*
*	函数功能：	扫描按键是否按下，如果有按下，进行对应的处理
*
*	入口参数：	void类型的参数指针
*
*	返回参数：	无
*
*	说明：		按键任务
************************************************************
*/
void KEY_Task(void *pvParameters)
{

	unsigned char key_event_r = KEY_NONE;

	while(1)
	{
		
		key_event_r = Keyboard();
		
		//单击判断-------------------------------------------------------
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
		
		//双击判断-------------------------------------------------------
		if(key_event_r == key_event[0][KEY_X_DOUBLE])
		{
			onenet_info.send_data |= SEND_TYPE_DATA;
			onenet_info.send_data |= SEND_TYPE_HEART;
			onenet_info.send_data |= SEND_TYPE_SUBSCRIBE;
			onenet_info.send_data |= SEND_TYPE_PUBLISH;
		}
		
		//长按判断-------------------------------------------------------
		if(key_event_r == key_event[0][KEY_X_DOWNLONG])
		{
			
		}
	
		RTOS_TimeDly(10); 								//挂起任务50ms
	
	}

}

/*
************************************************************
*	函数名称：	SENSOR_Task
*
*	函数功能：	传感器数据采集、显示
*
*	入口参数：	void类型的参数指针
*
*	返回参数：	无
*
*	说明：		传感器数据采集任务。进行外接传感器的数据采集、读取、显示
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
	SPILCD_Clear(BGC);									//清屏
	
	//标题显示
	SPILCD_DisZW(0, 0, RED, san);						//显示“三”
	SPILCD_DisZW(16, 0, RED, zhou);						//显示“轴”
	
	SPILCD_DisZW(0, 32, RED, wen);						//显示“温”
	SPILCD_DisZW(16, 32, RED, shi);						//显示“湿”
	SPILCD_DisZW(32, 32, RED, du);						//显示“度”
	
	SPILCD_DisZW(96, 64, RED, guang);					//显示“光”
	SPILCD_DisZW(112, 64, RED, min);					//显示“敏”
	
	SPILCD_DisZW(0, 64, RED, zhuang);					//显示“状”
	SPILCD_DisZW(16, 64, RED, tai);						//显示“态”
#else
	LCD1602_Clear(0xff);								//清屏
#endif

	while(1)
	{
		
		if(adxl362_info.device_ok) 						//只有设备存在时，才会读取值和显示
		{
			ADXL362_GetValue();							//采集传感器数据
				
#if(SPILCD_EN == 1)
			SPILCD_DisString(0, 16, 16, BLUE, 1, "X%0.1f,Y%0.1f,Z%0.1f    ", adxl362_info.x, adxl362_info.y, adxl362_info.z);
#else
			LCD1602_DisString(0x80, "X%0.1f,Y%0.1f,Z%0.1f", adxl362_info.x, adxl362_info.y, adxl362_info.z);
#endif
		}
		
		RTOS_TimeDly(1);
			
		if(sht20_info.device_ok) 						//只有设备存在时，才会读取值和显示
		{
			SHT20_GetValue();							//采集传感器数据
				
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
		SPILCD_BL_Ctl_Auto();							//自动调整SPILCD背光亮度
#endif
		
		if(++count >= 20)								//每隔一段时间发送一次红外数据
		{
			count = 0;
			
			NET_DEVICE_GetNWD();
		}
		
		RTOS_TimeDly(100); 								//挂起任务500ms
	
	}

}

/*
************************************************************
*	函数名称：	ALTER_Task
*
*	函数功能：	通过串口更改SSID、PSWD、DEVID、APIKEY
*
*	入口参数：	void类型的参数指针
*
*	返回参数：	无
*
*	说明：		更改后会保存到EEPROM里
************************************************************
*/
void ALTER_Task(void *pvParameters)
{

    while(1)
    {
    
		memset(alter_info.alter_buf, 0, sizeof(alter_info.alter_buf));
		
		while(!alter_info.rev_idle)
			RTOS_TimeDly(20);														//每100ms检查一次
		
		alter_info.rev_idle = 0;
		
		UsartPrintf(USART_DEBUG, "\r\nAlter Rev\r\n%s\r\n", alter_info.alter_buf);
        
		if(eeprom_info.device_ok)													//如果EEPROM存在
		{
			if(Info_Alter(alter_info.alter_buf))									//更改信息
			{
				onenet_info.status = ONENET_STATUS_DIS;
			}
		}
    
    }

}

/*
************************************************************
*	函数名称：	CLOCK_Task
*
*	函数功能：	网络校时、时间显示
*
*	入口参数：	void类型的参数指针
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void CLOCK_Task(void *pvParameter)
{
	
#if(NET_TIME_EN == 1)
	unsigned int second = 0, second_pre = 0;							//second是实时时间，second_pre差值比较
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
		if(get_net_time)												//需要获取时间
		{
			data_stream[0].flag = 0;									//不上传时间
			
			net_device_info.net_time = NET_DEVICE_GetTime();
			
			if(net_device_info.net_time)
			{
				second = RTC_GetCounter();
				
				if(((net_device_info.net_time <= second + 300) && (net_device_info.net_time >= second - 300)) || (second <= 100))
				{														//如果在±5分钟内，则认为时间正确
					RTC_SetTime(net_device_info.net_time + 4);			//设置RTC时间，加4是补上大概的时间差
					
					get_net_time = 0;
					
					data_stream[0].flag = 1;							//上传时间
				}
			}
		}

		second = RTC_GetCounter();										//获取秒值
		
		if(second > second_pre)
		{
			second_pre = second;
			time = localtime((const time_t *)&second);					//将秒值转为tm结构所表示的时间
			
			memset(myTime, 0, sizeof(myTime));
			snprintf(myTime, sizeof(myTime), "%d-%d-%d %d:%d:%d",
							time->tm_year + 1900, time->tm_mon + 1, time->tm_mday,
							time->tm_hour, time->tm_min, time->tm_sec);
#if(SPILCD_EN == 1)
			SPILCD_DisString(0, 115, 12, RED, 1, "%s     ", myTime);	//显示
#endif
		
			if(time->tm_hour == 0 && time->tm_min == 0 && time->tm_sec == 0)//每天0点时，更新一次时间
			{
				get_net_time = 1;
				net_device_info.net_time = 0;
			}
		}
#endif
		
		RTOS_TimeDly(20);													//挂起任务100ms
	
	}

}

/*
************************************************************
*	函数名称：	INIT_Task
*
*	函数功能：	初始化硬件、创建应用任务
*
*	入口参数：	void类型的参数指针
*
*	返回参数：	无
*
*	说明：		初始化任务
************************************************************
*/
void INIT_Task(void *pvParameters)
{

	Hardware_Init();								//硬件初始化
	
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
*	函数名称：	rt_application_init
*
*	函数功能：	任务创建
*
*	入口参数：	无
*
*	返回参数：	0
*
*	说明：		
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
