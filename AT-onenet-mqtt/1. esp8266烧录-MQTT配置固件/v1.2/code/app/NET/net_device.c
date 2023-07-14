/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	net_device.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2017-07-20
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		ESP8266��������
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//OS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

//ESP8266ͨ���ļ�
#include "esp_common.h"

//������ͷ�ļ�
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "net_device.h"


/*----------------------------������-----------------------------------------*/
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
//	�������ƣ�	NET_DEVICE_GetMacs
//
//	�������ܣ�	��ȡ��Χ�ȵ��mac
//
//	��ڲ�����	��
//
//	���ز�����	0-�ɹ�	1-ʧ��
//
//	˵����		
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
//	�������ƣ�	NET_DEVICE_GetTime
//
//	�������ܣ�	��ȡ����ʱ��
//
//	��ڲ�����	��
//
//	���ز�����	UTC��ֵ
//
//	˵����		������NTPЭ���������
//				NTP��������UDPЭ�飬�˿�123
//
//				�Ѳ��Կ��õ�NTP������-2017-11-07
//				1.cn.pool.ntp.org		��Դ�����ϳ��ģ���֪��������(ע�⡰1.��������ţ���������һ����)
//				cn.ntp.org.cn			��Դ���й�
//				edu.ntp.org.cn			��Դ���й�������
//				tw.ntp.org.cn			��Դ���й�̨��
//				us.ntp.org.cn			��Դ������
//				sgp.ntp.org.cn			��Դ���¼���
//				kr.ntp.org.cn			��Դ������
//				de.ntp.org.cn			��Դ���¹�
//				jp.ntp.org.cn			��Դ���ձ�
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
//	�������ƣ�	NET_DEVICE_Exist
//
//	�������ܣ�	�����豸���ڼ��
//
//	��ڲ�����	��
//
//	���ز�����	���ؽ��
//
//	˵����		0-�ɹ�		1-ʧ��
//==========================================================
_Bool NET_DEVICE_Exist(void)
{

	_Bool status = 1;

	if(system_get_chip_id() != 0)		//��ȡоƬID
		status = 0;
	
	return status;

}

//==========================================================
//	�������ƣ�	NET_DEVICE_Init
//
//	�������ܣ�	��ʼ������
//
//	��ڲ�����	��
//
//	���ز�����	0-�ɹ�	1-ʧ��
//
//	˵����		
//==========================================================
_Bool NET_DEVICE_Init(void)
{
	
	

	return 0;

}

