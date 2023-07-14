/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	net_task.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2018-03-29
	*
	*	版本： 		V1.0
	*
	*	说明： 		网络相关任务
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

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

//C库
#include <stdio.h>
#include <string.h>


//设备接收任务
#define RECV_TASK_PRIO				2
#define RECV_TASK_TICK				1
#define RECV_TASK_STK_SIZE			2048
ALIGN(RT_ALIGN_SIZE) unsigned char RECV_TASK_STK[RECV_TASK_STK_SIZE];
struct rt_thread RECV_Task_Handle;

//网络相关位任务
#define NET_FLAG_TASK_PRIO			3
#define NET_FLAG_TASK_TICK			1
#define NET_FLAG_TASK_STK_SIZE		2048
ALIGN(RT_ALIGN_SIZE) unsigned char NET_FLAG_TASK_STK[NET_FLAG_TASK_STK_SIZE];
struct rt_thread NET_FLAG_Task_Handle;

//数据发送任务
#define DATA_S_TASK_PRIO			4
#define DATA_S_TASK_TICK			1
#define DATA_S_TASK_STK_SIZE		1024
ALIGN(RT_ALIGN_SIZE) unsigned char DATA_S_TASK_STK[DATA_S_TASK_STK_SIZE];
struct rt_thread DATA_S_Task_Handle;

//网络初始化任务
#define NET_TASK_PRIO				5
#define NET_TASK_TICK				1
#define NET_TASK_STK_SIZE			2048
ALIGN(RT_ALIGN_SIZE) unsigned char NET_TASK_STK[NET_TASK_STK_SIZE];
struct rt_thread NET_Task_Handle;


const char *topics[] = {"kyLin_topic", "topic_test"};


extern DATA_STREAM data_stream[];
extern unsigned char data_stream_cnt;


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
__weak void NET_Event_CallBack(NET_EVENT net_event)
{

	switch((unsigned char)net_event)
	{
		case NET_EVENT_Timer_Check_Err:		//网络定时检查超时错误
		break;
		
		case NET_EVENT_Timer_Send_Err:		//网络发送失败错误
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
		break;
		case NET_EVENT_Device_Err:			//网络模组检测错误
		break;
		
		case NET_EVENT_Initialize:			//正在初始化网络模组
		break;
		
		case NET_EVENT_Init_Ok:				//网络模组初始化成功
		break;
		
		case NET_EVENT_Auto_Create_Ok:		//自动创建设备成功
		break;
		
		case NET_EVENT_Auto_Create_Err:		//自动创建设备失败
		break;
		
		case NET_EVENT_Connect:				//正在连接、登录OneNET
		break;
		
		case NET_EVENT_Connect_Ok:			//连接、登录成功
		break;
		
		case NET_EVENT_Connect_Err:			//连接、登录错误
		break;
		
		case NET_EVENT_Fault_Process:		//错误处理
		break;
		
		default:							//无
		break;
	}

}

/*
************************************************************
*	函数名称：	RECV_Task
*
*	函数功能：	处理平台下发的命令
*
*	入口参数：	void类型的参数指针
*
*	返回参数：	无
*
*	说明：		平台下发的命令并解析、处理
************************************************************
*/
void RECV_Task(void *pvParameters)
{

	while(1)
	{
		
		OneNET_CmdHandle();
		
		RTOS_TimeDly(4);
	
	}

}

