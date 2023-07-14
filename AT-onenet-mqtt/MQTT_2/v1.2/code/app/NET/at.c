/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	at.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2018-07-24
	*
	*	版本： 		V1.0
	*
	*	说明： 		AT指令解析
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//系统
#include "esp_common.h"
#include "uart.h"

//AirKiss
#include "smartconfig.h"

//
#include "onenet.h"
#include "fault.h"

//
#include "net_device.h"

//
#include "at.h"

//C库
#include <string.h>
#include <time.h>


AT_INFO at_info;


#define AT_CMD_LEN		16			//缓存AT指令的长度

#define AT_PARA_LEN		64			//缓存AT参数的长度


static char at_cmd[AT_CMD_LEN];		//只缓存AT命令
static char at_para[AT_PARA_LEN];	//只缓存AT参数


extern char dp_name[32];
extern char dp_value[64];
extern DATA_STREAM data_stream[];

extern char topics[16];

extern char publish_tp[16];
extern char publish_mg[32];


//WIFI功能
static void cmd_cb_cwjap(char *cmd, char *para);
static void cmd_cb_cwqap(char *cmd, char *para);
static void cmd_cb_airkiss(char *cmd, char *para);
static void cmd_cb_cipstatus(char *cmd, char *para);

//OneNET功能
static void cmd_cb_iotcfg(char *cmd, char *para);
static void cmd_cb_iotsend(char *cmd, char *para);
static void cmd_cb_iotsub(char *cmd, char *para);
static void cmd_cb_iotunsub(char *cmd, char *para);
static void cmd_cb_iotpub(char *cmd, char *para);
static void cmd_cb_iotstatus(char *cmd, char *para);
static void cmd_cb_iotgps(char *cmd, char *para);
static void cmd_cb_iotnetworkdelay(char *cmd, char *para);

//其他网络功能
static void cmd_cb_ntp(char *cmd, char *para);


AT_LIST_INFO at_list_info[] =
{

	//WIFI功能
	{"CWJAP", cmd_cb_cwjap},
	{"CWQAP", cmd_cb_cwqap},
	{"AIRKISS", cmd_cb_airkiss},
	{"CIPSTATUS", cmd_cb_cipstatus},
	
	//OneNET功能
	{"IOTCFG", cmd_cb_iotcfg},
	{"IOTSEND", cmd_cb_iotsend},
	{"IOTSUB", cmd_cb_iotsub},
	{"IOTUNSUB", cmd_cb_iotunsub},
	{"IOTPUB", cmd_cb_iotpub},
	{"IOTSTATUS", cmd_cb_iotstatus},
	{"IOTGPS", cmd_cb_iotgps},
	{"IOTNWD", cmd_cb_iotnetworkdelay},
	
	//其他网络功能
	{"NTP", cmd_cb_ntp},

};
unsigned char at_list_num = sizeof(at_list_info) / sizeof(at_list_info[0]);


/*
************************************************************
*	函数名称：	AT_CheckHead
*
*	函数功能：	检查是否有AT头
*
*	入口参数：	data：收到的数据
*
*	返回参数：	1-有	0-无
*
*	说明：		
************************************************************
*/
static _Bool AT_CheckHead(unsigned char *data)
{

	if(*data == 'A' && *(data + 1) == 'T' && *(data + 2) == '+')
		return 1;
	else
		return 0;

}

/*
************************************************************
*	函数名称：	AT_CheckEnd
*
*	函数功能：	检查是否有AT尾
*
*	入口参数：	data：收到的数据
*
*	返回参数：	1-有	0-无
*
*	说明：		
************************************************************
*/
static _Bool AT_CheckEnd(unsigned char *data)
{

	unsigned char len = strlen(data);
	
	if(*(data + len - 2) == '\r' && *(data + len - 1) == '\n')
		return 1;
	else
		return 0;

}

