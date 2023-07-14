/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	onenet.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-07-21
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

//OS
#include "freertos/FreeRTOS.h"

//网络设备
#include "net_device.h"
#include "lwip/lwip/sockets.h"
#include "esp_libc.h"

//协议文件
#include "onenet.h"
#include "fault.h"
#include "mqttkit.h"

//C库
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


//如果要采用自动创建设备的方式，apikey必须为master-key，且正式环境注册码有效
//如果填了devid，则不会自动创建设备，采用devid和apikey方式直接登录
ONETNET_INFO onenet_info = {"", "",
							"", "",
							"",
							"183.230.40.39", 6002,
							7, 0,
							0, 0, 1, 0, 0, 0, 0};


GPS_INFO gps_info;


//==========================================================
//	函数名称：	OneNET_RepetitionCreateFlag
//
//	函数功能：	允许重复注册设备
//
//	入口参数：	apikey：必须是masterkey
//
//	返回参数：	0-成功		1-失败
//
//	说明：		允许重复注册，否则第一次创建成功之后，再次创建会失败
//==========================================================
_Bool OneNET_RepetitionCreateFlag(const char *apikey)
{
	
	_Bool result = 1;
	unsigned char time_out = 200;
	int socket_fd = -1;
	char *send_buf = NULL;
	unsigned char *data_ptr = NULL;
	
	send_buf = (char *)os_malloc(136);
	if(send_buf == NULL)
		return result;
	
	data_ptr = (unsigned char *)os_malloc(320);
	if(data_ptr == NULL)
		return result;
	
	socket_fd = NET_DEVICE_GetSocket();
	
	if(NET_DEVICE_Connect(socket_fd, "183.230.40.33", 80) == 0)
	{
		snprintf(send_buf, 136, "PUT /register_attr HTTP/1.1\r\napi-key:%s\r\nHost:api.heclouds.com\r\n"
						"Content-Length:19\r\n\r\n"
						"{\"allow_dup\": true}", apikey);
		
		if(NET_DEVICE_SendData(socket_fd, send_buf, strlen(send_buf)) > 0)
		{
			while(--time_out)
			{
				if(NET_DEVICE_GetData(socket_fd, data_ptr, 320, MSG_DONTWAIT) > 0)
					break;
				
				vTaskDelay(1);
			}
			
			if(time_out)
			{
				if(strstr(data_ptr, "succ"))
				{
//					printf("Tips:	OneNET_RepetitionCreateFlag Ok\r\n");
					result = 0;
				}
				else if(strstr(data_ptr, "auth failed"))
				{
//					printf("WARN:	当前使用的不是masterkey 或 apikey错误\r\n");
				}
				else
					;//printf("Tips:	OneNET_RepetitionCreateFlag Err\r\n");
			}
			else
				;//printf("Tips:	OneNET_RepetitionCreateFlag Time Out\r\n");
		}
	}
	
	NET_DEVICE_Close(&socket_fd);
	
	os_free(send_buf);
	os_free(data_ptr);
	
	return result;

}

