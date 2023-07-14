/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	onenet.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-05-27
	*
	*	版本： 		V1.0
	*
	*	说明： 		OneNET平台应用示例
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f10x.h"

//网络设备
#include "net_device.h"

//协议文件
#include "onenet.h"

//命令回调
#include "cmd_callback.h"

//硬件驱动
#include "usart.h"
#include "delay.h"

//C库
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


ONETNET_INFO onenet_info = {"设备ID", "产品ID", "鉴权信息", "masterkey", 0, ONENET_STATUS_AP};


GPS_INFO gps;


//==========================================================
//	函数名称：	OneNET_DevLink
//
//	函数功能：	与onenet创建连接
//
//	入口参数：	devid：创建设备的devid
//				proid：产品ID
//				auth_key：创建设备的masterKey或apiKey或设备鉴权信息
//
//	返回参数：	无
//
//	说明：		与onenet平台建立连接，成功或会标记oneNetInfo.netWork网络状态标志
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
					case 0:			UsartPrintf(USART_DEBUG, "Tips:	连接成功\r\n");					break;
					case 1:			UsartPrintf(USART_DEBUG, "Tips:	协议错误\r\n");					break;
					case 2:			UsartPrintf(USART_DEBUG, "Tips:	非法的clientid\r\n");			break;
					case 3:			UsartPrintf(USART_DEBUG, "Tips:	服务器失败\r\n");				break;
					case 4:			UsartPrintf(USART_DEBUG, "Tips:	用户名或密码错误\r\n");			break;
					case 5:			UsartPrintf(USART_DEBUG, "Tips:	法链接(比如token非法)\r\n");		break;
					case 6:			UsartPrintf(USART_DEBUG, "Tips:	未知错误\r\n");					break;
				}
				
				if(status)
				{
					while(try_cnt--)
					{
						status = OneNET_CheckStatus();
						
						if(status == ONENET_STATUS_OK)
						{
							UsartPrintf(USART_DEBUG, "Tips:	连接成功\r\n");
							
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
//	函数名称：	OneNET_SendData
//
//	函数功能：	上传数据到平台
//
//	入口参数：	streamArray：数据流
//				streamArrayNum：数据流个数
//
//	返回参数：	SEND_TYPE_OK-发送成功	SEND_TYPE_DATA-需要重送
//
//	说明：		
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
//	函数名称：	OneNET_Publish
//
//	函数功能：	发布消息
//
//	入口参数：	topic：发布的主题
//				msg：消息内容
//
//	返回参数：	SEND_TYPE_OK-成功	SEND_TYPE_PUBLISH-需要重送
//
//	说明：		
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
//	函数名称：	OneNET_Subscribe
//
//	函数功能：	订阅
//
//	入口参数：	topics：订阅的topic
//				topic_cnt：topic个数
//
//	返回参数：	SEND_TYPE_OK-成功	SEND_TYPE_SUBSCRIBE-需要重发
//
//	说明：		
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
//	函数名称：	OneNET_UnSubscribe
//
//	函数功能：	取消订阅
//
//	入口参数：	topics：订阅的topic
//				topic_cnt：topic个数
//
//	返回参数：	SEND_TYPE_OK-发送成功	SEND_TYPE_UNSUBSCRIBE-需要重发
//
//	说明：		
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
//	函数名称：	OneNET_GetLocation
//
//	函数功能：	获取LBS定位数据
//
//	入口参数：	api_key：apikey
//				lon：缓存lon坐标
//				lat：缓存lat坐标
//
//	返回参数：	0-成功	其他错误码
//
//	说明：		
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
//	函数名称：	OneNET_CheckStatus
//
//	函数功能：	查询与OneNET的连接状态
//
//	入口参数：	无
//
//	返回参数：	0-已接入		1-未接入OneNET	2-未接入路由		255-错误
//
//	说明：		
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
//	函数名称：	OneNET_CmdHandle
//
//	函数功能：	读取平台rb中的数据
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
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
