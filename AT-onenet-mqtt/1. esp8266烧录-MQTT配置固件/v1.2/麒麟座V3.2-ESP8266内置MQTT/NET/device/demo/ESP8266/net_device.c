/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	net_device.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2019-12-20
	*
	*	版本： 		V1.6
	*
	*	说明： 		网络设备应用层
	*
	*	修改记录：	V1.1：1.平台IP和PORT通过参数传入的方式确定，解决了不同协议网络设备驱动不通用的问题。
	*					  2.取消了手动配置网络，上电等待wifi模块自动连接，若不成功则使用OneNET公众号进行配网。
	*					  3.NET_DEVICE_SendCmd新增参数“mode”，决定是否清除本次命令的返回值。
	*				V1.2：1.取消V1.1中的功能3。
	*					  2.更改了数据获取接口，适配rb机制。
	*					  3.取消了透传模式。
	*				V1.3：1.增加“忙”标志判断。
	*					  2.增加模组发送时间间隔，根据模组性能而定。
	*					  3.修改了 NET_DEVICE_Check 函数机制。
	*				V1.4：1.发送链表可选择加入表头或表尾。
	*				V1.5：1.修复 NET_DEVICE_AddDataSendList 函数添加到链表头造成野内存问题。
	*				V1.6：1.修改 NET_DEVICE_Connect 函数，保证连接的稳定性。
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f10x.h"

//单片机相关组件
#include "mcu_gpio.h"

//网络设备
#include "net_device.h"
#include "net_io.h"

//硬件驱动
#include "delay.h"
#include "usart.h"

#if(NET_TIME_EN == 1)
#include <time.h>
#endif

//C库
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


static _Bool nd_busy[2] = {0, 0};


NET_DEVICE_INFO net_device_info = {NULL, NULL, NULL, NULL, NULL,
									0, 0, 0, 0, 60,
									0, 0, 0, 0, 0, 1, 0};


GPS_INFO gps;


#if(LBS_WIFI_EN == 1)
LBS_WIFI_INFO lbs_wifi_info;
#endif


//为了通用性，gpio设备列表里的name固定这么写
const static GPIO_LIST net_device_gpio_list[] = {
													{GPIOC, GPIO_Pin_14, "nd_rst"},
												};


static _Bool cmd_resp_err_flag = 0;				//1-结果返回中检索到“ERROR”


static unsigned int network_delay_ticks = 0;


/*
************************************************************
*	函数名称：	NET_DEVCIE_IsBusReady
*
*	函数功能：	查询总线是否就绪
*
*	入口参数：	type：1-数据发送		0-命令发送
*
*	返回参数：	0-就绪	1-未就绪
*
*	说明：		
************************************************************
*/
__inline static _Bool NET_DEVCIE_IsBusReady(_Bool type)
{
	
	_Bool result = 1;

	RTOS_ENTER_CRITICAL();
	
	if(nd_busy[type] == 0)
	{
		nd_busy[type] = 1;
		
		result = 0;
	}
	
	RTOS_EXIT_CRITICAL();
	
	return result;

}

/*
************************************************************
*	函数名称：	NET_DEVCIE_FreeBus
*
*	函数功能：	释放总线
*
*	入口参数：	type：1-数据发送		0-命令发送
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
__inline static void NET_DEVCIE_FreeBus(_Bool type)
{

	RTOS_ENTER_CRITICAL();
	
	nd_busy[type] = 0;
	
	RTOS_EXIT_CRITICAL();

}

/*
************************************************************
*	函数名称：	NET_DEVICE_Delay
*
*	函数功能：	延时
*
*	入口参数：	time：延时时间
*
*	返回参数：	无
*
*	说明：		基于当前延时时基
************************************************************
*/
__inline static void NET_DEVICE_Delay(unsigned int time)
{

	RTOS_TimeDly(time);

}

//==========================================================
//	函数名称：	NET_DEVICE_PowerCtl
//
//	函数功能：	网络设备电源控制
//
//	入口参数：	flag：1-打开电源		0-关闭
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void NET_DEVICE_PowerCtl(_Bool flag)
{

	

}