/*
************************************************************
*	函数名称：	AT_GetCmd
*
*	函数功能：	检查并复制命令
*
*	入口参数：	data：收到的数据
*				cmd：命令缓存
*
*	返回参数：	1-找到cmd	0-未找到
*
*	说明：		
************************************************************
*/
static _Bool AT_GetCmd(unsigned char *data, char *cmd)
{

	char *data_ptr = strchr(data, '+');
	
	if(data_ptr)
	{
		char *data_ptr_t = NULL;
		unsigned char cmd_len = 0;
		
		++data_ptr;
		
		data_ptr_t = data_ptr;
		
		while((*data_ptr_t != '=') && (*data_ptr_t != '\r'))
		{
			data_ptr_t++;
			cmd_len++;
		}
		
		if(cmd_len >= AT_CMD_LEN)
			return 0;
		
		while((*data_ptr != '=') && (*data_ptr != '\r'))
			*cmd++ = *data_ptr++;
		
		*cmd++ = 0;
		
		return 1;
	}
	else
		return 0;

}

/*
************************************************************
*	函数名称：	AT_GetPara
*
*	函数功能：	检查并复制参数
*
*	入口参数：	data：收到的数据
*				para：参数缓存
*
*	返回参数：	1-找到para	0-未找到
*
*	说明：		
************************************************************
*/
static _Bool AT_GetPara(unsigned char *data, char *para)
{

	char *data_ptr = strchr(data, '=');
	
	if(data_ptr)
	{
		char *data_ptr_t = NULL;
		unsigned char para_len = 0;
		
		data_ptr_t = data_ptr;
		
		data_ptr_t++;
		while(*data_ptr_t++ != '\r')
			para_len++;
		
		if(para_len >= AT_PARA_LEN)
			return 0;
		
		while(*data_ptr++ != '\r')
			*para++ = *data_ptr;
		
		*para++ = 0;
		
		return 1;
	}
	else
		return 0;

}

/*
************************************************************
*	函数名称：	AT_GetPara_Signal
*
*	函数功能：	解析para里边的单个para值
*
*	入口参数：	para：整条命令
*				para_t：单个para缓存
*				num：查找第几个para(从0起)
*
*	返回参数：	1-找到para	0-未找到
*
*	说明：		
************************************************************
*/
static _Bool AT_GetPara_Signal(char *para, char *para_t, unsigned char num)
{
	
	unsigned char num_t = 0;
	_Bool result = 0;

	if(para)
	{
		while(num_t < num)
		{
			if(*para == ',')
				num_t++;
			
			if(*para == '\r')
				return result;
			
			para++;
		}
		
		while((*para != ',') && (*para != '\r'))
			*para_t++ = *para++;
		
		*para_t++ = 0;
		
		result = 1;
	}
	
	return result;

}

static void AT_ClearBf(void)
{

	memset(at_info.recv_buf, 0, sizeof(at_info.recv_buf));
	at_info.recv_len = 0;
	at_info.busy = 0;

}

/*
************************************************************
*	函数名称：	AT_Recv_Pro
*
*	函数功能：	AT指令解析
*
*	入口参数：	无
*
*	返回参数：	1-有命令处理	0-无命令处理
*
*	说明：		
************************************************************
*/
_Bool AT_Recv_Pro(void)
{
	
	_Bool result = 0;

	if(at_info.recv_flag)
	{
		at_info.recv_flag = 0;
		
		if(AT_CheckHead(at_info.recv_buf))
		{
			if(AT_CheckEnd(at_info.recv_buf))
			{
				at_info.busy = 1;
				
				if(AT_GetCmd(at_info.recv_buf, at_cmd))
				{
					unsigned char i = 0;
					
					for(; i < at_list_num; i++)
					{
						if(strcmp(at_cmd, at_list_info[i].cmd) == 0)
						{
							if(AT_GetPara(at_info.recv_buf, at_para))
								at_list_info[i].cmd_cb(at_cmd, at_para);
							else
								at_list_info[i].cmd_cb(at_cmd, NULL);
							
							result = 1;
							
							break;
						}
					}
				}
				
				AT_ClearBf();
			}
		}
		else
		{
			UartPrintf("+Event:ERROR Cmd\r\n");
			
			AT_ClearBf();
		}
	}
	
	return result;

}







static void cmd_cb_cwjap(char *cmd, char *para)
{
	
	char para_t[32];

	struct station_config sta_cfg = {"", "", 0, ""};
	
	if(AT_GetPara_Signal(para, para_t, 0) == 1)
	{
		strncpy(sta_cfg.ssid, para_t, strlen(para_t));
		
		if(AT_GetPara_Signal(para, para_t, 1) == 1)
		{
			strncpy(sta_cfg.password, para_t, strlen(para_t) + 1);
			
			UartPrintf("+Event:SSID:%s, PSWD:%s\r\n", sta_cfg.ssid, sta_cfg.password);
			
			wifi_station_set_config(&sta_cfg);
			wifi_station_disconnect();
			wifi_station_connect();
		}
	}

}