//==========================================================
//	�������ƣ�	NET_DEVICE_GetSocket
//
//	�������ܣ�	����socket�׽���
//
//	��ڲ�����	��
//
//	���ز�����	<0     ����socketʧ��
//          	>=0     socket������
//
//	˵����		����ֻ�Ǹ���һ������socket���ӷ���˵�����, ������ʽ���ѯ���socket api
//				���socket api:  
//					socket, gethostbyname, connect
//==========================================================
int NET_DEVICE_GetSocket(void)
{

	int sockfd;
	
	if(socket_num_used >= SOCKET_NUM)
		return -1;
	
	/* ����socket�׽��� */
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
//	�������ƣ�	NET_DEVICE_Connect
//
//	�������ܣ�	����socket�׽��ֲ����ӷ����
//
//	��ڲ�����	socket_fd��socket������
//				addr: ip��ַ
//				protno: �˿ں�
//
//	���ز�����	0-���ӳɹ�	1-����ʧ��
//
//	˵����		
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
	
    /* �ͻ��� ������TCP������������ */
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
//	�������ƣ�	NET_DEVICE_Close
//
//	�������ܣ�	�ر�socket�׽���
//
//	��ڲ�����	socket_fd��socket������
//
//	���ز�����	��֪��
//
//	˵����		
//==========================================================
int NET_DEVICE_Close(int *socket_fd)
{
	
	int result = -1;

//	printf("NET_DEVICE_Close: %d\r\n", *socket_fd);
	
	result = close(*socket_fd);	//�ر�����
	
	if(--socket_num_used > SOCKET_NUM)
		socket_num_used = 0;
	
	*socket_fd = -1;
	
	return result;

}

//==========================================================
//	�������ƣ�	NET_DEVICE_Send
//
//	�������ܣ�	��buffer�е�len�ֽ�����д��(����)socket������sockfd, �ɹ�ʱ����д��(���͵�)�ֽ���
//
//	��ڲ�����	socket_fd��socket������
//          	buffer: �跢�͵��ֽ�
//          	len: �跢�͵ĳ���
//
//	���ز�����	<=0     ����ʧ��
//          	>0      �ɹ����͵��ֽ���
//
//	˵����		����ֻ�Ǹ�����һ���������ݵ�����, ������ʽ���ѯ���socket api
//          	һ����˵, ���Ͷ���Ҫѭ������, ����Ϊ��Ҫ���͵��ֽ��� > socket��д������ʱ, һ��send�Ƿ��Ͳ����.
//				���socket api:  
//          		send
//==========================================================
int NET_DEVICE_SendData(int socket_fd, char* buffer, unsigned short len)
{

    int total  = 0;
    int n = 0;
	
    while (len != total)
    {
		//printf("%s[%d]%d,%d\n", __FUNCTION__, __LINE__,sockfd,len - total);
        /* ���ŷ���len - total���ֽڵ����� */
        //n = Send(sockfd,buffer + total,len - total,MSG_NOSIGNAL);
        n = send(socket_fd, buffer + total, len - total, MSG_PEEK);
		
        if (n <= 0)
        {
            UartPrintf("+Event:ERROR writing to socket: %d\n", n);
			
            return n;
        }
		
        /* �ɹ�������n���ֽڵ����� */
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
//	�������ƣ�	NET_DEVICE_GetData
//
//	�������ܣ�	��������
//
//	��ڲ�����	socket_fd��socket������
//				buf: ����
//				len: ����
//				flas: ��֪��
//
//	���ز�����	���յ����ֽ�����С��0��ʧ��
//
//	˵����		
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
//	�������ƣ�	NET_DEVICE_Check
//
//	�������ܣ�	wifi״̬���
//
//	��ڲ�����	��
//
//	���ز�����	����״̬
//
//	˵����		
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
//	�������ƣ�	wifi_handle_event_cb
//
//	�������ܣ�	wifi�¼��ص�����
//
//	��ڲ�����	evt: �¼�֪ͨ
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void wifi_handle_event_cb(System_Event_t *evt)
{

//	printf("event %x\n", evt->event_id);

	switch (evt->event_id)
	{
		case EVENT_STAMODE_CONNECTED:								//STAģʽ����·����

			/*
			printf("WIFI�¼�-STAģʽ-���ӳɹ�:\r\nconnect to ssid %s, channel %d\r\n",
			evt->event_info.connected.ssid,
			evt->event_info.connected.channel);
			*/
			
			UartPrintf("+Event:WIFI CONNECTED\r\n");
			
			net_device_info.connect_ap = 1;
			
		break;
		
		case EVENT_STAMODE_DISCONNECTED:							//STAģʽ�Ͽ�

			//evt->event_info.disconnected.reason
			//201:����SSID����
			//204:����PASSWORD����
			
			/*
			printf("WIFI�¼�-STAģʽ-�Ͽ�����:\r\ndisconnect from ssid %s, reason %d\r\n",
			evt->event_info.disconnected.ssid,
			evt->event_info.disconnected.reason);
			*/
			
			UartPrintf("+Event:WIFI DISCONNECT\r\n");

			net_device_info.err_count++;

			net_device_info.connect_ap = 0;
			net_device_info.get_ip = 0;
			net_device_info.net_work = 0;
			
		break;
		
		case EVENT_STAMODE_AUTHMODE_CHANGE:							//STAģʽ�ı�
			
			/*
			printf("WIFI�¼�-STAģʽ-ģʽ�ı�:\r\nmode: %d -> %d\r\n",
			evt->event_info.auth_change.old_mode,
			evt->event_info.auth_change.new_mode);
			*/
			
		break;
		
		case EVENT_STAMODE_GOT_IP:									//STAģʽ��ȡ��IP
			
			/*
			printf("WIFI�¼�-STAģʽ-�ѻ�ȡIP:\r\nip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR,
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
			printf("WIFI�¼�-:\r\nstation: " MACSTR "join, AID = %d\r\n",
			MAC2STR(evt->event_info.sta_connected.mac),
			evt->event_info.sta_connected.aid);
			*/
			
		break;
		
		case EVENT_SOFTAPMODE_STADISCONNECTED:
			
			/*
			printf("WIFI�¼�-:\r\nstation: " MACSTR "leave, AID = %d\r\n",
			MAC2STR(evt->event_info.sta_disconnected.mac),
			evt->event_info.sta_disconnected.aid);
			*/
			
		break;
		
		default:
			
		break;
	}
}

