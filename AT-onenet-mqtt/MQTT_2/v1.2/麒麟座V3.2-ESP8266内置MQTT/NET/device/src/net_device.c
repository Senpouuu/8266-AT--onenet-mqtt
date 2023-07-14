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

//C库
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


static _Bool nd_busy = 0;


NET_DEVICE_INFO net_device_info = {NULL, NULL, NULL, 0, 0, 0};


//为了通用性，gpio设备列表里的name固定这么写
const static GPIO_LIST net_device_gpio_list[] = {
													{GPIOC, GPIO_Pin_14, "nd_rst"},
												};


static _Bool cmd_resp_err_flag = 0;				//1-结果返回中检索到“ERROR”


/*
************************************************************
*	函数名称：	NET_DEVCIE_IsBusReady
*
*	函数功能：	查询总线是否就绪
*
*	入口参数：	
*
*	返回参数：	0-就绪	1-未就绪
*
*	说明：		
************************************************************
*/
__inline static _Bool NET_DEVCIE_IsBusReady(void)
{
	
	_Bool result = 1;

	RTOS_ENTER_CRITICAL();
	
	if(nd_busy == 0)
	{
		nd_busy = 1;
		
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
*	入口参数：	
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
__inline static void NET_DEVCIE_FreeBus(void)
{

	RTOS_ENTER_CRITICAL();
	
	nd_busy = 0;
	
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
	
	NET_DEVICE_PowerCtl(1);

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
//	说明：		
//==========================================================
unsigned int NET_DEVICE_GetTime(void)
{
	
	net_device_info.net_time = 0;

#if(NET_TIME_EN == 1)
	
	UsartPrintf(USART_DEBUG, "Tips:	Wait for NET Time...\r\n");
	
	if(NET_DEVICE_SendCmd("AT+NTP\r\n", "+Event:", 1200) == 0)
	{
		char time_str[24];
		
		if(sscanf(net_device_info.cmd_resp, "+Event:%[^,],%d", time_str, &net_device_info.net_time) == 2)
		{
			UsartPrintf(USART_DEBUG, "Time: %s, %d\r\n", time_str, net_device_info.net_time);
		}
	}
			
#endif
	
	return net_device_info.net_time;

}

//==========================================================
//	函数名称：	NET_DEVICE_GetNWD
//
//	函数功能：	获取网络延迟
//
//	入口参数：	无
//
//	返回参数：	返回结果
//
//	说明：		
//==========================================================
unsigned short NET_DEVICE_GetNWD(void)
{

	if(NET_DEVICE_SendCmd("AT+IOTNWD\r\n", "+Event:", 200) == 0)
	{
		if(sscanf(net_device_info.cmd_resp, "+Event:network delay(ms): %d", &net_device_info.network_delay_time) == 1)
		{
			UsartPrintf(USART_DEBUG, "network delay(ms): %d\r\n", net_device_info.network_delay_time);
		}
	}
	
	return net_device_info.network_delay_time;

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
				NET_DEVICE_SendCmd("AT+AIRKISS\r\n", "Ready", 200);
				UsartPrintf(USART_DEBUG, "请使用OneNET微信公众号配置SSID和PSWD\r\n");
				
				while(1)
				{
					MCU_GPIO_Output_Ctl("led1", 0);MCU_GPIO_Output_Ctl("led2", 0);
					MCU_GPIO_Output_Ctl("led3", 0);MCU_GPIO_Output_Ctl("led4", 0);
					
					if(strstr((char *)net_device_info.cmd_resp, "Airkiss OK"))
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
				
				while(NET_DEVICE_SendCmd("AT+CWJAP=ssid,password\r\n", "WIFI GOT IP", 800))
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

	switch(net_device_info.init_step)
	{
		case 0:
		
			if(NET_DEVICE_Exist() == 0)
			{
				net_device_info.init_step++;
			}
		
		break;
		
		case 1:
		
#if(NET_TIME_EN == 1)
			if(NET_DEVICE_GetTime())
#endif
			{
				net_device_info.init_step++;
			}
		
		break;
		
		default:
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
	
	MCU_GPIO_Output_Ctl("nd_rst", 0);		//复位
	NET_DEVICE_Delay(50);
	
	MCU_GPIO_Output_Ctl("nd_rst", 1);		//结束复位
	NET_DEVICE_Delay(200);

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
	
	if(NET_DEVCIE_IsBusReady() == 1)
		return 1;
	
	NET_IO_Send((unsigned char *)cmd, strlen(cmd));			//写命令到网络设备
	
	if(res == NULL)											//如果为空，则只是发送
	{
		NET_DEVCIE_FreeBus();
		
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
	
	NET_DEVCIE_FreeBus();
	
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
		if(NET_DEVICE_SendCmd("AT+CIPSTATUS\r\n", "+STATUS", 200) == 0)				//发送状态监测
		{
			data_ptr = strstr(net_device_info.cmd_resp, "STATUS:");
			if(sscanf(data_ptr, "STATUS:%d", &status_result) == 1)
			{
				switch(status_result)
				{
					case 0:
						UsartPrintf(USART_DEBUG, "Tips:	NET_DEVICE IDLE\r\n");
						status = NET_DEVICE_INITIAL;
					break;
					
					case 1:
						//UsartPrintf(USART_DEBUG, "Tips:	NET_DEVICE Connecting\r\n");
						status = NET_DEVICE_CONNECTING;
					break;
					
					case 2:
						UsartPrintf(USART_DEBUG, "Tips:	NET_DEVICE Password Err\r\n");
						status = NET_DEVICE_PSWD_ERR;
					break;
					
					case 3:
						UsartPrintf(USART_DEBUG, "Tips:	NET_DEVICE No AP\r\n");
						status = NET_DEVICE_NO_AP;
					break;
					
					case 4:
						UsartPrintf(USART_DEBUG, "Tips:	NET_DEVICE Connect Err\r\n");
						status = NET_DEVICE_CONNCET_ERR;
					break;
					
					case 5:
						UsartPrintf(USART_DEBUG, "Tips:	NET_DEVICE Got IP\r\n");
						status = NET_DEVICE_GOT_IP;
					break;
				}
				
				break;
			}
		}
		
		NET_DEVICE_Delay(20);
	}
	
	return status;

}