static void cmd_cb_cwqap(char *cmd, char *para)
{

	NET_DEVICE_Close(&net_device_info.socket_fd);
	
	wifi_station_disconnect();

}

static void cmd_cb_airkiss(char *cmd, char *para)
{

	wifi_station_disconnect();							//先停止WIFI自动连接

	smartconfig_stop();									//停止配网，以防止启动配网时提示"busy"
	Smartconfig_AirKiss();								//启动配网

}

static void cmd_cb_cipstatus(char *cmd, char *para)
{
	
	switch(wifi_station_get_connect_status())
	{
		case STATION_IDLE:
			UartPrintf("+STATUS:0\r\n");
		break;
		
		case STATION_CONNECTING:
			UartPrintf("+STATUS:1\r\n");
		break;
		
		case STATION_WRONG_PASSWORD:
			UartPrintf("+STATUS:2\r\n");
		break;
		
		case STATION_NO_AP_FOUND:
			UartPrintf("+STATUS:3\r\n");
		break;
		
		case STATION_CONNECT_FAIL:
			UartPrintf("+STATUS:4\r\n");
		break;
		
		case STATION_GOT_IP:
			UartPrintf("+STATUS:5\r\n");
		break;
	}

}

static void cmd_cb_iotcfg(char *cmd, char *para)
{

	char para_t[32];
	
	if(AT_GetPara_Signal(para, para_t, 0) == 1)
	{
		strncpy(onenet_info.dev_id, para_t, strlen(para_t) + 1);
		
		if(AT_GetPara_Signal(para, para_t, 1) == 1)
		{
			strncpy(onenet_info.pro_id, para_t, strlen(para_t) + 1);
			
			if(AT_GetPara_Signal(para, para_t, 2) == 1)
			{
				strncpy(onenet_info.auth_info, para_t, strlen(para_t) + 1);
				
//				printf("+Event:devid:%s, proid:%s, auth_info:%s\r\n",
//							onenet_info.dev_id, onenet_info.pro_id, onenet_info.auth_info);
				
				onenet_info.reconnect = 1;
				
				net_fault_info.net_fault_level = NET_FAULT_LEVEL_1;
			}
		}
	}

}

static void cmd_cb_iotsend(char *cmd, char *para)
{
	
	char type[4];
	
	if(onenet_info.net_work == 0)
	{
		UartPrintf("+Event:IOT ERROR\r\n");
		return;
	}
	
	if((onenet_info.send_data & ~SEND_TYPE_OK) != 0)
	{
		UartPrintf("+Event:busy\r\n");
		return;
	}
	
	if(AT_GetPara_Signal(para, type, 0) == 1)
	{
		if(AT_GetPara_Signal(para, dp_name, 1) == 1)
		{
			if(type[0] == '2')
			{
				if(AT_GetPara_Signal(para, gps_info.lon, 2) == 1)
				{
					if(AT_GetPara_Signal(para, gps_info.lat, 3) == 1)
					{
						data_stream[2].flag = 1;
						onenet_info.send_data |= SEND_TYPE_DATA;
					}
				}
			}
			else
			{
				if(AT_GetPara_Signal(para, dp_value, 2) == 1)
				{
					switch(type[0])
					{
						case '0':		//数值类型
						
							data_stream[0].flag = 1;
							onenet_info.send_data |= SEND_TYPE_DATA;
						
						break;
						
						case '1':		//string类型
						
							data_stream[1].flag = 1;
							onenet_info.send_data |= SEND_TYPE_DATA;
						
						break;
					}
				}
			}
		}
	}

}

static void cmd_cb_iotsub(char *cmd, char *para)
{

	if(onenet_info.net_work == 0)
	{
		UartPrintf("+Event:IOT ERROR\r\n");
		return;
	}
	
	if((onenet_info.send_data & ~SEND_TYPE_OK) != 0)
	{
		UartPrintf("+Event:busy\r\n");
		return;
	}
	
	if(AT_GetPara_Signal(para, topics, 0) == 1)
	{
		onenet_info.send_data |= SEND_TYPE_SUBSCRIBE;
	}

}