/*
************************************************************
*	函数名称：	NET_FLAG_Task
*
*	函数功能：	网络相关标志
*
*	入口参数：	void类型的参数指针
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void NET_FLAG_Task(void *pvParameters)
{
	
	unsigned short data_count = 0, check_count = 0;
	
	unsigned char err_count = 0;

	while(1)
	{
		
		//数据-------------------------------------------------------------------
		if(++data_count >= 300)
		{
			data_count = 0;
			
			NET_Event_CallBack(NET_EVENT_Send_Data);
			
			onenet_info.send_data |= SEND_TYPE_DATA;							//发送数据
		}
		
		//接入检测---------------------------------------------------------------
		if(++check_count >= 200)
		{
			check_count = 0;
			
			if(OneNET_CheckStatus())
			{
				if(++err_count >= 3)
				{
					NET_DEVICE_Reset();
					onenet_info.status = ONENET_STATUS_AP;
				}
			}
			else
				err_count = 0;
		}
		
		RTOS_TimeDly(10);
	
	}

}

/*
************************************************************
*	函数名称：	DATA_S_Task
*
*	函数功能：	数据发送主任务
*
*	入口参数：	void类型的参数指针
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void DATA_S_Task(void *pvParameters)
{
	
	while(1)
	{
		
		if(onenet_info.send_data & SEND_TYPE_DATA)
		{
			if(OneNET_SendData(data_stream, data_stream_cnt) == SEND_TYPE_OK)									//上传数据到平台
				onenet_info.send_data &= ~SEND_TYPE_DATA;
		}
		
		if(onenet_info.send_data & SEND_TYPE_SUBSCRIBE)
		{
			if(OneNET_Subscribe(topics, 2) == SEND_TYPE_OK)														//订阅主题
				onenet_info.send_data &= ~SEND_TYPE_SUBSCRIBE;
		}
		
		if(onenet_info.send_data & SEND_TYPE_UNSUBSCRIBE)
		{
			if(OneNET_UnSubscribe(topics, 2) == SEND_TYPE_OK)													//取消订阅的主题
				onenet_info.send_data &= ~SEND_TYPE_UNSUBSCRIBE;
		}
		
		if(onenet_info.send_data & SEND_TYPE_PUBLISH)
		{
			if(OneNET_Publish("pc_topic", "Publish Test") == SEND_TYPE_OK)										//发布主题
				onenet_info.send_data &= ~SEND_TYPE_PUBLISH;
		}
		
		RTOS_TimeDly(10);												//挂起任务50ms
	
	}

}

/*
************************************************************
*	函数名称：	NET_Task
*
*	函数功能：	网络连接、平台接入
*
*	入口参数：	void类型的参数指针
*
*	返回参数：	无
*
*	说明：		网络连接任务任务。会在心跳检测里边检测网络连接状态，如果有错，会标记状态，然后在这里进行重连
************************************************************
*/
void NET_Task(void *pvParameters)
{
	
	NET_DEVICE_IO_Init();													//网络设备IO初始化
	NET_DEVICE_Reset();														//网络设备复位
	
	NET_Event_CallBack(NET_EVENT_Initialize);
	
	while(1)
	{
		
		if(onenet_info.status != ONENET_STATUS_OK)
		{
			if(!NET_DEVICE_Init())											//初始化网络设备，能连入网络
			{
				NET_Event_CallBack(NET_EVENT_Connect);
				
				OneNET_DevLink(onenet_info.dev_id, onenet_info.pro_id, onenet_info.auth_info);
				
				if(onenet_info.status == ONENET_STATUS_OK)
				{
					if(OneNET_GetLocation(onenet_info.api_key, gps.lon, gps.lat) == 0)
						gps.flag = 1;
					
					RTOS_TimeDly(10);
					
					onenet_info.send_data |= SEND_TYPE_SUBSCRIBE;			//连接成功则订阅主题
					
					NET_Event_CallBack(NET_EVENT_Connect_Ok);
				}
				else
				{
					NET_Event_CallBack(NET_EVENT_Connect_Err);
				}
			}
		}
		
		RTOS_TimeDly(5);
	
	}

}








/*
************************************************************
*	函数名称：	NET_Task_Init
*
*	函数功能：	网络相关任务
*
*	入口参数：	void类型的参数指针
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void NET_Task_Init(void)
{

	//创建应用任务
	
	if(rt_thread_init(&RECV_Task_Handle, "RECV", RECV_Task, RT_NULL,
						(unsigned char *)&RECV_TASK_STK[0], RECV_TASK_STK_SIZE, RECV_TASK_PRIO, RECV_TASK_TICK) == RT_EOK)
		rt_thread_startup(&RECV_Task_Handle);
	
	if(rt_thread_init(&NET_FLAG_Task_Handle, "NET_FLAG", NET_FLAG_Task, RT_NULL,
						(unsigned char *)&NET_FLAG_TASK_STK[0], NET_FLAG_TASK_STK_SIZE, NET_FLAG_TASK_PRIO, NET_FLAG_TASK_TICK) == RT_EOK)
		rt_thread_startup(&NET_FLAG_Task_Handle);
	
	if(rt_thread_init(&DATA_S_Task_Handle, "DATA_S", DATA_S_Task, RT_NULL,
						(unsigned char *)&DATA_S_TASK_STK[0], DATA_S_TASK_STK_SIZE, DATA_S_TASK_PRIO, DATA_S_TASK_TICK) == RT_EOK)
		rt_thread_startup(&DATA_S_Task_Handle);
	
	if(rt_thread_init(&NET_Task_Handle, "NET", NET_Task, RT_NULL,
						(unsigned char *)&NET_TASK_STK[0], NET_TASK_STK_SIZE, NET_TASK_PRIO, NET_TASK_TICK) == RT_EOK)
		rt_thread_startup(&NET_Task_Handle);

}