//==========================================================
//	函数名称：	OneNET_CreateDevice
//
//	函数功能：	在产品中创建一个设备
//
//	入口参数：	reg_code：正式环境注册码
//				dev_name：设备名
//				auth_info：鉴权信息
//				devid：保存返回的devid
//				apikey：保存返回的apikey
//
//	返回参数：	0-成功		1-失败
//
//	说明：		
//==========================================================
_Bool OneNET_CreateDevice(const char *reg_code, const char *dev_name, const char *auth_info, char *devid, char *apikey)
{
	
	_Bool result = 1;
	int socket_fd = -1;
	unsigned short send_len = 20 + strlen(dev_name) + strlen(auth_info);
	unsigned char time_out = 200;
	char *send_buf = NULL, *data_ptr = NULL, *data_ptr_t = NULL;
	
	send_buf = os_malloc(send_len + 140);
	if(send_buf == NULL)
		return result;
	
	data_ptr = (unsigned char *)os_malloc(320);
	if(data_ptr == NULL)
		return result;
	
	socket_fd = NET_DEVICE_GetSocket();
	
	if(NET_DEVICE_Connect(socket_fd, "183.230.40.33", 80) == 0)
	{
		snprintf(send_buf, 140 + send_len, "POST /register_de?register_code=%s HTTP/1.1\r\n"
						"Host: api.heclouds.com\r\n"
						"Content-Length:%d\r\n\r\n"
						"{\"sn\":\"%s\",\"title\":\"%s\"}",
		
						reg_code, send_len, auth_info, dev_name);
		
		if(NET_DEVICE_SendData(socket_fd, send_buf, strlen(send_buf)) > 0)
		{
			while(--time_out)
			{
				if(NET_DEVICE_GetData(socket_fd, data_ptr, 320, MSG_DONTWAIT) > 0)
					break;
				
				vTaskDelay(1);
			}
			
			if(time_out)
			{
				data_ptr_t = strstr(data_ptr, "device_id");
				
				if(strstr(data_ptr, "auth failed"))
				{
//					printf("WARN:	正式环境注册码错误\r\n");
				}
			}
			
			if(data_ptr_t)
			{
				if(sscanf(data_ptr_t, "device_id\":\"%[^\"]\",\"key\":\"%[^\"]\"", devid, apikey) == 2)
				{
//					printf("create device: %s, %s\r\n", devid, apikey);
					result = 0;
				}
			}
		}
	}
	
	NET_DEVICE_Close(&socket_fd);
	
	os_free(send_buf);
	os_free(data_ptr);
	
	return result;

}

//==========================================================
//	函数名称：	OneNET_GetLocation
//
//	函数功能：	获取LBS定位数据
//
//	入口参数：	device_id：设备ID
//				api_key：apikey
//				lon：缓存lon坐标
//				lat：缓存lat坐标
//
//	返回参数：	0-成功	其他错误码
//
//	说明：		
//==========================================================
unsigned char OneNET_GetLocation(const char *device_id, const char *api_key, char *lon, char *lat)
{
	
	unsigned char result = SEND_TYPE_GPS;

	char *data_ptr = NULL;
	char *send_buf = NULL;
	unsigned char time_out = 200;
	
	int socket_fd = -1;
	
	if(!net_device_info.net_work)
		return result;
	
	if(strlen(device_id) < 6 || strlen(api_key) != 28)
		return result;
	
	if(lon == NULL || lat == NULL)
		return result;
	
	send_buf = (char *)os_malloc(256);
	data_ptr = (char *)os_malloc(320);
	if(send_buf == NULL || data_ptr == NULL)
	{
		if(send_buf)		os_free(send_buf);
		if(data_ptr)		os_free(data_ptr);
		
		return result;
	}
	
	memset(send_buf, 0, 256);
	memset(data_ptr, 0, 320);
	
	socket_fd = NET_DEVICE_GetSocket();
	
	if(NET_DEVICE_Connect(socket_fd, "183.230.40.33", 80) == 0)
	{
		snprintf(send_buf, 256, "GET /devices/%s/lbs/latestWifiLocation HTTP/1.1\r\n"
									"api-key:%s\r\n"
									"Host:api.heclouds.com\r\n\r\n",
									device_id, api_key);
		
		NET_DEVICE_SendData(socket_fd, (unsigned char *)send_buf, strlen(send_buf));
		
		while(--time_out)
		{
			if(NET_DEVICE_GetData(socket_fd, data_ptr, 320, MSG_DONTWAIT) > 0)
				break;
			
			vTaskDelay(1);
		}
		
		//{"errno":0,"data":{"at":"2018-12-07 15:28:05.348","accuracy":696,"lon":106.xxx,"lat":29.xxx},"error":"succ"}
		
		if(data_ptr != NULL)
		{
			data_ptr = strstr(data_ptr, "\"lon\":");
			if(sscanf(data_ptr, "\"lon\":%[^,],\"lat\":%[^}]},\"error\":\"succ\"}", lon, lat) == 2)
			{
				//UartPrintf("lon:%s, lat: %s\r\n", lon, lat);
				
				result = SEND_TYPE_OK;
			}
		}
		else
		{
			data_ptr = strstr(data_ptr, "\"error\":");
			if(data_ptr)
				;//UartPrintf("错误信息:%s\r\n", data_ptr);
		}
	}
	
	NET_DEVICE_Close(&socket_fd);
	
	os_free(send_buf);
	os_free(data_ptr);
	
	return result;

}