//==========================================================
//	函数名称：	NET_DEVICE_IO_Init
//
//	函数功能：	初始化网络设备IO层
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		初始化网络设备的控制引脚、数据收发功能等
//==========================================================
void NET_DEVICE_IO_Init(void)
{
	
	MCU_GPIO_Init(net_device_gpio_list[0].gpio_group, net_device_gpio_list[0].gpio_pin, GPIO_Mode_Out_PP, GPIO_Speed_50MHz, net_device_gpio_list[0].gpio_name);
	
	NET_IO_Init();											//网络设备数据IO层初始化
	
	if(net_device_info.reboot == 0)
	{
		NET_DEVICE_PowerCtl(1);
	}
	
	net_device_info.reboot = 0;

}

//==========================================================
//	函数名称：	NET_DEVICE_GetSerial
//
//	函数功能：	获取一个唯一串号
//
//	入口参数：	serial：指针地址
//
//	返回参数：	0-成功	1-失败
//
//	说明：		唯一串号：GSM模组可以是IMEI等唯一ID；WIFI模组可以获取MAC地址等
//==========================================================
_Bool NET_DEVICE_GetSerial(char **serial)
{
	
	_Bool result = 1;
	char *data_ptr = NULL, *data_ptr_t = NULL;
	unsigned char len = 0, i = 0;
	
	if(!NET_DEVICE_SendCmd("AT+CWJAP?\r\n", "OK", 400))
	{
		//+CSQ: +CWJAP:"ONENET","xx:xx:xx:xx:xx:xx",6,-28\r\n\r\nOK
		data_ptr = net_device_info.cmd_resp;
		
		//找到serial开头
		while(*data_ptr != '\0' && i < 3)
		{
			if(*data_ptr++ == '"')
				i++;
		}
		
		if(i == 3)
		{
			data_ptr_t = data_ptr;
		
			//计算serial长度
			while(*data_ptr_t != '"')
			{
				len++;
				
				if(*data_ptr_t++ == '\0')
					return result;
			}
			
			*serial = (char *)NET_MallocBuffer(len + 1);
			if(*serial == NULL)
				return result;
			
			memset(*serial, 0, len + 1);
			
			//复制数据
			memcpy(*serial, data_ptr, len);
			
			UsartPrintf(USART_DEBUG, "Serial: %s\r\n", *serial);
			
			result = 0;
		}
	}
	
	return result;

}

//==========================================================
//	函数名称：	NET_DEVICE_GetSignal
//
//	函数功能：	获取信号值
//
//	入口参数：	无
//
//	返回参数：	返回信号值
//
//	说明：		
//==========================================================
signed char NET_DEVICE_GetSignal(void)
{
	
	char *data_ptr = NULL;
	char num_buf[4] = {0, 0, 0, 0};
	unsigned char i = 0;
	
	if(!net_device_info.net_work)
		return 0;
	
	//+CSQ: +CWJAP:"ONENET","xx:xx:xx:xx:xx:xx",6,-28\r\n\r\nOK
	if(NET_DEVICE_SendCmd("AT+CWJAP?\r\n", "OK", 400) == 0)
	{
		data_ptr = net_device_info.cmd_resp;
		
		while(*data_ptr != '\0' && i < 3)
		{
			if(*data_ptr++ == ',')
				i++;
		}
		
		if(i == 3)
		{
			i = 0;
			while(*data_ptr != '\r')
				num_buf[i++] = *data_ptr++;
			
			net_device_info.signal = (signed char)atoi(num_buf);
		}
		else
			net_device_info.signal = 0;
	}
	else
		net_device_info.signal = 0;
	
	return net_device_info.signal;

}

