/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	onenet.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2017-05-27
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		OneNETƽ̨Ӧ��ʾ��
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"

//�����豸
#include "net_device.h"

//Э���ļ�
#include "onenet.h"

//����ص�
#include "cmd_callback.h"

//Ӳ������
#include "usart.h"
#include "delay.h"

//C��
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


ONETNET_INFO onenet_info = {"�豸ID", "��ƷID", "��Ȩ��Ϣ", "masterkey", 0, ONENET_STATUS_AP};


GPS_INFO gps;


//==========================================================
//	�������ƣ�	OneNET_DevLink
//
//	�������ܣ�	��onenet��������
//
//	��ڲ�����	devid�������豸��devid
//				proid����ƷID
//				auth_key�������豸��masterKey��apiKey���豸��Ȩ��Ϣ
//
//	���ز�����	��
//
//	˵����		��onenetƽ̨�������ӣ��ɹ������oneNetInfo.netWork����״̬��־
//==========================================================
void OneNET_DevLink(const char *devid, const char *proid, const char *auth_info)
{
	
	if(onenet_info.status != ONENET_STATUS_OK)
	{
		char send_buf[64];
		unsigned char try_cnt = 3;
		unsigned int status = 0;
		
		snprintf(send_buf, sizeof(send_buf), "AT+IOTCFG=%s,%s,%s\r\n", devid, proid, auth_info);
		
		if(NET_DEVICE_SendCmd(send_buf, "+Event:Connect:", 400) == 0)
		{
			if(sscanf(net_device_info.cmd_resp, "+Event:Connect:%d", &status) == 1)
			{
				switch(status)
				{
					case 0:			UsartPrintf(USART_DEBUG, "Tips:	���ӳɹ�\r\n");					break;
					case 1:			UsartPrintf(USART_DEBUG, "Tips:	Э�����\r\n");					break;
					case 2:			UsartPrintf(USART_DEBUG, "Tips:	�Ƿ���clientid\r\n");			break;
					case 3:			UsartPrintf(USART_DEBUG, "Tips:	������ʧ��\r\n");				break;
					case 4:			UsartPrintf(USART_DEBUG, "Tips:	�û������������\r\n");			break;
					case 5:			UsartPrintf(USART_DEBUG, "Tips:	������(����token�Ƿ�)\r\n");		break;
					case 6:			UsartPrintf(USART_DEBUG, "Tips:	δ֪����\r\n");					break;
				}
				
				if(status)
				{
					while(try_cnt--)
					{
						status = OneNET_CheckStatus();
						
						if(status == ONENET_STATUS_OK)
						{
							UsartPrintf(USART_DEBUG, "Tips:	���ӳɹ�\r\n");
							
							break;
						}
						
						RTOS_TimeDly(20);
					}
				}
				
				onenet_info.status = status;
			}
		}
	}
	
}

//==========================================================
//	�������ƣ�	OneNET_SendData
//
//	�������ܣ�	�ϴ����ݵ�ƽ̨
//
//	��ڲ�����	streamArray��������
//				streamArrayNum������������
//
//	���ز�����	SEND_TYPE_OK-���ͳɹ�	SEND_TYPE_DATA-��Ҫ����
//
//	˵����		
//==========================================================
unsigned char OneNET_SendData(DATA_STREAM *streamArray, unsigned short streamArrayCnt)
{
	
	unsigned char i = 0, j = 0, send_count = 0;
	char send_buf[64];
	
	if(onenet_info.status != ONENET_STATUS_OK)
		return SEND_TYPE_DATA;
	
	for(; i < streamArrayCnt; i++)
	{
		if(streamArray[i].flag == 1)
		{
			memset(send_buf, 0, sizeof(send_buf));
			DSTREAM_GetDataStream_Body(send_buf, sizeof(send_buf), streamArray[i].dataType, streamArray[i].name, streamArray[i].dataPoint);
			
			send_count = 0;
			while((NET_DEVICE_SendCmd(send_buf, "Send OK", 200)) && (++send_count <=3))
				RTOS_TimeDly(100);
			
			if(send_count >= 3)
				UsartPrintf(USART_DEBUG, "ERROR:	%s:%s Send Err\r\n", streamArray[i].name, (char *)streamArray[i].dataPoint);
			else
				j++;
			
			//RTOS_TimeDly(10);
		}
	}
	
	UsartPrintf(USART_DEBUG, "Tips:	Send %d of %d Data Point\r\n", j, streamArrayCnt);
	
	return SEND_TYPE_OK;
	
}