//==========================================================
//	函数名称：	OneNET_ConnectIP
//
//	函数功能：	连接IP
//
//	入口参数：	ip：IP地址缓存指针
//				port：端口缓存指针
//
//	返回参数：	返回初始化结果
//
//	说明：		0-成功		1-失败
//==========================================================
_Bool OneNET_ConnectIP(const unsigned char *addr, short port)
{

	_Bool result = 1;
	
	if(addr == NULL || port <= 0)
		return result;
	
	if(!net_device_info.net_work)									//网络模组具备网络连接能力
		return result;
	
	if(onenet_info.connect_ip)										//已经连接了IP
		return !result;
	
	if(net_device_info.socket_fd < 0)
		net_device_info.socket_fd = NET_DEVICE_GetSocket();
	
	if(NET_DEVICE_Connect(net_device_info.socket_fd, addr, port) == 0)
	{
		result = 0;
		onenet_info.connect_ip = 1;
	}
	else
	{
		vTaskDelay(50);
		
		if(++onenet_info.err_count >= 5)
		{
			onenet_info.err_count = 0;
			NET_DEVICE_Close(&net_device_info.socket_fd);
//			printf("请检查IP地址和PORT是否正确\r\n");
		}
	}
	
	return result;

}

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
void OneNET_DevLink(const char* devid, const char *proid, const char* auth_info)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};					//协议包
	
	unsigned char time_out = 200;
	
	if(!onenet_info.connect_ip)											//还未连接IP
		return;
	
	UartPrintf("+Event:OneNET Link:"
			"PROID: %s,AUIF: %s,DEVID:%s\r\n"
			, proid, auth_info, devid);

	if(MQTT_PacketConnect(proid, auth_info, devid, 256, 0, MQTT_QOS_LEVEL0, NULL, NULL, 0, &mqttPacket) == 0)
	{
		if(NET_DEVICE_SendData(net_device_info.socket_fd, mqttPacket._data, mqttPacket._len) <= 0)//上传平台
		{
			onenet_info.reconnect = 1;
			net_fault_info.net_fault_level = NET_FAULT_LEVEL_1;
		}
		
		while(!onenet_info.net_work && --time_out)
			vTaskDelay(1);
		
		MQTT_DeleteBuffer(&mqttPacket);								//删包
	}
	else
//		printf("WARN:	EDP_PacketConnect Failed\r\n");
	
	if(onenet_info.net_work)										//如果接入成功
	{
		onenet_info.err_count = 0;
		onenet_info.reinit_count = 0;
	}
	else
	{
		if(++onenet_info.err_count >= 5)							//如果超过设定次数后，还未接入平台
		{
			onenet_info.net_work = 0;
			onenet_info.err_count = 0;
			net_fault_info.net_fault_level = net_fault_info.net_fault_level_r =
			NET_FAULT_LEVEL_3;										//错误等级3
		}
	}
	
}

//==========================================================
//	函数名称：	OneNET_DisConnect
//
//	函数功能：	与平台断开连接
//
//	入口参数：	无
//
//	返回参数：	0-成功		1-失败
//
//	说明：		
//==========================================================
_Bool OneNET_DisConnect(void)
{

	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};							//协议包

	if(!onenet_info.net_work)
		return 1;
	
	if(MQTT_PacketDisConnect(&mqttPacket) == 0)
	{
		NET_DEVICE_SendData(net_device_info.socket_fd, mqttPacket._data, mqttPacket._len);//上传平台
		
		MQTT_DeleteBuffer(&mqttPacket);											//删包
	}
	
	return 0;

}

//==========================================================
//	函数名称：	OneNET_SendData
//
//	函数功能：	上传数据到平台
//
//	入口参数：	type：发送数据的格式
//				devid：设备ID
//				apikey：设备apikey
//				streamArray：数据流
//				streamArrayNum：数据流个数
//
//	返回参数：	SEND_TYPE_OK-发送成功	SEND_TYPE_DATA-需要重送
//
//	说明：		
//==========================================================
unsigned char OneNET_SendData(FORMAT_TYPE type, char *devid, char *apikey, DATA_STREAM *streamArray, unsigned short streamArrayCnt)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};											//协议包
	
	_Bool status = SEND_TYPE_OK;
	short body_len = 0;
	_Bool err_flag = 0;
	
	if(!onenet_info.net_work)
		return SEND_TYPE_DATA;
	
	if(type < 1 && type > 5)
		return SEND_TYPE_DATA;
	
