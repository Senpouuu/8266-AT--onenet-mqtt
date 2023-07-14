#ifndef _NET_TASK_H_
#define _NET_TASK_H_





typedef enum
{

	NET_EVENT_Timer_Check_Err = 0,			//网络定时检查超时错误
	NET_EVENT_Timer_Send_Err,				//网络发送失败错误
	
	NET_EVENT_Send_HeartBeat,				//即将发送心跳包
	NET_EVENT_Send_Data,					//即将发送数据点
	NET_EVENT_Send_Subscribe,				//即将发送订阅数据
	NET_EVENT_Send_UnSubscribe,				//即将发送取消订阅数据
	NET_EVENT_Send_Publish,					//即将发送推送数据
	
	NET_EVENT_Send,							//开始发送数据
	NET_EVENT_Recv,							//Modbus用-收到数据查询指令
	
	NET_EVENT_Check_Status,					//进入网络模组状态检查
	
	NET_EVENT_Device_Ok,					//网络模组检测Ok
	NET_EVENT_Device_Err,					//网络模组检测错误
	
	NET_EVENT_Initialize,					//正在初始化网络模组
	NET_EVENT_Init_Ok,						//网络模组初始化成功
	
	NET_EVENT_Auto_Create_Ok,				//自动创建设备成功
	NET_EVENT_Auto_Create_Err,				//自动创建设备失败
	
	NET_EVENT_Connect,						//正在连接、登录OneNET
	NET_EVENT_Connect_Ok,					//连接、登录成功
	NET_EVENT_Connect_Err,					//连接、登录错误
	
	NET_EVENT_Fault_Process,				//进入错误处理

} NET_EVENT;


__weak void NET_Event_CallBack(NET_EVENT net_event);

void NET_Task_Init(void);


#endif