//==========================================================
//	�������ƣ�	OneNET_Publish
//
//	�������ܣ�	������Ϣ
//
//	��ڲ�����	topic������������
//				msg����Ϣ����
//
//	���ز�����	SEND_TYPE_OK-�ɹ�	SEND_TYPE_PUBLISH-��Ҫ����
//
//	˵����		
//==========================================================
unsigned char OneNET_Publish(const char *topic, const char *msg)
{

	unsigned short len = 0;
	unsigned char result = 255;
	char *pub_buf = NULL;
	
	if(onenet_info.status != ONENET_STATUS_OK)
		return SEND_TYPE_PUBLISH;
	
	len = strlen(topic) + strlen(msg) + 14;
	
	pub_buf = NET_MallocBuffer(len);
	if(pub_buf == NULL)
		return SEND_TYPE_PUBLISH;
	
	memset(pub_buf, 0, len);
	
	UsartPrintf(USART_DEBUG, "Tips:	Publish, Topic:%s, Msg:%s\r\n", topic, msg);
	
	snprintf(pub_buf, len, "AT+IOTPUB=%s,%s\r\n", topic, msg);
	
	if(NET_DEVICE_SendCmd(pub_buf, "Publish", 200) == 0)
	{
		UsartPrintf(USART_DEBUG, "Tips:	Publish Ok\r\n");
		result = SEND_TYPE_OK;
	}
	else
	{
		UsartPrintf(USART_DEBUG, "Tips:	Publish Err\r\n");
		result = SEND_TYPE_PUBLISH;
	}
	
	NET_FreeBuffer(pub_buf);
	
	return result;

}

//==========================================================
//	�������ƣ�	OneNET_Subscribe
//
//	�������ܣ�	����
//
//	��ڲ�����	topics�����ĵ�topic
//				topic_cnt��topic����
//
//	���ز�����	SEND_TYPE_OK-�ɹ�	SEND_TYPE_SUBSCRIBE-��Ҫ�ط�
//
//	˵����		
//==========================================================
unsigned char OneNET_Subscribe(const char *topics[], unsigned char topic_cnt)
{
	
	char sub_buf[32];
	unsigned char i = 0;
	
	if(onenet_info.status != ONENET_STATUS_OK)
		return SEND_TYPE_SUBSCRIBE;
	
	for(; i < topic_cnt;)
	{
		UsartPrintf(USART_DEBUG, "Tips:	Subscribe, Topic:%s,", topics[i]);
		
		snprintf(sub_buf, sizeof(sub_buf), "AT+IOTSUB=%s\r\n", topics[i]);
		
		if(NET_DEVICE_SendCmd(sub_buf, "Subscribe OK", 200) == 0)
		{
			UsartPrintf(USART_DEBUG, " Ok\r\n");
			
			i++;
		}
		else
			UsartPrintf(USART_DEBUG, " Err\r\n");
	}
	
	return SEND_TYPE_OK;

}