//	printf("Tips:	OneNET_SendData-EDP_TYPE%d\r\n", type);
	
	body_len = DSTREAM_GetDataStream_Body_Measure(type, streamArray, streamArrayCnt, 0);		//获取当前需要发送的数据流的总长度
	if(body_len > 0)
	{
		if(MQTT_PacketSaveData(devid, body_len, NULL, (uint8)type, &mqttPacket) == 0)
		{
			body_len = DSTREAM_GetDataStream_Body(type, streamArray, streamArrayCnt, mqttPacket._data, mqttPacket._size, mqttPacket._len);
			
			if(body_len > 0)
			{
				mqttPacket._len += body_len;
				
				UartPrintf("+Event:Send %d Bytes\r\n", mqttPacket._len);
				
				if(NET_DEVICE_SendData(net_device_info.socket_fd, mqttPacket._data, mqttPacket._len) <= 0)
					err_flag = 1;
			}
			else
				;//printf("WARN:	DSTREAM_GetDataStream_Body Failed\r\n");
				
			MQTT_DeleteBuffer(&mqttPacket);														//删包
		}
		else
			;//printf("WARN:	MQTT_NewBuffer Failed\r\n");
	}
	else if(body_len < 0)
		status = SEND_TYPE_OK;
	else
		status = SEND_TYPE_DATA;
	
	if(err_flag)
	{
		status = SEND_TYPE_OK;
		onenet_info.reconnect = 1;
		net_fault_info.net_fault_level = NET_FAULT_LEVEL_1;
	}
	else
	{
		onenet_info.send_count++;
		net_fault_info.net_fault_level_r = NET_FAULT_LEVEL_0;									//发送之后清除标记
	}
	
	return status;
	
}

//==========================================================
//	函数名称：	OneNet_HeartBeat
//
//	函数功能：	发送心跳请求
//
//	入口参数：	无
//
//	返回参数：	SEND_TYPE_OK-发送成功	SEND_TYPE_HEART-需要重送
//
//	说明：		
//==========================================================
unsigned char OneNET_SendData_Heart(void)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};				//协议包
	
	if(!onenet_info.net_work)										//如果网络为连接
		return SEND_TYPE_HEART;
	
	if(MQTT_PacketPing(&mqttPacket))
		return SEND_TYPE_HEART;
	
	onenet_info.heart_beat = 0;
	
	if(NET_DEVICE_SendData(net_device_info.socket_fd, mqttPacket._data, mqttPacket._len) <= 0)
	{
		onenet_info.reconnect = 1;
		net_fault_info.net_fault_level = NET_FAULT_LEVEL_1;
	}
	
	MQTT_DeleteBuffer(&mqttPacket);									//删包
	
	return SEND_TYPE_OK;
	
}

//==========================================================
//	函数名称：	OneNet_HeartBeat_Check
//
//	函数功能：	发送心跳后的心跳检测
//
//	入口参数：	无
//
//	返回参数：	0-成功	1-等待
//
//	说明：		基于调用时基，runCount每隔此函数调用一次的时间自增
//				达到设定上限检测心跳标志位是否就绪
//				上限时间可以不用太精确
//==========================================================
_Bool OneNET_Check_Heart(void)
{
	
	static unsigned char runCount = 0;
	
	if(!onenet_info.net_work)
		return 1;

	if(onenet_info.heart_beat == 1)
	{
		runCount = 0;
		onenet_info.err_count = 0;
		
		return 0;
	}
	
	if(++runCount >= 100)
	{
		runCount = 0;
		
//		UartPrintf("+Event:HeartBeat TimeOut: %d\r\n", onenet_info.err_count);
		onenet_info.send_data |= SEND_TYPE_HEART;								//发送心跳请求
		
		if(++onenet_info.err_count >= 3)
		{
			unsigned char errType = 0;
			
			onenet_info.err_count = 0;
			
			errType = NET_DEVICE_Check();												//网络设备状态检查
			if(errType == NET_DEVICE_CONNECTED || errType == NET_DEVICE_CLOSED || errType == NET_DEVICE_GOT_IP)
			{
				onenet_info.reconnect = 1;
				net_fault_info.net_fault_level = net_fault_info.net_fault_level_r = NET_FAULT_LEVEL_1;
			}
			else if(errType == NET_DEVICE_NO_DEVICE || errType == NET_DEVICE_INITIAL)
			{
				onenet_info.reconnect = 1;
				net_fault_info.net_fault_level = net_fault_info.net_fault_level_r = NET_FAULT_LEVEL_3;
			}
			else
				net_fault_info.net_fault_level = net_fault_info.net_fault_level_r = NET_FAULT_LEVEL_0;
		}
	}
	
	return 1;

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

	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};							//协议包

	if(!onenet_info.net_work)
		return SEND_TYPE_PUBLISH;
	