//==========================================================
//	函数名称：	NET_DEVICE_GetMacs
//
//	函数功能：	获取周围热点的mac
//
//	入口参数：	无
//
//	返回参数：	0-成功	1-失败
//
//	说明：		
//==========================================================
_Bool NET_DEVICE_GetMacs(void)
{
	
#if(LBS_WIFI_EN == 1)
	unsigned char find_cnt = 2, err_cnt = 10, suc_cnt = 0;

	//+CWLAP:(0,"SSID",-75,"xx:xx:xx:xx:xx:xx",4,-4,0)
	if(NET_DEVICE_SendCmd("AT+CWLAP\r\n", "+CWLAP:", 400) == 0)
	{
		char *data_ptr = net_device_info.cmd_resp;
		
		memset(lbs_wifi_info.mac, 0, sizeof(lbs_wifi_info.mac));
		
		while(find_cnt && err_cnt--)
		{
			data_ptr = strstr(data_ptr, "+CWLAP:");
			if(data_ptr)
			{
				char *data_ptr_t = NULL;
				unsigned char i = 0, j = 0;
				_Bool flag = 0;
				
				data_ptr_t = strchr(data_ptr, '(');
				while(*data_ptr_t != ')')										//判断完整性
				{
					if(*data_ptr_t++ == ',')
					{
						if(++i == 6)
						{
							flag = 1;
							break;
						}
					}
					
					if(++j >= 128)												//数据不完整时，如果检测次数过多则放弃
						break;
				}
				
				if(flag)
				{
					char *data_ptr_t = data_ptr;
					unsigned char i = 0;
					
					while(*data_ptr_t != ')' && i < 3)
					{
						if(*data_ptr_t++ == ',')
							i++;
					}
					
					if(i == 3)
					{
						data_ptr_t++;
						suc_cnt++;
						strncat(lbs_wifi_info.mac, data_ptr_t, 17);
						if(--find_cnt)
							strcat(lbs_wifi_info.mac, "|");
					}
				}
			}
			
			data_ptr++;
		}
		
		if(lbs_wifi_info.mac[strlen(lbs_wifi_info.mac) - 1] == '|')
			lbs_wifi_info.mac[strlen(lbs_wifi_info.mac) - 1] = 0;
		
		UsartPrintf(USART_DEBUG, "Mac: %s\r\n", lbs_wifi_info.mac);
		
		if(suc_cnt >= 2)
			lbs_wifi_info.lbs_wifi_ok = 1;
	}
	else
		UsartPrintf(USART_DEBUG, "Mac: Err\r\n");
	
	return suc_cnt < 2 ? 1 : 0;
#else
	return 0;
#endif

}