static void cmd_cb_iotunsub(char *cmd, char *para)
{

	if(onenet_info.net_work == 0)
	{
		UartPrintf("+Event:IOT ERROR\r\n");
		return;
	}
	
	if((onenet_info.send_data & ~SEND_TYPE_OK) != 0)
	{
		UartPrintf("+Event:busy\r\n");
		return;
	}
	
	if(AT_GetPara_Signal(para, topics, 0) == 1)
	{
		onenet_info.send_data |= SEND_TYPE_UNSUBSCRIBE;
	}

}

static void cmd_cb_iotpub(char *cmd, char *para)
{

	if(onenet_info.net_work == 0)
	{
		UartPrintf("+Event:IOT ERROR\r\n");
		return;
	}
	
	if((onenet_info.send_data & ~SEND_TYPE_OK) != 0)
	{
		UartPrintf("+Event:busy\r\n");
		return;
	}
	
	if(AT_GetPara_Signal(para, publish_tp, 0) == 1)
	{
		if(AT_GetPara_Signal(para, publish_mg, 1) == 1)
		{
			onenet_info.send_data |= SEND_TYPE_PUBLISH;
		}
	}

}

static void cmd_cb_iotstatus(char *cmd, char *para)
{

	if(net_device_info.net_work == 0)	//不具备网络连接能力
	{
		UartPrintf("+IOT:status:2\r\n");
		return;
	}
		
	if(onenet_info.net_work == 0)		//IOT---掉线
		UartPrintf("+IOT:status:1\r\n");
	else								//IOT---已接入
		UartPrintf("+IOT:status:0\r\n");

}

static void cmd_cb_iotgps(char *cmd, char *para)
{

	if(onenet_info.net_work == 0)
	{
		UartPrintf("+Event:IOT ERROR\r\n");
		return;
	}
	
	if((onenet_info.send_data & ~SEND_TYPE_OK) != 0)
	{
		UartPrintf("+Event:busy\r\n");
		return;
	}
	
	if(AT_GetPara_Signal(para, onenet_info.api_key, 0) == 1)
	{
		if(strlen(onenet_info.api_key) < 28)
			UartPrintf("+Event:apikey err\r\n");
		else
		{
			strcpy(dp_name, "$OneNET_LBS_WIFI");
			strcpy(dp_value, lbs_wifi_info.mac);
			snprintf(dp_value, sizeof(dp_value), "{\"macs\":\"%s\"}", lbs_wifi_info.mac);
			
			data_stream[1].flag = 1;
			onenet_info.send_data |= SEND_TYPE_DATA;
			onenet_info.send_data |= SEND_TYPE_GPS;
			
			while(gps_info.flag == 0)
				vTaskDelay(3);
			
			gps_info.flag = 0;
			
			UartPrintf("+Event:lon:%s, lat:%s\r\n", gps_info.lon, gps_info.lat);
			
			onenet_info.reconnect = 1;
			
			net_fault_info.net_fault_level = NET_FAULT_LEVEL_1;
		}
	}
	else
		UartPrintf("+Event:para err\r\n");

}

static void cmd_cb_iotnetworkdelay(char *cmd, char *para)
{
	
	if(net_device_info.net_work == 0)	//不具备网络连接能力
	{
		UartPrintf("+Event:IOT ERROR\r\n");
	}
	else
		UartPrintf("+Event:network delay(ms): %d\r\n", net_device_info.network_delay_time);

}

static void cmd_cb_ntp(char *cmd, char *para)
{

	unsigned int net_time = 0;
	
	struct tm *time;
	
	char date_time[24];
	
	if(net_device_info.net_work == 0)	//不具备网络连接能力
	{
		UartPrintf("+Event:ERROR\r\n");
		return;
	}
	
	net_time = NET_DEVICE_GetTime();
	
	time = localtime((const time_t *)&net_time);
	
	memset(date_time, 0, sizeof(date_time));
	snprintf(date_time, sizeof(date_time), "%d-%d-%d %d:%d:%d",
					time->tm_year + 1900, time->tm_mon + 1, time->tm_mday,
					time->tm_hour, time->tm_min, time->tm_sec);

	UartPrintf("+Event:%s,%d\r\n", date_time, net_time);

}