//	printf("Publish Topic: %s, Msg: %s\r\n", topic, msg);
	UartPrintf("+Event:Publish\r\n", topic, msg);
	
	if(MQTT_PacketPublish(MQTT_PUBLISH_ID, topic, msg, strlen(msg), MQTT_QOS_LEVEL2, 0, 1, &mqttPacket) == 0)
	{
		if(NET_DEVICE_SendData(net_device_info.socket_fd, mqttPacket._data, mqttPacket._len) <= 0)//上传平台
		{
			onenet_info.reconnect = 1;
			net_fault_info.net_fault_level = NET_FAULT_LEVEL_1;
		}
		
		MQTT_DeleteBuffer(&mqttPacket);											//删包
	}
	
	return SEND_TYPE_OK;

}

//==========================================================
//	函数名称：	OneNET_Subscribe
//
//	函数功能：	订阅
//
//	入口参数：	topics：订阅的topic
//
//	返回参数：	SEND_TYPE_OK-成功	SEND_TYPE_SUBSCRIBE-需要重发
//
//	说明：		
//==========================================================
unsigned char OneNET_Subscribe(const signed char *topics)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};							//协议包

	if(!onenet_info.net_work)
		return SEND_TYPE_SUBSCRIBE;
	
	UartPrintf("+Event:Subscribe Topic: %s\r\n", topics);
	
	if(MQTT_PacketSubscribe(MQTT_SUBSCRIBE_ID, MQTT_QOS_LEVEL2, topics, &mqttPacket) == 0)
	{
		if(NET_DEVICE_SendData(net_device_info.socket_fd, mqttPacket._data, mqttPacket._len) <= 0)//上传平台
		{
			onenet_info.reconnect = 1;
			net_fault_info.net_fault_level = NET_FAULT_LEVEL_1;
		}
		
		MQTT_DeleteBuffer(&mqttPacket);											//删包
	}
	
	return SEND_TYPE_OK;

}

//==========================================================
//	函数名称：	OneNET_UnSubscribe
//
//	函数功能：	取消订阅
//
//	入口参数：	topics：订阅的topic
//
//	返回参数：	SEND_TYPE_OK-发送成功	SEND_TYPE_UNSUBSCRIBE-需要重发
//
//	说明：		
//==========================================================
unsigned char OneNET_UnSubscribe(const signed char *topics)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};							//协议包

	if(!onenet_info.net_work)
		return SEND_TYPE_UNSUBSCRIBE;
	
	UartPrintf("+Event:UnSubscribe Topic: %s\r\n", topics);
	
	if(MQTT_PacketUnSubscribe(MQTT_UNSUBSCRIBE_ID, topics, &mqttPacket) == 0)
	{
		if(NET_DEVICE_SendData(net_device_info.socket_fd, mqttPacket._data, mqttPacket._len) <= 0)//上传平台
		{
			onenet_info.reconnect = 1;
			net_fault_info.net_fault_level = NET_FAULT_LEVEL_1;
		}
		
		MQTT_DeleteBuffer(&mqttPacket);											//删包
	}
	
	return SEND_TYPE_OK;

}

static unsigned short num_len(unsigned short num)
{

	if(num < 10)
		return 1;
	else if(num < 100 && num >= 10)
		return 2;
	else if(num < 1000 && num >= 100)
		return 3;
	else if(num < 10000 && num >= 1000)
		return 4;
	else if(num <= 65535 && num >= 10000)
		return 5;

}

