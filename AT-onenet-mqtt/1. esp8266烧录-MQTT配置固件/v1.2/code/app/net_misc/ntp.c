/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	ntp.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2018-01-22
	*
	*	版本： 		V1.0
	*
	*	说明： 		获取NTP服务器数据
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

#include "lwip/udp.h"

//
#include "ntp.h"

//OS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#include "net_device.h"

//C库
#include <string.h>
#include <time.h>


NTP_TIME_INFO ntp_time_info = {0, 0};


//==========================================================
//	函数名称：	NTP_ResetStatus
//
//	函数功能：	重置状态标志
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void NTP_ResetStatus(void)
{

	ntp_time_info.ntp_time = 0;
	ntp_time_info.ntp_time_ok = 0;

}

//==========================================================
//	函数名称：	NTP_GetTime
//
//	函数功能：	获取网络时间
//
//	入口参数：	无
//
//	返回参数：	1-成功	0-失败
//
//	说明：		可搜索NTP协议相关资料
//				NTP服务器：UDP协议，端口123
//
//				已测试可用的NTP服务器-2017-11-07
//				1.cn.pool.ntp.org		来源：网上抄的，不知道哪来的(注意“1.”不是序号，是域名的一部分)
//				cn.ntp.org.cn			来源：中国
//				edu.ntp.org.cn			来源：中国教育网
//				tw.ntp.org.cn			来源：中国台湾
//				us.ntp.org.cn			来源：美国
//				sgp.ntp.org.cn			来源：新加坡
//				kr.ntp.org.cn			来源：韩国
//				de.ntp.org.cn			来源：德国
//				jp.ntp.org.cn			来源：日本
//==========================================================
_Bool NTP_GetTime(void)
{
	
#if(NTP_TIME_EN == 1)
	struct udp_pcb pcb;
	struct pbuf time_stc;
	
	struct tm *local_time;
	unsigned char time_out = 200;
	unsigned char *data_ptr = NULL;
	unsigned char time_buffer[48];
	int socket_id = -1;
	
	if(!net_device_info.net_work)									//网络模组具备网络连接能力
		return ntp_time_info.ntp_time_ok;
	
	if(udp_connect(&pcb, "edu.ntp.org.cn", 123) == ERR_OK)
	{
		memset(time_buffer, 0, sizeof(time_buffer));
		
		time_buffer[0] = 0xE3;										//LI, Version, Mode
		time_buffer[1] = 0;											//表示本地时钟的层次水平
		time_buffer[2] = 6;											//八位signed integer，表示连续信息之间的最大间隔
		time_buffer[3] = 0xEB;										//表示本地时钟精度，精确到秒的平方级
		
		//udp_send(&pcb, 
	}
#endif
	
	return ntp_time_info.ntp_time_ok;

}
