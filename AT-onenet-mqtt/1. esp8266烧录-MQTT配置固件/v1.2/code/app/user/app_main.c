/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	main.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-07-19
	*
	*	版本： 		V1.0
	*
	*	说明： 		用户代码
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//通用头文件
#include "esp_common.h"

//OS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

//协议
#include "onenet.h"
#include "fault.h"

//网络驱动
#include "net_device.h"
#include "lwip/lwip/sockets.h"

//数据流
#include "dataStreamName.h"


#define HEART_TIME		1000


char dp_name[32];
char dp_value[32];

char topics[16];

char publish_tp[16];
char publish_mg[32];


//数据流
DATA_STREAM data_stream[] = {
								{dp_name, &dp_value, TYPE_NUM, 0},
								{dp_name, &dp_value, TYPE_STRING, 0},
								{dp_name, &gps_info, TYPE_GPS, 0},
							};
unsigned char data_stream_cnt = sizeof(data_stream) / sizeof(data_stream[0]);


extern void wifi_handle_event_cb(System_Event_t *evt);


//注意:	1.最高优先级为14。
//		2.可用优先级为1, 3~9
//		3.任务栈大小范围 176~512

//OneNET数据接收任务
#define ONENET_TASK_PRIO		9
#define ONENET_STK_SIZE			512
void ONENET_Task(void *pvParameters);

//AT指令解析任务
#define AT_TASK_PRIO			8
#define AT_STK_SIZE				512
void AT_Task(void *pvParameters);

//NET标志任务
#define NET_FLAG_TASK_PRIO		4
#define NET_FLAG_STK_SIZE		512
void NET_FLAG_Task(void *pvParameters);

//NET任务
#define NET_TASK_PRIO			3
#define NET_STK_SIZE			512
void NET_Task(void *pvParameters);

//杂类任务
#define MISC_TASK_PRIO			1
#define MISC_STK_SIZE			512
void MISC_Task(void *pvParameters);


/*
************************************************************
*	函数名称：	user_app
*
*	函数功能：	用户代码
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void user_app(void)
{

	wifi_set_opmode(STATION_MODE);													//设置为station模式
	wifi_set_event_handler_cb(wifi_handle_event_cb);								//注册wifi事件回调

	//创建任务
/*******************************************网络任务，用户无需关心***********************************************/
	xTaskCreate(ONENET_Task, "ONENET", ONENET_STK_SIZE, NULL, ONENET_TASK_PRIO, NULL);
	
	xTaskCreate(AT_Task, "AT", AT_STK_SIZE, NULL, AT_TASK_PRIO, NULL);
	
	xTaskCreate(NET_FLAG_Task, "NET_FLAG", NET_FLAG_STK_SIZE, NULL, NET_FLAG_TASK_PRIO, NULL);
	
	xTaskCreate(NET_Task, "NET", NET_STK_SIZE, NULL, NET_TASK_PRIO, NULL);
/****************************************************************************************************************/
	
	xTaskCreate(MISC_Task, "MISC", MISC_STK_SIZE, NULL, MISC_TASK_PRIO, NULL);

}