//==========================================================
//	函数名称：	NET_DEVICE_GetTime
//
//	函数功能：	获取网络时间
//
//	入口参数：	无
//
//	返回参数：	UTC秒值
//
//	说明：		可搜索NTP协议相关资料
//				NTP服务器：UDP协议，端口123
//
//				已测试可用的NTP服务器-2017-11-07
//				1.cn.pool.ntp.org		来源：网上抄的，不知道哪来的(注意“1.”不是序号，是域名的一部分)
//				cn.ntp.org.cn			来源：中国
//				edu.ntp.org.cn			来源：中国教育网
//				183.230.40.42			来源：OneNET
//				tw.ntp.org.cn			来源：中国台湾
//				us.ntp.org.cn			来源：美国
//				sgp.ntp.org.cn			来源：新加坡
//				kr.ntp.org.cn			来源：韩国
//				de.ntp.org.cn			来源：德国
//				jp.ntp.org.cn			来源：日本
//==========================================================
unsigned int NET_DEVICE_GetTime(void)
{
	
	unsigned int second = 0;
	
#if(NET_TIME_EN == 1)
	struct tm *local_time;
	unsigned char time_out = 200;
	char *data_ptr = NULL;
	unsigned char times = 0;
	char *ntp_server[] = {"183.230.40.42", "cn.ntp.org.cn", "edu.ntp.org.cn"};
	
	unsigned char time_buffer[48];

//	NET_DEVICE_Close();
	
	while(times < sizeof(ntp_server) / sizeof(ntp_server[0]))
	{
		if(NET_DEVICE_Connect("UDP", ntp_server[times], "123") == 0)
		{
			memset(time_buffer, 0, sizeof(time_buffer));
			
			time_buffer[0] = 0xE3;							//LI, Version, Mode
			time_buffer[1] = 0;								//表示本地时钟的层次水平
			time_buffer[2] = 6;								//八位signed integer，表示连续信息之间的最大间隔
			time_buffer[3] = 0xEB;							//表示本地时钟精度，精确到秒的平方级
			
			NET_DEVICE_SendData(time_buffer, sizeof(time_buffer));
			net_device_info.cmd_ipd = NULL;
			
			while(--time_out)
			{
				if(net_device_info.cmd_ipd != NULL)
					break;
				
				NET_DEVICE_Delay(2);
			}
			
//			NET_DEVICE_Close();
			
			if(net_device_info.cmd_ipd)
			{
				data_ptr = net_device_info.cmd_ipd;
				
				if(((*data_ptr >> 6) & 0x03) == 3)			//bit6和bit7同为1表示当前不可对时（服务器处于闰秒状态）
					break;
				
				second = *(data_ptr + 40) << 24 | *(data_ptr + 41) << 16 | *(data_ptr + 42) << 8 | *(data_ptr + 43);
				second -= 2208960000UL;						//时区修正
				
				local_time = localtime(&second);
				
				UsartPrintf(USART_DEBUG, "UTC Time: %d-%d-%d %d:%d:%d\r\n",
										local_time->tm_year + 1900, local_time->tm_mon + 1, local_time->tm_mday,
										local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
				
				break;
			}
		}
		
		++times;
		NET_DEVICE_Delay(100);
	}
#endif
	
	return second;

}

//==========================================================
//	函数名称：	NET_DEVICE_Exist
//
//	函数功能：	网络设备存在检查
//
//	入口参数：	无
//
//	返回参数：	返回结果
//
//	说明：		0-成功		1-失败
//==========================================================
_Bool NET_DEVICE_Exist(void)
{

	unsigned char time_out = 10;
	unsigned char config_time = 0;
	_Bool status = 1;
	unsigned char key_count = 0, wait_count = 0, wifi_status = 0;
	
	while(time_out--)												//等待
	{
		NET_DEVICE_Delay(200);										//挂起等待
		
		if(MCU_GPIO_Input_Read("key1") == 0)						//长按进入配网模式
			key_count++;
		else
			key_count = 0;
		
		wifi_status = NET_DEVICE_Check();							//获取状态
		if(wifi_status == NET_DEVICE_NO_DEVICE)
			wait_count++;
		
		if(wifi_status || key_count)								//如果收到数据
		{
			if(wifi_status == NET_DEVICE_GOT_IP && !key_count)
			{
				status = 0;
				break;
			}
			else if(wait_count >= 10 || key_count)
			{
				wait_count = 0;
				key_count = 0;
#if(PHONE_AP_MODE == 0)
				NET_DEVICE_SendCmd("AT+CWMODE=1\r\n", "OK", 200);
				NET_DEVICE_Delay(20);
				NET_DEVICE_SendCmd("AT+CWDHCP=1,1\r\n", "OK", 200);
				NET_DEVICE_Delay(20);
				NET_DEVICE_SendCmd("AT+CWSTARTSMART=2\r\n", "OK", 400);
				UsartPrintf(USART_DEBUG, "请使用OneNET微信公众号配置SSID和PSWD\r\n");
				
				while(1)
				{
					MCU_GPIO_Output_Ctl("led1", 0);MCU_GPIO_Output_Ctl("led2", 0);
					MCU_GPIO_Output_Ctl("led3", 0);MCU_GPIO_Output_Ctl("led4", 0);
					
					if(strstr((char *)net_device_info.cmd_resp, "Smart get wifi info"))
					{
						UsartPrintf(USART_DEBUG, "收到:\r\n%s\r\n", strstr((char *)net_device_info.cmd_resp, "ssid:"));
						status = 1;
						
						MCU_GPIO_Output_Ctl("led1", 1);MCU_GPIO_Output_Ctl("led2", 1);
						MCU_GPIO_Output_Ctl("led3", 1);MCU_GPIO_Output_Ctl("led4", 1);
						
						break;
					}
					else
					{
						if(++config_time >= 30)													//超时时间--30s
						{
							config_time = 0;
							
							MCU_GPIO_Output_Ctl("led1", 1);MCU_GPIO_Output_Ctl("led2", 1);
							MCU_GPIO_Output_Ctl("led3", 1);MCU_GPIO_Output_Ctl("led4", 1);
							
							break;
						}
					}
					
					NET_DEVICE_Delay(100);
					
					MCU_GPIO_Output_Ctl("led1", 1);MCU_GPIO_Output_Ctl("led2", 1);
					MCU_GPIO_Output_Ctl("led3", 1);MCU_GPIO_Output_Ctl("led4", 1);
					
					NET_DEVICE_Delay(100);
				}
				
				if(config_time != 0)															//如果为0，则是超时退出
				{
					config_time = 0;
					while(NET_DEVICE_Check() != NET_DEVICE_GOT_IP)								//等待WIFI接入
					{
						if(++config_time >= 10)
						{
							UsartPrintf(USART_DEBUG, "接入超时,请检查WIFI配置\r\n");
							break;
						}
						
						NET_DEVICE_Delay(200);
					}
				}
#else
				UsartPrintf(USART_DEBUG, "STA Tips:	Link Wifi\r\n");
				
				while(NET_DEVICE_SendCmd("AT\r\n", "OK", 200))
					NET_DEVICE_Delay(100);
				
				status = 0;
				
				break;
#endif
			}
			else
				status = 1;
		}
	}
	
	return status;

}

//==========================================================
//	函数名称：	NET_DEVICE_Init
//
//	函数功能：	网络设备初始化
//
//	入口参数：	无
//
//	返回参数：	返回初始化结果
//
//	说明：		0-成功		1-失败
//==========================================================
_Bool NET_DEVICE_Init(void)
{
	
	_Bool status = 1;
	
	net_device_info.net_work = 0;

	switch(net_device_info.init_step)
	{
		case 0:
		
			UsartPrintf(USART_DEBUG, "Tips:	AT+CWMODE_CUR=1\r\n");
			if(!NET_DEVICE_SendCmd("AT+CWMODE_CUR=1\r\n", "OK", 200))
				net_device_info.init_step++;
		
		break;
		
		case 1:
		
			UsartPrintf(USART_DEBUG, "Tips:	AT+CIPMUX=0\r\n");
			if(!NET_DEVICE_SendCmd("AT+CIPMUX=0\r\n", "OK", 200))
				net_device_info.init_step++;
			else
			{
				if(strstr(net_device_info.cmd_resp, "link is builded"))
					net_device_info.init_step++;
			}
		
		break;
		
		case 2:
		
			UsartPrintf(USART_DEBUG, "Tips:	AT+CWDHCP=1,1\r\n");
			if(!NET_DEVICE_SendCmd("AT+CWDHCP=1,1\r\n", "OK", 200))
				net_device_info.init_step++;
		
		break;
		
		case 3:
		
#if(PHONE_AP_MODE == 1)
			UsartPrintf(USART_DEBUG, "Tips:	AT+CWJAP\r\n");
			if(!NET_DEVICE_SendCmd("AT+CWJAP=\"ONENET\",\"IOT@Chinamobile123\"\r\n", "GOT IP", 400))
#endif
			net_device_info.init_step++;
		break;
		
		case 4:
		
			if(NET_DEVICE_GetMacs() == 0)
				net_device_info.init_step++;
		
		break;
		
		case 5:
			
#if(NET_TIME_EN == 1)
			if(!net_device_info.net_time)
				net_device_info.net_time = NET_DEVICE_GetTime();
#endif
			net_device_info.init_step++;
			
		break;
		
		case 6:
		
			net_device_info.send_count = 0;
			UsartPrintf(USART_DEBUG, "Tips:	ESP8266 STA_Mode OK\r\n");
			net_device_info.init_step++;
		
		break;
		
		default:
			
			status = 0;
			net_device_info.net_work = 1;
		
		break;
	}
	
	return status;

}

//==========================================================
//	函数名称：	NET_DEVICE_Reset
//
//	函数功能：	网络设备复位
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void NET_DEVICE_Reset(void)
{

	UsartPrintf(USART_DEBUG, "Tips:	NET_DEVICE_Reset\r\n");
	
	MCU_GPIO_Output_Ctl("nd_rst", 0);		//复位
	NET_DEVICE_Delay(50);
	
	MCU_GPIO_Output_Ctl("nd_rst", 1);		//结束复位
	NET_DEVICE_Delay(200);

}

//==========================================================
//	函数名称：	NET_DEVICE_Close
//
//	函数功能：	关闭网络连接
//
//	入口参数：	无
//
//	返回参数：	0-成功	1-失败
//
//	说明：		
//==========================================================
_Bool NET_DEVICE_Close(void)
{
	
	_Bool result = 1;
	
	UsartPrintf(USART_DEBUG, "Tips:	CLOSE\r\n");

	result = NET_DEVICE_SendCmd("AT+CIPCLOSE\r\n", "OK", 400);
	
	NET_DEVICE_Delay(30);
	
	return result;

}

//==========================================================
//	函数名称：	NET_DEVICE_Connect
//
//	函数功能：	重连平台
//
//	入口参数：	type：TCP 或 UDP
//				ip：IP地址缓存指针
//				port：端口缓存指针
//
//	返回参数：	返回连接结果
//
//	说明：		0-成功		1-失败
//==========================================================
_Bool NET_DEVICE_Connect(char *type, char *ip, char *port)
{
	
	_Bool status = 1;
	char cmd_buf[48];
	unsigned char err_count = 5;
	
	snprintf(cmd_buf, sizeof(cmd_buf), "AT+CIPSTART=\"%s\",\"%s\",%s\r\n", type, ip, port);
	
	UsartPrintf(USART_DEBUG, "Tips:	%s", cmd_buf);

	do
	{
		
		NET_DEVICE_Close();
		status = NET_DEVICE_SendCmd(cmd_buf, "OK", 1000);
		
		if(status == 0)
		{
			err_count = 0;
		}
		else
		{
			unsigned char connect_status = NET_DEVICE_Check();
			
			if((*type == 'U' && connect_status == NET_DEVICE_GOT_IP) ||
				(*type == 'T' && connect_status == NET_DEVICE_CONNECTED))
			{
				err_count = 0;
			}
			else
			{
				err_count--;
				
				NET_DEVICE_Delay(100);
			}
		}
		
	} while(err_count);
	
	return status;

}

//==========================================================
//	函数名称：	NET_DEVICE_CmdHandle
//
//	函数功能：	检查命令返回是否正确
//
//	入口参数：	cmd：需要发送的命令
//
//	返回参数：	无
//
//	说明：		命令处理成功则将指针置NULL
//==========================================================
void NET_DEVICE_CmdHandle(char *cmd)
{
	
	if(strstr(cmd, net_device_info.cmd_hdl) != NULL)
		net_device_info.cmd_hdl = NULL;
	else if(strstr(cmd, "ERROR") != NULL)
	{
		cmd_resp_err_flag = 1;
		net_device_info.cmd_hdl = NULL;
	}
	
	net_device_info.cmd_resp = cmd;

}

//==========================================================
//	函数名称：	NET_DEVICE_SendCmd
//
//	函数功能：	向网络设备发送一条命令，并等待正确的响应
//
//	入口参数：	cmd：需要发送的命令
//				res：需要检索的响应
//				time_out：等待时间(以系统时基为准)
//
//	返回参数：	返回连接结果
//
//	说明：		0-成功		1-失败
//==========================================================
_Bool NET_DEVICE_SendCmd(char *cmd, char *res, unsigned short time_out)
{
	
	_Bool result = 0;
	
	if(NET_DEVCIE_IsBusReady(0) == 1)
		return 1;
	
	NET_IO_Send((unsigned char *)cmd, strlen(cmd));			//写命令到网络设备
	
	if(res == NULL)											//如果为空，则只是发送
	{
		NET_DEVCIE_FreeBus(0);
		
		return 0;
	}
	
	net_device_info.cmd_hdl = res;							//需要所搜的关键词
	
	while((net_device_info.cmd_hdl != NULL) && --time_out)	//等待
		NET_DEVICE_Delay(2);
	
	if((time_out == 0) || (cmd_resp_err_flag == 1))
	{
		cmd_resp_err_flag = 0;
		result = 1;
	}
	
	NET_DEVCIE_FreeBus(0);
	
	return result;

}

//==========================================================
//	函数名称：	NET_DEVICE_SendData
//
//	函数功能：	使网络设备发送数据到平台
//
//	入口参数：	data：需要发送的数据
//				len：数据长度
//
//	返回参数：	0-发送完成	1-发送失败
//
//	说明：		
//==========================================================
_Bool NET_DEVICE_SendData(unsigned char *data, unsigned short len)
{
	
	_Bool result = 1;
	char cmd_buf[40];
	
	if(NET_DEVCIE_IsBusReady(1) == 1)
		return 1;
	
	net_device_info.send_count++;
	
	NET_DEVICE_Delay(1);

	sprintf(cmd_buf, "AT+CIPSEND=%d\r\n", len);		//发送命令
	if(!NET_DEVICE_SendCmd(cmd_buf, "OK", 400)) 	//收到‘>’时可以发送数据
	{
		NET_IO_Send(data, len);  					//发送设备连接请求数据
		
		network_delay_ticks = RTOS_GetTicks();
		
		result = 0;
	}
	
	NET_DEVICE_Delay(10);
	
	NET_DEVCIE_FreeBus(1);
	
	return result;

}

//==========================================================
//	函数名称：	NET_DEVICE_Read
//
//	函数功能：	读取一帧数据
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
unsigned char *NET_DEVICE_Read(void)
{

	return NET_IO_Read();

}

//==========================================================
//	函数名称：	NET_DEVICE_GetIPD
//
//	函数功能：	获取平台返回的数据
//
//	入口参数：	data_ptr：原始数据指针
//
//	返回参数：	平台返回的原始数据
//
//	说明：		不同网络设备返回的格式不同，需要去调试
//				如ESP8266的返回格式为	"+IPD,x:yyy"	x代表数据长度，yyy是数据内容
//==========================================================
unsigned char *NET_DEVICE_GetIPD(unsigned char *data_ptr)
{
	
	char *data_ptr_t = (char *)data_ptr;
	
	if(data_ptr_t != NULL)
	{
		data_ptr_t = strstr(data_ptr_t, "IPD,");					//搜索“IPD”头
		if(data_ptr_t == NULL)										//如果没找到，可能是IPD头的延迟，还是需要等待一会，但不会超过设定的时间
		{
			//UsartPrintf(USART_DEBUG, "\"IPD\" not found\r\n");
		}
		else
		{
			if(sscanf(data_ptr_t, "IPD,%d", &net_device_info.data_bytes) == 1)
			{
				data_ptr_t = strchr(data_ptr_t, ':');
				
				if(data_ptr_t != NULL)
				{
					data_ptr_t++;
					
					net_device_info.network_delay_time = (RTOS_GetTicks() - network_delay_ticks) * (1000 / RTOS_TICK_PER_SEC);
					//UsartPrintf(USART_DEBUG, "network delay time: %d ms\r\n", net_device_info.network_delay_time);
					
					return (unsigned char *)(data_ptr_t);
				}
			}
		}
	}

	return NULL;													//超时还未找到，返回空指针

}

//==========================================================
//	函数名称：	NET_DEVICE_Check
//
//	函数功能：	检查网络设备连接状态
//
//	入口参数：	无
//
//	返回参数：	返回状态
//
//	说明：		
//==========================================================
unsigned char NET_DEVICE_Check(void)
{
	
	unsigned char status = NET_DEVICE_NO_DEVICE, err_count = 3;
	int status_result = 255;
	char *data_ptr = NULL;
	
	while(--err_count)
	{
		if(NET_DEVICE_SendCmd("AT+CIPSTATUS\r\n", "OK", 400) == 0)				//发送状态监测
		{
			data_ptr = strstr(net_device_info.cmd_resp, "STATUS:");
			if(sscanf(data_ptr, "STATUS:%d", &status_result) == 1)
			{
				switch(status_result)
				{
					case 2:
						UsartPrintf(USART_DEBUG, "Tips:	NET_DEVICE Got IP\r\n");
						status = NET_DEVICE_GOT_IP;
					break;
					
					case 3:
						UsartPrintf(USART_DEBUG, "Tips:	NET_DEVICE Connect OK\r\n");
						status = NET_DEVICE_CONNECTED;
					break;
					
					case 4:
						UsartPrintf(USART_DEBUG, "Tips:	NET_DEVICE Lost Connect\r\n");
						status = NET_DEVICE_CLOSED;
					break;
					
					case 5:
						//UsartPrintf(USART_DEBUG, "Tips:	NET_DEVICE Lost\r\n");		//设备丢失
						status = NET_DEVICE_NO_DEVICE;
					break;
				}
				
				break;
			}
		}
		
		NET_DEVICE_Delay(20);
	}
	
	return status;

}

//==========================================================
//	函数名称：	NET_DEVICE_ReConfig
//
//	函数功能：	设备网络设备初始化的步骤
//
//	入口参数：	步骤值
//
//	返回参数：	无
//
//	说明：		该函数设置的参数在网络设备初始化里边用到
//==========================================================
void NET_DEVICE_ReConfig(unsigned char step)
{

	net_device_info.init_step = step;

}

/******************************************************************************************
										消息队列
******************************************************************************************/

//==========================================================
//	函数名称：	NET_DEVICE_CheckListHead
//
//	函数功能：	检查发送链表头是否为空
//
//	入口参数：	无
//
//	返回参数：	0-空	1-不为空
//
//	说明：		
//==========================================================
_Bool NET_DEVICE_CheckListHead(void)
{

	if(net_device_info.head == NULL)
		return 0;
	else
		return 1;

}

//==========================================================
//	函数名称：	NET_DEVICE_GetListHeadBuf
//
//	函数功能：	获取链表里需要发送的数据指针
//
//	入口参数：	无
//
//	返回参数：	获取链表里需要发送的数据指针
//
//	说明：		
//==========================================================
unsigned char *NET_DEVICE_GetListHeadBuf(void)
{

	return net_device_info.head->buf;

}

//==========================================================
//	函数名称：	NET_DEVICE_GetListHeadLen
//
//	函数功能：	获取链表里需要发送的数据长度
//
//	入口参数：	无
//
//	返回参数：	获取链表里需要发送的数据长度
//
//	说明：		
//==========================================================
unsigned short NET_DEVICE_GetListHeadLen(void)
{

	return net_device_info.head->dataLen;

}

//==========================================================
//	函数名称：	NET_DEVICE_AddDataSendList
//
//	函数功能：	在发送链表尾新增一个发送链表
//
//	入口参数：	buf：需要发送的数据
//				data_len：数据长度
//				mode：0-添加到头部	1-添加到尾部
//
//	返回参数：	0-成功	其他-失败
//
//	说明：		异步发送方式
//==========================================================
unsigned char NET_DEVICE_AddDataSendList(unsigned char *buf ,unsigned short data_len, _Bool mode)
{
	
	struct NET_SEND_LIST *current = (struct NET_SEND_LIST *)NET_MallocBuffer(sizeof(struct NET_SEND_LIST));
																//分配内存
	
	_Bool head_insert_flag = 0;
	
	if(current == NULL)
		return 1;
	
	current->buf = (unsigned char *)NET_MallocBuffer(data_len);	//分配内存
	if(current->buf == NULL)
	{
		NET_FreeBuffer(current);								//失败则释放
		return 2;
	}
	
	if(net_device_info.head == NULL)							//如果head为NULL
		net_device_info.head = current;							//head指向当前分配的内存区
	else														//如果head不为NULL
	{
		if(mode)
			net_device_info.end->next = current;				//则end指向当前分配的内存区
		else
		{
			struct NET_SEND_LIST *head_t = NULL;
			
			head_t = net_device_info.head;
			net_device_info.head = current;
			current->next = head_t;
			
			head_insert_flag = 1;
		}
	}
	
	memcpy(current->buf, buf, data_len);						//复制数据
	current->dataLen = data_len;
	if(head_insert_flag == 0)
	{
		current->next = NULL;									//下一段为NULL
		net_device_info.end = current;							//end指向当前分配的内存区
	}
	
	return 0;

}

//==========================================================
//	函数名称：	NET_DEVICE_DeleteDataSendList
//
//	函数功能：	从链表头删除一个链表
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
_Bool NET_DEVICE_DeleteDataSendList(void)
{
	
	struct NET_SEND_LIST *next = net_device_info.head->next;	//保存链表头的下一段数据地址
	
	net_device_info.head->dataLen = 0;
	net_device_info.head->next = NULL;
	NET_FreeBuffer(net_device_info.head->buf);					//释放内存
	net_device_info.head->buf = NULL;
	NET_FreeBuffer(net_device_info.head);						//释放内存
	
	net_device_info.head = next;								//链表头指向下一段数据
	
	return 0;

}
