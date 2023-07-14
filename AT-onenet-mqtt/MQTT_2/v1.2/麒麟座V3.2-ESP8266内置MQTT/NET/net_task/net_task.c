/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	net_task.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2018-03-29
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		�����������
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

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

//C��
#include <stdio.h>
#include <string.h>


//�豸��������
#define RECV_TASK_PRIO				2
#define RECV_TASK_TICK				1
#define RECV_TASK_STK_SIZE			2048
ALIGN(RT_ALIGN_SIZE) unsigned char RECV_TASK_STK[RECV_TASK_STK_SIZE];
struct rt_thread RECV_Task_Handle;

//�������λ����
#define NET_FLAG_TASK_PRIO			3
#define NET_FLAG_TASK_TICK			1
#define NET_FLAG_TASK_STK_SIZE		2048
ALIGN(RT_ALIGN_SIZE) unsigned char NET_FLAG_TASK_STK[NET_FLAG_TASK_STK_SIZE];
struct rt_thread NET_FLAG_Task_Handle;

//���ݷ�������
#define DATA_S_TASK_PRIO			4
#define DATA_S_TASK_TICK			1
#define DATA_S_TASK_STK_SIZE		1024
ALIGN(RT_ALIGN_SIZE) unsigned char DATA_S_TASK_STK[DATA_S_TASK_STK_SIZE];
struct rt_thread DATA_S_Task_Handle;

//�����ʼ������
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
__weak void NET_Event_CallBack(NET_EVENT net_event)
{

	switch((unsigned char)net_event)
	{
		case NET_EVENT_Timer_Check_Err:		//���綨ʱ��鳬ʱ����
		break;
		
		case NET_EVENT_Timer_Send_Err:		//���緢��ʧ�ܴ���
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
		break;
		case NET_EVENT_Device_Err:			//����ģ�������
		break;
		
		case NET_EVENT_Initialize:			//���ڳ�ʼ������ģ��
		break;
		
		case NET_EVENT_Init_Ok:				//����ģ���ʼ���ɹ�
		break;
		
		case NET_EVENT_Auto_Create_Ok:		//�Զ������豸�ɹ�
		break;
		
		case NET_EVENT_Auto_Create_Err:		//�Զ������豸ʧ��
		break;
		
		case NET_EVENT_Connect:				//�������ӡ���¼OneNET
		break;
		
		case NET_EVENT_Connect_Ok:			//���ӡ���¼�ɹ�
		break;
		
		case NET_EVENT_Connect_Err:			//���ӡ���¼����
		break;
		
		case NET_EVENT_Fault_Process:		//������
		break;
		
		default:							//��
		break;
	}

}

/*
************************************************************
*	�������ƣ�	RECV_Task
*
*	�������ܣ�	����ƽ̨�·�������
*
*	��ڲ�����	void���͵Ĳ���ָ��
*
*	���ز�����	��
*
*	˵����		ƽ̨�·����������������
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
*	�������ƣ�	NET_FLAG_Task
*
*	�������ܣ�	������ر�־
*
*	��ڲ�����	void���͵Ĳ���ָ��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void NET_FLAG_Task(void *pvParameters)
{
	
	unsigned short data_count = 0, check_count = 0;
	
	unsigned char err_count = 0;

	while(1)
	{
		
		//����-------------------------------------------------------------------
		if(++data_count >= 300)
		{
			data_count = 0;
			
			NET_Event_CallBack(NET_EVENT_Send_Data);
			
			onenet_info.send_data |= SEND_TYPE_DATA;							//��������
		}
		
		//������---------------------------------------------------------------
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
*	�������ƣ�	DATA_S_Task
*
*	�������ܣ�	���ݷ���������
*
*	��ڲ�����	void���͵Ĳ���ָ��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void DATA_S_Task(void *pvParameters)
{
	
	while(1)
	{
		
		if(onenet_info.send_data & SEND_TYPE_DATA)
		{
			if(OneNET_SendData(data_stream, data_stream_cnt) == SEND_TYPE_OK)									//�ϴ����ݵ�ƽ̨
				onenet_info.send_data &= ~SEND_TYPE_DATA;
		}
		
		if(onenet_info.send_data & SEND_TYPE_SUBSCRIBE)
		{
			if(OneNET_Subscribe(topics, 2) == SEND_TYPE_OK)														//��������
				onenet_info.send_data &= ~SEND_TYPE_SUBSCRIBE;
		}
		
		if(onenet_info.send_data & SEND_TYPE_UNSUBSCRIBE)
		{
			if(OneNET_UnSubscribe(topics, 2) == SEND_TYPE_OK)													//ȡ�����ĵ�����
				onenet_info.send_data &= ~SEND_TYPE_UNSUBSCRIBE;
		}
		
		if(onenet_info.send_data & SEND_TYPE_PUBLISH)
		{
			if(OneNET_Publish("pc_topic", "Publish Test") == SEND_TYPE_OK)										//��������
				onenet_info.send_data &= ~SEND_TYPE_PUBLISH;
		}
		
		RTOS_TimeDly(10);												//��������50ms
	
	}

}

/*
************************************************************
*	�������ƣ�	NET_Task
*
*	�������ܣ�	�������ӡ�ƽ̨����
*
*	��ڲ�����	void���͵Ĳ���ָ��
*
*	���ز�����	��
*
*	˵����		���������������񡣻������������߼����������״̬������д�����״̬��Ȼ���������������
************************************************************
*/
void NET_Task(void *pvParameters)
{
	
	NET_DEVICE_IO_Init();													//�����豸IO��ʼ��
	NET_DEVICE_Reset();														//�����豸��λ
	
	NET_Event_CallBack(NET_EVENT_Initialize);
	
	while(1)
	{
		
		if(onenet_info.status != ONENET_STATUS_OK)
		{
			if(!NET_DEVICE_Init())											//��ʼ�������豸������������
			{
				NET_Event_CallBack(NET_EVENT_Connect);
				
				OneNET_DevLink(onenet_info.dev_id, onenet_info.pro_id, onenet_info.auth_info);
				
				if(onenet_info.status == ONENET_STATUS_OK)
				{
					if(OneNET_GetLocation(onenet_info.api_key, gps.lon, gps.lat) == 0)
						gps.flag = 1;
					
					RTOS_TimeDly(10);
					
					onenet_info.send_data |= SEND_TYPE_SUBSCRIBE;			//���ӳɹ���������
					
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
*	�������ƣ�	NET_Task_Init
*
*	�������ܣ�	�����������
*
*	��ڲ�����	void���͵Ĳ���ָ��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void NET_Task_Init(void)
{

	//����Ӧ������
	
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