//==========================================================
//	函数名称：	OneNET_RevPro
//
//	函数功能：	平台返回数据检测
//
//	入口参数：	dataPtr：平台返回的数据
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void OneNET_RevPro(unsigned char *cmd)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};								//协议包
	
	signed char *req_payload = NULL;
	signed char *cmdid_topic = NULL;
	
	unsigned short topic_len = 0;
	unsigned short req_len = 0;
	
	unsigned char qos = 0;
	static unsigned short pkt_id = 0;
	
	switch(MQTT_UnPacketRecv(cmd))
	{
		case MQTT_PKT_CONNACK:
		
			switch(MQTT_UnPacketConnectAck(cmd))
			{
				case 0:
					UartPrintf("+Event:Connect:0\r\n");
					onenet_info.net_work = 1;
				break;
				
				case 1:UartPrintf("+Event:Connect:1\r\n");break;				//协议错误
				case 2:UartPrintf("+Event:Connect:2\r\n");break;				//非法的clientid
				case 3:UartPrintf("+Event:Connect:3\r\n");break;				//服务器失败
				case 4:UartPrintf("+Event:Connect:4\r\n");break;				//用户名或密码错误
				case 5:UartPrintf("+Event:Connect:5\r\n");break;				//非法链接(比如token非法)
				
				default:UartPrintf("+Event:Connect:6\r\n");break;				//未知错误
			}
		
		break;
		
		case MQTT_PKT_PINGRESP:
		
//			printf("Tips:	HeartBeat OK\r\n");
			onenet_info.heart_beat = 1;
		
		break;
		
		case MQTT_PKT_CMD:															//命令下发
			
			if(MQTT_UnPacketCmd(cmd, &cmdid_topic, &req_payload, &req_len) == 0)	//解出topic和消息体
			{
				unsigned short i = 0;
				unsigned short len = 0;
				unsigned char *buf = NULL;
				
				len = 6 + num_len(req_len) + req_len;
				
				buf = (unsigned char *)os_malloc(len);
				if(buf != NULL)
				{
					memset(buf, 0, len);
					snprintf(buf, len, "+CMD,%d,", req_len);
					while(buf[i++] != 0);
					memcpy(buf + i - 1, req_payload, req_len);
					
					uart_send_hex(buf, len);
				}
				
//				printf("cmdid: %s, req: %s, req_len: %d\r\n", cmdid_topic, req_payload, req_len);
				
				if(MQTT_PacketCmdResp(cmdid_topic, req_payload, &mqttPacket) == 0)	//命令回复组包
				{
//					printf("Tips:	Send CmdResp\r\n");
					
					if(NET_DEVICE_SendData(net_device_info.socket_fd, mqttPacket._data, mqttPacket._len) <= 0)//上传平台
					{
						onenet_info.reconnect = 1;
						net_fault_info.net_fault_level = NET_FAULT_LEVEL_1;
					}
					MQTT_DeleteBuffer(&mqttPacket);									//删包
				}
				
				MQTT_FreeBuffer(cmdid_topic);
				MQTT_FreeBuffer(req_payload);
				onenet_info.send_data |= SEND_TYPE_DATA;
			}
		
		break;
			
		case MQTT_PKT_PUBLISH:														//接收的Publish消息
		
			if(MQTT_UnPacketPublish(cmd, &cmdid_topic, &topic_len, &req_payload, &req_len, &qos, &pkt_id) == 0)
			{
				unsigned short i = 0;
				unsigned short len = 0;
				unsigned char *buf = NULL;
				
				len = 8 + num_len(topic_len) + topic_len + num_len(req_len) + req_len;
				
				buf = (unsigned char *)os_malloc(len);
				if(buf != NULL)
				{
					memset(buf, 0, len);
					snprintf(buf, len, "+PUB,%d,%s,%d,", topic_len, cmdid_topic, req_len);
					while(buf[i++] != 0);
					memcpy(buf + i - 1, req_payload, req_len);
					
					uart_send_hex(buf, len);
					uart_send_hex("\r\n", 2);
				}
				
//				printf("topic: %s, topic_len: %d, payload: %s, payload_len: %d\r\n",
//																	cmdid_topic, topic_len, req_payload, req_len);
				
				switch(qos)
				{
					case 1:															//收到publish的qos为1，设备需要回复Ack
					
						if(MQTT_PacketPublishAck(pkt_id, &mqttPacket) == 0)
						{
//							printf("Tips:	Send PublishAck\r\n");
							if(NET_DEVICE_SendData(net_device_info.socket_fd, mqttPacket._data, mqttPacket._len) <= 0)
							{
								onenet_info.reconnect = 1;
								net_fault_info.net_fault_level = NET_FAULT_LEVEL_1;
							}
							MQTT_DeleteBuffer(&mqttPacket);
						}
					
					break;
					
					case 2:															//收到publish的qos为2，设备先回复Rec
																					//平台回复Rel，设备再回复Comp
						if(MQTT_PacketPublishRec(pkt_id, &mqttPacket) == 0)
						{
//							printf("Tips:	Send PublishRec\r\n");
							if(NET_DEVICE_SendData(net_device_info.socket_fd, mqttPacket._data, mqttPacket._len) <= 0)
							{
								onenet_info.reconnect = 1;
								net_fault_info.net_fault_level = NET_FAULT_LEVEL_1;
							}
							MQTT_DeleteBuffer(&mqttPacket);
						}
					
					break;
					
					default:
						break;
				}
				
				MQTT_FreeBuffer(cmdid_topic);
				MQTT_FreeBuffer(req_payload);
				onenet_info.send_data |= SEND_TYPE_DATA;
			}
		
		break;
			
		case MQTT_PKT_PUBACK:														//发送Publish消息，平台回复的Ack
		
			if(MQTT_UnPacketPublishAck(cmd) == 0)
			{
				onenet_info.send_count = 0;
				onenet_info.send_ok = 1;
				UartPrintf("+Event:Send OK\r\n");
			}
			
		break;
			
		case MQTT_PKT_PUBREC:														//发送Publish消息，平台回复的Rec，设备需回复Rel消息
		
			if(MQTT_UnPacketPublishRec(cmd) == 0)
			{
//				printf("Tips:	Rev PublishRec\r\n");
				if(MQTT_PacketPublishRel(MQTT_PUBLISH_ID, &mqttPacket) == 0)
				{
//					printf("Tips:	Send PublishRel\r\n");
					if(NET_DEVICE_SendData(net_device_info.socket_fd, mqttPacket._data, mqttPacket._len) <= 0)
					{
						onenet_info.reconnect = 1;
						net_fault_info.net_fault_level = NET_FAULT_LEVEL_1;
					}
					MQTT_DeleteBuffer(&mqttPacket);
				}
			}
		
		break;
			
		case MQTT_PKT_PUBREL:														//收到Publish消息，设备回复Rec后，平台回复的Rel，设备需再回复Comp
			
			if(MQTT_UnPacketPublishRel(cmd, pkt_id) == 0)
			{
//				printf("Tips:	Rev PublishRel\r\n");
				if(MQTT_PacketPublishComp(MQTT_PUBLISH_ID, &mqttPacket) == 0)
				{
//					printf("Tips:	Send PublishComp\r\n");
					if(NET_DEVICE_SendData(net_device_info.socket_fd, mqttPacket._data, mqttPacket._len) <= 0)
					{
						onenet_info.reconnect = 1;
						net_fault_info.net_fault_level = NET_FAULT_LEVEL_1;
					}
					MQTT_DeleteBuffer(&mqttPacket);
				}
			}
		
		break;
		
		case MQTT_PKT_PUBCOMP:														//发送Publish消息，平台返回Rec，设备回复Rel，平台再返回的Comp
		
			if(MQTT_UnPacketPublishComp(cmd) == 0)
			{
//				printf("Tips:	Rev PublishComp\r\n");
			}
		
		break;
			
		case MQTT_PKT_SUBACK:														//发送Subscribe消息的Ack
		
			if(MQTT_UnPacketSubscribe(cmd) == 0)
				UartPrintf("+Event:Subscribe OK\r\n");
			else
				UartPrintf("+Event:Subscribe Err\r\n");
		
		break;
			
		case MQTT_PKT_UNSUBACK:														//发送UnSubscribe消息的Ack
		
			if(MQTT_UnPacketUnSubscribe(cmd) == 0)
				UartPrintf("+Event:UnSubscribe OK\r\n");
			else
				UartPrintf("+Event:UnSubscribe Err\r\n");
		
		break;
		
		default:
			
		break;
	}

}