//==========================================================
//	�������ƣ�	OneNET_UnSubscribe
//
//	�������ܣ�	ȡ������
//
//	��ڲ�����	topics�����ĵ�topic
//				topic_cnt��topic����
//
//	���ز�����	SEND_TYPE_OK-���ͳɹ�	SEND_TYPE_UNSUBSCRIBE-��Ҫ�ط�
//
//	˵����		
//==========================================================
unsigned char OneNET_UnSubscribe(const char *topics[], unsigned char topic_cnt)
{
	
	char unsub_buf[32];
	unsigned char i = 0;
	
	if(onenet_info.status != ONENET_STATUS_OK)
		return SEND_TYPE_UNSUBSCRIBE;
	
	for(; i < topic_cnt;)
	{
		UsartPrintf(USART_DEBUG, "Tips:	UnSubscribe, Topic:%s,", topics[i]);
		
		snprintf(unsub_buf, sizeof(unsub_buf), "AT+IOTUNSUB=%s\r\n", topics[i]);
		
		if(NET_DEVICE_SendCmd(unsub_buf, "UnSubscribe OK", 200) == 0)
		{
			UsartPrintf(USART_DEBUG, " Ok\r\n");
			
			i++;
		}
		else
			UsartPrintf(USART_DEBUG, " Err\r\n");
	}
	
	return SEND_TYPE_OK;

}

//==========================================================
//	�������ƣ�	OneNET_GetLocation
//
//	�������ܣ�	��ȡLBS��λ����
//
//	��ڲ�����	api_key��apikey
//				lon������lon����
//				lat������lat����
//
//	���ز�����	0-�ɹ�	����������
//
//	˵����		
//==========================================================
unsigned char OneNET_GetLocation(const char *api_key, char *lon, char *lat)
{
	
	unsigned char result = 1;

#if(LBS_WIFI_EN == 1)
	char cmd_buf[48];
	
	if(api_key == NULL || lon == NULL || lat == NULL)
		return result;
	
	if(strlen(api_key) < 28)
		return result;
	
	snprintf(cmd_buf, sizeof(cmd_buf), "AT+IOTGPS=%s\r\n", api_key);
	
	if(NET_DEVICE_SendCmd(cmd_buf, "+Event:lon:", 200) == 0)
	{
		if(sscanf(net_device_info.cmd_resp, "+Event:lon:%[^,], lat:%[^\r]", lon, lat) == 2)
		{
			UsartPrintf(USART_DEBUG, "lon:%s, lat: %s\r\n", lon, lat);
			
			result = 0;
		}
	}
			
#endif
	
	return result;

}

//==========================================================
//	�������ƣ�	OneNET_CheckStatus
//
//	�������ܣ�	��ѯ��OneNET������״̬
//
//	��ڲ�����	��
//
//	���ز�����	0-�ѽ���		1-δ����OneNET	2-δ����·��		255-����
//
//	˵����		
//==========================================================
unsigned int OneNET_CheckStatus(void)
{
	
	unsigned int status = 255;
	
	if(NET_DEVICE_SendCmd("AT+IOTSTATUS\r\n", "+IOT:status:", 200) == 0)
	{
		if(sscanf(net_device_info.cmd_resp, "+IOT:status:%d", &status) == 1)
		{
			
		}
	}
	
	return status;

}

//==========================================================
//	�������ƣ�	OneNET_CmdHandle
//
//	�������ܣ�	��ȡƽ̨rb�е�����
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void OneNET_CmdHandle(void)
{
	
	unsigned char *dataPtr = NULL, i = 0;

	dataPtr = NET_DEVICE_Read();

	if(dataPtr != NULL)
	{
		if(strstr((char *)dataPtr, "+CMD,"))
		{
			UsartPrintf(USART_DEBUG, "Tips:	Cmd Recv: %s\r\n", dataPtr);
			
			i = 0;
			while(i < 2)
			{
				if(*dataPtr++ == ',')
					i++;
			}
			
			CALLBACK_Execute((char *)dataPtr);
		}
		else if(strstr((char *)dataPtr, "+PUB,"))
		{
			UsartPrintf(USART_DEBUG, "Tips:	Pub Recv: %s\r\n", dataPtr);
			
			i = 0;
			while(i < 4)
			{
				if(*dataPtr++ == ',')
					i++;
			}
			
			CALLBACK_Execute((char *)dataPtr);
		}
		else
			NET_DEVICE_CmdHandle((char *)dataPtr);
	}

}