/*
************************************************************
*	函数名称：	ONENET_Task
*
*	函数功能：	OneNET数据接收
*
*	入口参数：	pvParameter: 任务参数
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
#define BUFFER_SIZE		256
void ONENET_Task(void *pvParameter)
{

	unsigned short i = 0;
	
	unsigned char *data_ptr = (unsigned char *)os_malloc(BUFFER_SIZE);

	if(data_ptr == NULL)
	{
		while(1)
		{
			UartPrintf("+Event:ONENET_Task-Malloc Err.please Reboot\r\n");

			vTaskDelay(500);
		}
	}

	while(1)
	{
		
		if(onenet_info.connect_ip)
		{
			if(NET_DEVICE_GetData(net_device_info.socket_fd, data_ptr, BUFFER_SIZE, MSG_DONTWAIT) > 0)	//使用MSG_DONTWAIT会比较稳定
			{
				OneNET_RevPro(data_ptr);								//集中处理
				
				for(i = 0; i < BUFFER_SIZE; i++)
					data_ptr[i] = 0;
			}
		}
		
		vTaskDelay(1);													//挂起任务10ms

	}

}

/*
************************************************************
*	函数名称：	AT_Task
*
*	函数功能：	AT指令解析
*
*	入口参数：	pvParameter: 任务参数
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void AT_Task(void *pvParameter)
{

	while(1)
	{
		
		AT_Recv_Pro();
	
		vTaskDelay(5);
	}

}

/*
************************************************************
*	函数名称：	NET_FLAG_Task
*
*	函数功能：	网络相关标志
*
*	入口参数：	pvParameter: 任务参数
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void NET_FLAG_Task(void *pvParameter)
{
	
	unsigned short heart_time = 0;

	while(1)
	{
		
		//错误处理--------------------------------------------------------------------------------------------
		if(net_fault_info.net_fault_level != NET_FAULT_LEVEL_0)		//如果错误标志被设置
		{
//			printf("WARN:	Fault Process\r\n");
			
			NET_Fault_Process();									//进入错误处理函数
		}

		//心跳发送标志----------------------------------------------------------------------------------------
		if(++heart_time >= HEART_TIME)
		{
			heart_time = 0;

			onenet_info.send_data |= SEND_TYPE_HEART;				//发送心跳请求
		}
		
		//数据发送--------------------------------------------------------------------------------------------
		if(onenet_info.send_data & SEND_TYPE_DATA)
		{
			if(OneNET_SendData(FORMAT_TYPE3, NULL, NULL, data_stream, data_stream_cnt) == SEND_TYPE_OK)
			{
				unsigned char i = 0;
				
				onenet_info.send_data &= ~SEND_TYPE_DATA;
				
				for(; i < data_stream_cnt; i++)
				{
					if(data_stream[i].flag == 1)
						data_stream[i].flag = 0;
				}
			}
		}
		
		if(onenet_info.send_data & SEND_TYPE_SUBSCRIBE)
		{
			if(OneNET_Subscribe(topics) == SEND_TYPE_OK)
				onenet_info.send_data &= ~SEND_TYPE_SUBSCRIBE;
		}
		
		if(onenet_info.send_data & SEND_TYPE_UNSUBSCRIBE)
		{
			if(OneNET_UnSubscribe(topics) == SEND_TYPE_OK)
				onenet_info.send_data &= ~SEND_TYPE_UNSUBSCRIBE;
		}
		
		if(onenet_info.send_data & SEND_TYPE_PUBLISH)
		{
			if(OneNET_Publish(publish_tp, publish_mg) == SEND_TYPE_OK)
				onenet_info.send_data &= ~SEND_TYPE_PUBLISH;
		}
		
		if(onenet_info.send_data & SEND_TYPE_HEART)
		{
			if(OneNET_SendData_Heart() == SEND_TYPE_OK)
				onenet_info.send_data &= ~SEND_TYPE_HEART;
		}
		
		if(onenet_info.send_data & SEND_TYPE_GPS)
		{
			if(OneNET_GetLocation(onenet_info.dev_id, onenet_info.api_key, gps_info.lon, gps_info.lat) == SEND_TYPE_OK)
			{
				onenet_info.send_data &= ~SEND_TYPE_GPS;
				
				gps_info.flag = 1;
			}
		}

		//心跳检测--------------------------------------------------------------------------------------------
		OneNET_Check_Heart();
	
		vTaskDelay(1);
	}

}

/*
************************************************************
*	函数名称：	NET_Task
*
*	函数功能：	网络连接、OneNET接入任务
*
*	入口参数：	pvParameter: 任务参数
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void NET_Task(void *pvParameter)
{

	while(1)
	{

		if(onenet_info.reconnect == 1)
		{
			OneNET_ConnectIP(onenet_info.ip, onenet_info.port);
					
			OneNET_DevLink(onenet_info.dev_id, onenet_info.pro_id, onenet_info.auth_info);
			
			if(onenet_info.net_work)
			{
				onenet_info.reconnect = 0;
				
				net_fault_info.net_fault_flag = 1;
			}
		}
		
		vTaskDelay(10);
		
	}

}

/*
************************************************************
*	函数名称：	MISC_Task
*
*	函数功能：	处理一些杂类功能
*
*	入口参数：	pvParameter: 任务参数
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void MISC_Task(void *pvParameter)
{

	while(1)
	{

		vTaskDelay(200);

	}

}

