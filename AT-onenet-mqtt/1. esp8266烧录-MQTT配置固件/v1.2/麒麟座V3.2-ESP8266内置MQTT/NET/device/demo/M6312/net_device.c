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
	*					  2.增加基站定位功能，在net_device.h里通过宏M631X_LOCATION来确实是否使用。
	*					  3.NET_DEVICE_SendCmd新增参数“mode”，决定是否清除本次命令的返回值。
	*				V1.2：1.取消V1.1中的功能3。
	*					  2.更改了数据获取接口，适配rb机制。
	*					  3.取消了透传模式
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
									0, 0, 0, 0, 250,
									0, 0, 0, 0, 0, 1, 0};


GPS_INFO gps;


#if(LBS_EN == 1)
LBS_INFO lbs_info;
#endif


//为了通用性，gpio设备列表里的name固定这么写
const static GPIO_LIST net_device_gpio_list[] = {
													{GPIOC, GPIO_Pin_15, "nd_rst"},
													{GPIOC, GPIO_Pin_4, "nd_pwrk"},
													{GPIOC, GPIO_Pin_5, "nd_sta"},
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

	if(flag == 1)
	{
		MCU_GPIO_Output_Ctl("nd_pwrk", 1);
		
		while(!MCU_GPIO_Input_Read("nd_sta"))
			NET_DEVICE_Delay(2);
		
		MCU_GPIO_Output_Ctl("nd_pwrk", 0);
	}
	else
	{
		MCU_GPIO_Output_Ctl("nd_pwrk", 1);
		
		while(MCU_GPIO_Input_Read("nd_sta"))
			NET_DEVICE_Delay(2);
		
		MCU_GPIO_Output_Ctl("nd_pwrk", 0);
	}

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
	
	MCU_GPIO_Init(net_device_gpio_list[1].gpio_group, net_device_gpio_list[1].gpio_pin, GPIO_Mode_Out_PP, GPIO_Speed_50MHz, net_device_gpio_list[1].gpio_name);
	
	MCU_GPIO_Init(net_device_gpio_list[2].gpio_group, net_device_gpio_list[2].gpio_pin, GPIO_Mode_IPD, GPIO_Speed_50MHz, net_device_gpio_list[2].gpio_name);
	
	NET_IO_Init();									//网络设备数据IO层初始化
	
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
	unsigned char len = 0;
	
	if(!NET_DEVICE_SendCmd("AT+CGSN\r\n", "+CGSN:", 200))
	{
		//+CGSN: 865533030194330\r\nOK
		data_ptr = net_device_info.cmd_resp;
		
		//找到serial开头
		while(*data_ptr < '0' || *data_ptr > '9')
		{
			if(*data_ptr++ == '\0')
				return result;
		}
		
		data_ptr_t = data_ptr;
		
		//计算serial长度
		while(*data_ptr_t >= '0' && *data_ptr_t <= '9')
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
	
	//+CSQ: 31,0\r\n\r\nOK
	if(NET_DEVICE_SendCmd("AT+CSQ\r\n", "OK", 200) == 0)
	{
		data_ptr = net_device_info.cmd_resp;
		
		while(*data_ptr < '0' || *data_ptr > '9')
			data_ptr++;
		
		while(*data_ptr >= '0' && *data_ptr <= '9')
			num_buf[i++] = *data_ptr++;
		
		net_device_info.signal = (signed char)atoi(num_buf);
	}
	else
		net_device_info.signal = 0;
	
	return net_device_info.signal;

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

	NET_DEVICE_Close();
	
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
			
			NET_DEVICE_Close();
			
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

	return NET_DEVICE_SendCmd("AT\r\n", "OK", 200);

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
	
	NET_DEVICE_Delay(200);
	
	switch(net_device_info.init_step)
	{
		case 0:
			if(!NET_DEVICE_SendCmd("AT+CMVERSION\r\n", "CMIOT", 200))		//查询软件版本号
			{
				UsartPrintf(USART_DEBUG, "%s\r\n", net_device_info.cmd_resp);
				net_device_info.init_step++;
				NET_DEVICE_Delay(200);
			}
		break;
			
		case 1:
			UsartPrintf(USART_DEBUG, "Tips:	ATE0\r\n");
			if(!NET_DEVICE_SendCmd("ATE0\r\n", "OK", 200))					//关闭回显
				net_device_info.init_step++;
		break;
			
		case 2:
			UsartPrintf(USART_DEBUG, "Tips:	AT+CPIN?\r\n");
			if(!NET_DEVICE_SendCmd("AT+CPIN?\r\n", "+CPIN: READY", 400))	//确保SIM卡PIN码解锁，返回READY，表示解锁成功
				net_device_info.init_step++;
		break;
		
		case 3:																//自动判断卡类型
		{
			UsartPrintf(USART_DEBUG, "Tips:	AT+CREG?\r\n");
			
			if(NET_DEVICE_SendCmd("AT+CREG?\r\n", "1,1", 400) == 0)
			{
				net_device_info.init_step++;
			}
			else
			{
				if(strstr(net_device_info.cmd_resp, "1,5"))
					net_device_info.init_step++;
			}
		}
		break;
			
		case 4:
			UsartPrintf(USART_DEBUG, "Tips:	AT+CGREG=2\r\n");				//使网络注册状态，位置消息的主动回显有效
			if(!NET_DEVICE_SendCmd("AT+CGREG=2\r\n", "OK", 400))
				net_device_info.init_step++;
		break;
			
		case 5:
			UsartPrintf(USART_DEBUG, "Tips:	AT+CGREG?\r\n");				//检查网络注册状态
			if(!NET_DEVICE_SendCmd("AT+CGREG?\r\n", "+CGREG:", 400))
			{
				//+CGREG: 2,1,"xxx","xxx",1									//mode,state,lac,ci,AcT
				
#if(LBS_EN == 1)
				char *data_ptr = strchr(net_device_info.cmd_resp, '"');
				if(data_ptr)
				{
					int nt = 0;
					
					if(sscanf(data_ptr, "\"%[^\"]\",\"%[^\"]\",%d", lbs_info.lac, lbs_info.cell_id, &nt) == 3)
					{
						lbs_info.flag = 16;									//数据为hex
						lbs_info.lbs_ok = 1;
						
						lbs_info.network_type = nt;
						UsartPrintf(USART_DEBUG, "cid: %s, lac: %s, nt: %d, flag: %d\r\n", lbs_info.cell_id, lbs_info.lac,
																							lbs_info.network_type, lbs_info.flag);
					}
				}
#endif
				NET_DEVICE_SendCmd("AT+CGREG=0\r\n", NULL, 400);			//取消自动报告
				NET_DEVICE_Delay(20);
				net_device_info.init_step++;
			}
		break;
		
		case 6:
			UsartPrintf(USART_DEBUG, "Tips:	AT+CGACT=1,1\r\n");
			if(!NET_DEVICE_SendCmd("AT+CGACT=1,1\r\n", "OK", 400)) 			//激活
				net_device_info.init_step++;
		break;
		
		case 7:
			UsartPrintf(USART_DEBUG, "Tips:	AT+CGATT=1\r\n");				//附着GPRS业务
			if(!NET_DEVICE_SendCmd("AT+CGATT=1\r\n", "1", 400))
				net_device_info.init_step++;
		break;
			
		case 8:
			UsartPrintf(USART_DEBUG, "Tips:	AT+CMMUX=0\r\n");
			NET_DEVICE_SendCmd("AT+CMMUX=0\r\n", "OK", 200); 				//必须为单连接，不然平台IP都连不上
		
			net_device_info.init_step++;
		break;
			
		case 9:
			UsartPrintf(USART_DEBUG, "Tips:	AT+CMHEAD=1\r\n");
		
			if(!NET_DEVICE_SendCmd("AT+CMHEAD=1\r\n", "OK", 200))			//显示IP头
				net_device_info.init_step++;
		break;
			
		case 10:
		
#if(NET_TIME_EN == 1)
			if(!net_device_info.net_time)
				net_device_info.net_time = NET_DEVICE_GetTime();
#endif
			net_device_info.init_step++;
		
		break;
			
		default:
			net_device_info.send_count = 0;
			net_device_info.net_work = 1;
			status = 0;
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
	
	NET_DEVICE_SendCmd("AT+CMRESET\r\n", NULL, 0);

	MCU_GPIO_Output_Ctl("nd_rst", 1);		//复位
	NET_DEVICE_Delay(50);
	
	MCU_GPIO_Output_Ctl("nd_rst", 0);		//结束复位
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

	result = NET_DEVICE_SendCmd("AT+IPCLOSE\r\n", "OK", 400);
	
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
	
	snprintf(cmd_buf, sizeof(cmd_buf), "AT+IPSTART=\"%s\",\"%s\",%s\r\n", type, ip, port);
	
	UsartPrintf(USART_DEBUG, "Tips:	%s", cmd_buf);
	
	do
	{
		
		NET_DEVICE_Close();
		
		if(*type == 'T')
			status = NET_DEVICE_SendCmd(cmd_buf, "CONNECT OK", 1000);
		else if(*type == 'U')
			status = NET_DEVICE_SendCmd(cmd_buf, "BIND OK", 1000);
		
		if(status == 0)
		{
			err_count = 0;
		}
		else
		{
			unsigned char connect_status = NET_DEVICE_Check();
			
			if(connect_status == NET_DEVICE_CONNECTING)
			{
				unsigned char connect_count = 20;
				unsigned char connect_status_t = NET_DEVICE_NO_DEVICE;
				
				do
				{
					connect_status_t = NET_DEVICE_Check();
					
					if(connect_status_t == NET_DEVICE_CONNECTED)
					{
						status = 0;
						err_count = 0;
						
						break;
					}
					else if(connect_status_t == NET_DEVICE_CLOSED)
					{
						err_count--;
						
						break;
					}
					
					connect_count--;
					if(connect_count == 0)
					{
						err_count--;
						
						if(err_count == 0)
						{
							NET_DEVICE_Reset();
							NET_DEVICE_ReConfig(0);
						}
					}
					
					NET_DEVICE_Delay(200);
				} while(connect_count);
			}
			else if(connect_status == NET_DEVICE_CONNECTED)
			{
				status = 0;
				err_count = 0;
			}
			else if(connect_status == NET_DEVICE_INITIAL)
			{
				return 1;
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
//	else if(strstr(cmd, "ERROR") != NULL)
//	{
//		cmd_resp_err_flag = 1;
//		net_device_info.cmd_hdl = NULL;
//	}
	
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

	sprintf(cmd_buf, "AT+IPSEND=%d\r\n", len);				//发送命令
	if(!NET_DEVICE_SendCmd(cmd_buf, ">", 400))				//收到‘>’时可以发送数据
	{
		NET_IO_Send(data, len);  							//发送设备连接请求数据
		
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

//==========================================================
//	函数名称：	NET_DEVICE_GetIPD
//
//	函数功能：	获取平台返回的数据
//
//	入口参数：	timeOut等待的时间(乘以10ms)
//
//	返回参数：	平台返回的原始数据
//
//	说明：		不同网络设备返回的格式不同，需要去调试
//				如M6312的返回格式为	"<IPDATA: x>\r\nyyy"	x代表数据长度，yyy是数据内容
//==========================================================
unsigned char *NET_DEVICE_GetIPD(unsigned char *data_ptr)
{
	
	char *data_ptr_t = (char *)data_ptr;
	
	if(data_ptr_t != NULL)
	{
		data_ptr_t = strstr(data_ptr_t, "IPDATA:");					//搜索“IPDATA”头
		if(data_ptr_t == NULL)										//如果没找到，可能是IPD头的延迟，还是需要等待一会，但不会超过设定的时间
		{
			//UsartPrintf(USART_DEBUG, "\"IPDATA\" not found\r\n");
		}
		else
		{
			if(sscanf(data_ptr_t, "IPDATA: %d", &net_device_info.data_bytes) == 1)
			{
				data_ptr_t = strchr(data_ptr_t, '\n');
				
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
	
	while(--err_count)
	{
		if(NET_DEVICE_SendCmd("AT+CMSTATE\r\n", "+CMSTATE:", 400) == 0)			//发送状态监测
		{
			if(strstr(net_device_info.cmd_resp, "CONNECT OK"))					//TCP已连接
			{
				UsartPrintf(USART_DEBUG, "Tips:	NET_DEVICE TCP CONNECT OK\r\n");
				status = NET_DEVICE_CONNECTED;
				
				break;
			}
			if(strstr(net_device_info.cmd_resp, "BIND OK"))						//UDP已连接
			{
				UsartPrintf(USART_DEBUG, "Tips:	NET_DEVICE UDP CONNECT OK\r\n");
				status = NET_DEVICE_CONNECTED;
				
				break;
			}
			else if(strstr(net_device_info.cmd_resp, "CONNECTING"))				//连接中...
			{
				UsartPrintf(USART_DEBUG, "Tips:	NET_DEVICECONNECTING...\r\n");
				status = NET_DEVICE_CONNECTING;
				
				break;
			}
			else if(strstr(net_device_info.cmd_resp, "IP CLOSE"))				//已关闭
			{
				UsartPrintf(USART_DEBUG, "Tips:	NET_DEVICECLOSED\r\n");
				status = NET_DEVICE_CLOSED;
				
				break;
			}
			else if(strstr(net_device_info.cmd_resp, "IP INITIA"))				//已关闭
			{
				UsartPrintf(USART_DEBUG, "Tips:	NET_DEVICE INITIA\r\n");		//初始态
				status = NET_DEVICE_INITIAL;
				
				break;
			}
		}
		
		NET_DEVICE_Delay(20);
	}
	
	return status;

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
	
	struct NET_SEND_LIST *next = net_device_info.head->next;
	
	net_device_info.head->dataLen = 0;
	net_device_info.head->next = NULL;
	NET_FreeBuffer(net_device_info.head->buf);
	NET_FreeBuffer(net_device_info.head);
	
	net_device_info.head = next;
	
	return 0;

}
