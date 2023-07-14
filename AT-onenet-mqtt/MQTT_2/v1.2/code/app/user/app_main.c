/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	main.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2017-07-19
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		�û�����
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//ͨ��ͷ�ļ�
#include "esp_common.h"

//OS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

//Э��
#include "onenet.h"
#include "fault.h"

//��������
#include "net_device.h"
#include "lwip/lwip/sockets.h"

//������
#include "dataStreamName.h"


#define HEART_TIME		1000


char dp_name[32];
char dp_value[32];

char topics[16];

char publish_tp[16];
char publish_mg[32];


//������
DATA_STREAM data_stream[] = {
								{dp_name, &dp_value, TYPE_NUM, 0},
								{dp_name, &dp_value, TYPE_STRING, 0},
								{dp_name, &gps_info, TYPE_GPS, 0},
							};
unsigned char data_stream_cnt = sizeof(data_stream) / sizeof(data_stream[0]);


extern void wifi_handle_event_cb(System_Event_t *evt);


//ע��:	1.������ȼ�Ϊ14��
//		2.�������ȼ�Ϊ1, 3~9
//		3.����ջ��С��Χ 176~512

//OneNET���ݽ�������
#define ONENET_TASK_PRIO		9
#define ONENET_STK_SIZE			512
void ONENET_Task(void *pvParameters);

//ATָ���������
#define AT_TASK_PRIO			8
#define AT_STK_SIZE				512
void AT_Task(void *pvParameters);

//NET��־����
#define NET_FLAG_TASK_PRIO		4
#define NET_FLAG_STK_SIZE		512
void NET_FLAG_Task(void *pvParameters);

//NET����
#define NET_TASK_PRIO			3
#define NET_STK_SIZE			512
void NET_Task(void *pvParameters);

//��������
#define MISC_TASK_PRIO			1
#define MISC_STK_SIZE			512
void MISC_Task(void *pvParameters);


/*
************************************************************
*	�������ƣ�	user_app
*
*	�������ܣ�	�û�����
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void user_app(void)
{

	wifi_set_opmode(STATION_MODE);													//����Ϊstationģʽ
	wifi_set_event_handler_cb(wifi_handle_event_cb);								//ע��wifi�¼��ص�

	//��������
/*******************************************���������û��������***********************************************/
	xTaskCreate(ONENET_Task, "ONENET", ONENET_STK_SIZE, NULL, ONENET_TASK_PRIO, NULL);
	
	xTaskCreate(AT_Task, "AT", AT_STK_SIZE, NULL, AT_TASK_PRIO, NULL);
	
	xTaskCreate(NET_FLAG_Task, "NET_FLAG", NET_FLAG_STK_SIZE, NULL, NET_FLAG_TASK_PRIO, NULL);
	
	xTaskCreate(NET_Task, "NET", NET_STK_SIZE, NULL, NET_TASK_PRIO, NULL);
/****************************************************************************************************************/
	
	xTaskCreate(MISC_Task, "MISC", MISC_STK_SIZE, NULL, MISC_TASK_PRIO, NULL);

}


/*
************************************************************
*	�������ƣ�	ONENET_Task
*
*	�������ܣ�	OneNET���ݽ���
*
*	��ڲ�����	pvParameter: �������
*
*	���ز�����	��
*
*	˵����		
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
			if(NET_DEVICE_GetData(net_device_info.socket_fd, data_ptr, BUFFER_SIZE, MSG_DONTWAIT) > 0)	//ʹ��MSG_DONTWAIT��Ƚ��ȶ�
			{
				OneNET_RevPro(data_ptr);								//���д���
				
				for(i = 0; i < BUFFER_SIZE; i++)
					data_ptr[i] = 0;
			}
		}
		
		vTaskDelay(1);													//��������10ms

	}

}

/*
************************************************************
*	�������ƣ�	AT_Task
*
*	�������ܣ�	ATָ�����
*
*	��ڲ�����	pvParameter: �������
*
*	���ز�����	��
*
*	˵����		
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
*	�������ƣ�	NET_FLAG_Task
*
*	�������ܣ�	������ر�־
*
*	��ڲ�����	pvParameter: �������
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void NET_FLAG_Task(void *pvParameter)
{
	
	unsigned short heart_time = 0;

	while(1)
	{
		
		//������--------------------------------------------------------------------------------------------
		if(net_fault_info.net_fault_level != NET_FAULT_LEVEL_0)		//��������־������
		{
//			printf("WARN:	Fault Process\r\n");
			
			NET_Fault_Process();									//�����������
		}

		//�������ͱ�־----------------------------------------------------------------------------------------
		if(++heart_time >= HEART_TIME)
		{
			heart_time = 0;

			onenet_info.send_data |= SEND_TYPE_HEART;				//������������
		}
		
		//���ݷ���--------------------------------------------------------------------------------------------
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

		//�������--------------------------------------------------------------------------------------------
		OneNET_Check_Heart();
	
		vTaskDelay(1);
	}

}

/*
************************************************************
*	�������ƣ�	NET_Task
*
*	�������ܣ�	�������ӡ�OneNET��������
*
*	��ڲ�����	pvParameter: �������
*
*	���ز�����	��
*
*	˵����		
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
*	�������ƣ�	MISC_Task
*
*	�������ܣ�	����һЩ���๦��
*
*	��ڲ�����	pvParameter: �������
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void MISC_Task(void *pvParameter)
{

	while(1)
	{

		vTaskDelay(200);

	}

}

