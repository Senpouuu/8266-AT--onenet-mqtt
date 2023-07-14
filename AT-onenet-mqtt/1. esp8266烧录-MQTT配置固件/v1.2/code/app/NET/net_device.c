/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	net_device.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-07-20
	*
	*	版本： 		V1.0
	*
	*	说明： 		ESP8266网络驱动
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//OS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

//ESP8266通用文件
#include "esp_common.h"

//驱动层头文件
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "net_device.h"


/*----------------------------错误码-----------------------------------------*/
#define ERR_CREATE_SOCKET	-1 
#define ERR_HOSTBYNAME		-2 
#define ERR_CONNECT			-3 
#define ERR_SEND			-4
#define ERR_TIMEOUT0		-5
#define ERR_RECV			-6



NET_DEVICE_INFO net_device_info = {-1, 0, 0, 0, 0, 0, 0, 0};


LBS_WIFI_INFO lbs_wifi_info;


typedef struct
{

	int socket_id[SOCKET_NUM];
	
	unsigned int network_delay_ticks[SOCKET_NUM];

} NETWORK_DELAY;

static NETWORK_DELAY network_delay = {{-1, -1, -1, -1, -1}, {0, 0, 0, 0, 0}};


static unsigned char socket_num_used = 0;


void ICACHE_FLASH_ATTR scan_done(void *arg, STATUS status)
{

	if(status == OK)
	{
		struct bss_info *bss_link = (struct bss_info *)arg;
		
		unsigned char count = 0;
		unsigned char mac_t[24];
		
		memset(lbs_wifi_info.mac, 0, sizeof(lbs_wifi_info.mac));

		while(bss_link != NULL)
		{
			snprintf(mac_t, sizeof(mac_t), ""MACSTR"", MAC2STR(bss_link->bssid));
			strncat(lbs_wifi_info.mac, mac_t, strlen(mac_t));
			
			if(++count >= 2)
				break;
			
			strncat(lbs_wifi_info.mac, "|", 1);

			bss_link = bss_link->next.stqe_next;

		}
		
		lbs_wifi_info.lbs_wifi_ok = 1;
		
		//UartPrintf("Mac: %s\r\n", lbs_wifi_info.mac);
	}
	else
	{
		printf("Mac: Err\r\n");
	}

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
unsigned char NET_DEVICE_GetMacs(void)
{

	//http://www.mcublog.cn/esp8266/2019_11/stationmoshi/
	if(wifi_station_scan(0, scan_done) == 1)
	{
		
	}
	
	return 0;

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
//				tw.ntp.org.cn			来源：中国台湾
//				us.ntp.org.cn			来源：美国
//				sgp.ntp.org.cn			来源：新加坡
//				kr.ntp.org.cn			来源：韩国
//				de.ntp.org.cn			来源：德国
//				jp.ntp.org.cn			来源：日本
//==========================================================
unsigned int NET_DEVICE_GetTime(void)
{
	
	static _Bool flag = 1;
	unsigned char time_out = 200;
	
	unsigned int net_time = 0;
	
	if(net_device_info.net_work == 1)
	{
		if(flag)
		{
			flag = 0;
			
			sntp_setservername(0, "183.230.40.42");
			sntp_setservername(1, "edu.ntp.org.cn");
			sntp_setservername(2, "cn.ntp.org.cn");

			sntp_init();
		}
		
		while(--time_out)
		{
			net_time = sntp_get_current_timestamp();
			if(net_time)
				break;
			else
			{
				vTaskDelay(499);
			}
			
			vTaskDelay(1);
		}
		
		sntp_stop();
	}
	
	return net_time;

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

	_Bool status = 1;

	if(system_get_chip_id() != 0)		//读取芯片ID
		status = 0;
	
	return status;

}

//==========================================================
//	函数名称：	NET_DEVICE_Init
//
//	函数功能：	初始化网络
//
//	入口参数：	无
//
//	返回参数：	0-成功	1-失败
//
//	说明：		
//==========================================================
_Bool NET_DEVICE_Init(void)
{
	
	

	return 0;

}

//==========================================================
//	函数名称：	NET_DEVICE_GetSocket
//
//	函数功能：	创建socket套接字
//
//	入口参数：	无
//
//	返回参数：	<0     创建socket失败
//          	>=0     socket描述符
//
//	说明：		这里只是给出一个创建socket连接服务端的例子, 其他方式请查询相关socket api
//				相关socket api:  
//					socket, gethostbyname, connect
//==========================================================
int NET_DEVICE_GetSocket(void)
{

	int sockfd;
	
	if(socket_num_used >= SOCKET_NUM)
		return -1;
	
	/* 创建socket套接字 */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
//	printf("Open sockfd = %d\n", sockfd);
    if (sockfd < 0)
	{
        UartPrintf("+Event:ERROR opening socket\n");
        return ERR_CREATE_SOCKET; 
    }
	
	socket_num_used++;
	
	return sockfd;

}

//==========================================================
//	函数名称：	NET_DEVICE_Connect
//
//	函数功能：	创建socket套接字并连接服务端
//
//	入口参数：	socket_fd：socket描述符
//				addr: ip地址
//				protno: 端口号
//
//	返回参数：	0-连接成功	1-连接失败
//
//	说明：		
//==========================================================
_Bool NET_DEVICE_Connect(int socket_fd, const unsigned char *addr, short port)
{

    struct sockaddr_in serv_addr;
	_Bool result = 1;
	
	if(socket_fd < 0)
	{
		UartPrintf("+Event:socket fd err: %d\r\n", socket_fd);
		
		return result;
	}

//	printf("ipaddr:%s:%d\n", addr, port);
	
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    
	serv_addr.sin_addr.s_addr = inet_addr(addr);

    serv_addr.sin_port = htons(port);
	
    /* 客户端 建立与TCP服务器的连接 */
    if(connect(socket_fd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    {
        UartPrintf("+Event:ERROR connecting\n");
        return result;
    }

	result = 0;
 //   UartPrintf("connect to server %s:%d succ!...\n", addr, port);

    return result;

}

//==========================================================
//	函数名称：	NET_DEVICE_Close
//
//	函数功能：	关闭socket套接字
//
//	入口参数：	socket_fd：socket描述符
//
//	返回参数：	不知道
//
//	说明：		
//==========================================================
int NET_DEVICE_Close(int *socket_fd)
{
	
	int result = -1;

//	printf("NET_DEVICE_Close: %d\r\n", *socket_fd);
	
	result = close(*socket_fd);	//关闭连接
	
	if(--socket_num_used > SOCKET_NUM)
		socket_num_used = 0;
	
	*socket_fd = -1;
	
	return result;

}

//==========================================================
//	函数名称：	NET_DEVICE_Send
//
//	函数功能：	将buffer中的len字节内容写入(发送)socket描述符sockfd, 成功时返回写的(发送的)字节数
//
//	入口参数：	socket_fd：socket描述符
//          	buffer: 需发送的字节
//          	len: 需发送的长度
//
//	返回参数：	<=0     发送失败
//          	>0      成功发送的字节数
//
//	说明：		这里只是给出了一个发送数据的例子, 其他方式请查询相关socket api
//          	一般来说, 发送都需要循环发送, 是因为需要发送的字节数 > socket的写缓存区时, 一次send是发送不完的.
//				相关socket api:  
//          		send
//==========================================================
int NET_DEVICE_SendData(int socket_fd, char* buffer, unsigned short len)
{

    int total  = 0;
    int n = 0;
	
    while (len != total)
    {
		//printf("%s[%d]%d,%d\n", __FUNCTION__, __LINE__,sockfd,len - total);
        /* 试着发送len - total个字节的数据 */
        //n = Send(sockfd,buffer + total,len - total,MSG_NOSIGNAL);
        n = send(socket_fd, buffer + total, len - total, MSG_PEEK);
		
        if (n <= 0)
        {
            UartPrintf("+Event:ERROR writing to socket: %d\n", n);
			
            return n;
        }
		
        /* 成功发送了n个字节的数据 */
        total += n;
    }
    /* wululu test print send bytes */
	
	for(n = 0; n < SOCKET_NUM; n++)
	{
		if(network_delay.socket_id[n] == -1)
		{
			network_delay.socket_id[n] = socket_fd;
			network_delay.network_delay_ticks[n] = xTaskGetTickCount();
			
			break;
		}
	}
    
    return total;
	
}

//==========================================================
//	函数名称：	NET_DEVICE_GetData
//
//	函数功能：	接收数据
//
//	入口参数：	socket_fd：socket描述符
//				buf: 缓存
//				len: 长度
//				flas: 不知道
//
//	返回参数：	接收到的字节数。小于0则失败
//
//	说明：		
//==========================================================
int NET_DEVICE_GetData(int socket_fd, void *buf, unsigned int len, int flags)
{

	int recv_bytes = lwip_recv(socket_fd, buf, len, flags);
	
	if(recv_bytes > 0)
	{
		unsigned char i = 0;
		
		for(; i < SOCKET_NUM; i++)
		{
			if((network_delay.socket_id[i] != -1) && (network_delay.socket_id[i] == socket_fd))
			{
				net_device_info.network_delay_time = (xTaskGetTickCount() - network_delay.network_delay_ticks[i]) * (1000 / configTICK_RATE_HZ);
				network_delay.socket_id[i] = -1;
				
				break;
			}
		}
	}

	return recv_bytes;
	
}


//==========================================================
//	函数名称：	NET_DEVICE_Check
//
//	函数功能：	wifi状态检查
//
//	入口参数：	无
//
//	返回参数：	返回状态
//
//	说明：		
//==========================================================
unsigned char NET_DEVICE_Check(void)
{

	STATION_STATUS status = NET_DEVICE_NO_ERR;

	switch(wifi_station_get_connect_status())
	{
		case STATION_IDLE:        /**< ESP8266 station idle */

			//printf("NET_DEVICE_Check--STATION_IDLE\r\n");
			status = NET_DEVICE_INITIAL;

		break;
		
    	case STATION_CONNECTING:      /**< ESP8266 station is connecting to AP*/

			//printf("NET_DEVICE_Check--STATION_CONNECTING\r\n");
			status = NET_DEVICE_CONNECTING;

		break;
		
    	case STATION_WRONG_PASSWORD:  /**< the password is wrong*/

			//printf("NET_DEVICE_Check--STATION_WRONG_PASSWORD\r\n");
			status = NET_DEVICE_PSWD_ERR;

		break;
		
    	case STATION_NO_AP_FOUND:     /**< ESP8266 station can not find the target AP*/

			//printf("NET_DEVICE_Check--STATION_NO_AP_FOUND\r\n");
			status = NET_DEVICE_NO_AP;

		break;
		
    	case STATION_CONNECT_FAIL:    /**< ESP8266 station fail to connect to AP*/

			//printf("NET_DEVICE_Check--STATION_CONNECT_FAIL\r\n");
			status = NET_DEVICE_CLOSED;

		break;
		
    	case STATION_GOT_IP:           /**< ESP8266 station got IP address from AP*/

			//printf("NET_DEVICE_Check--STATION_GOT_IP\r\n");
			status = NET_DEVICE_GOT_IP;

		break;
	}

	return status;

}

//==========================================================
//	函数名称：	wifi_handle_event_cb
//
//	函数功能：	wifi事件回调函数
//
//	入口参数：	evt: 事件通知
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void wifi_handle_event_cb(System_Event_t *evt)
{

//	printf("event %x\n", evt->event_id);

	switch (evt->event_id)
	{
		case EVENT_STAMODE_CONNECTED:								//STA模式接入路由器

			/*
			printf("WIFI事件-STA模式-连接成功:\r\nconnect to ssid %s, channel %d\r\n",
			evt->event_info.connected.ssid,
			evt->event_info.connected.channel);
			*/
			
			UartPrintf("+Event:WIFI CONNECTED\r\n");
			
			net_device_info.connect_ap = 1;
			
		break;
		
		case EVENT_STAMODE_DISCONNECTED:							//STA模式断开

			//evt->event_info.disconnected.reason
			//201:代表SSID错误
			//204:代表PASSWORD错误
			
			/*
			printf("WIFI事件-STA模式-断开连接:\r\ndisconnect from ssid %s, reason %d\r\n",
			evt->event_info.disconnected.ssid,
			evt->event_info.disconnected.reason);
			*/
			
			UartPrintf("+Event:WIFI DISCONNECT\r\n");

			net_device_info.err_count++;

			net_device_info.connect_ap = 0;
			net_device_info.get_ip = 0;
			net_device_info.net_work = 0;
			
		break;
		
		case EVENT_STAMODE_AUTHMODE_CHANGE:							//STA模式改变
			
			/*
			printf("WIFI事件-STA模式-模式改变:\r\nmode: %d -> %d\r\n",
			evt->event_info.auth_change.old_mode,
			evt->event_info.auth_change.new_mode);
			*/
			
		break;
		
		case EVENT_STAMODE_GOT_IP:									//STA模式获取到IP
			
			/*
			printf("WIFI事件-STA模式-已获取IP:\r\nip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR,
			IP2STR(&evt->event_info.got_ip.ip),
			IP2STR(&evt->event_info.got_ip.mask),
			IP2STR(&evt->event_info.got_ip.gw));
			printf("\r\n");
			*/
			
			UartPrintf("+Event:WIFI GOT IP\r\n");

			net_device_info.get_ip = 1;
			net_device_info.net_work = 1;
			
			NET_DEVICE_GetMacs();
			
		break;
		
		case EVENT_SOFTAPMODE_STACONNECTED:
			
			/*
			printf("WIFI事件-:\r\nstation: " MACSTR "join, AID = %d\r\n",
			MAC2STR(evt->event_info.sta_connected.mac),
			evt->event_info.sta_connected.aid);
			*/
			
		break;
		
		case EVENT_SOFTAPMODE_STADISCONNECTED:
			
			/*
			printf("WIFI事件-:\r\nstation: " MACSTR "leave, AID = %d\r\n",
			MAC2STR(evt->event_info.sta_disconnected.mac),
			evt->event_info.sta_disconnected.aid);
			*/
			
		break;
		
		default:
			
		break;
	}
}

