#ifndef _NET_DEVICE_H_
#define _NET_DEVICE_H_





#define SOCKET_NUM				5


typedef struct
{

	int socket_fd;
	
	unsigned short network_delay_time;

	unsigned char init_step : 2;
	unsigned char connect_ap : 1;		//1-已接入路由		0-未接入路由
	unsigned char get_ip : 1;			//1-已获取到IP		0-未获取到IP
	unsigned char net_work : 1;			//1-具备网络能力	0-不具备网络能力
	unsigned char err_count : 2;
	unsigned char reverse : 1;

} NET_DEVICE_INFO;

extern NET_DEVICE_INFO net_device_info;


typedef struct
{

	char mac[48];							//mac
	
	_Bool lbs_wifi_ok;

} LBS_WIFI_INFO;

extern LBS_WIFI_INFO lbs_wifi_info;


#define NET_DEVICE_CONNECTED	0	//已连接
#define NET_DEVICE_CONNECTING	1	//连接中
#define NET_DEVICE_CLOSED		2	//已断开
#define NET_DEVICE_GOT_IP		3	//已获取到IP
#define NET_DEVICE_NO_DEVICE	4	//无设备
#define NET_DEVICE_INITIAL		5	//初始化状态
#define NET_DEVICE_PSWD_ERR		6	//密码错误
#define NET_DEVICE_NO_AP		7	//无热点
#define NET_DEVICE_BUSY			254	//忙
#define NET_DEVICE_NO_ERR		255 //无错误


unsigned char NET_DEVICE_GetMacs(void);

unsigned int NET_DEVICE_GetTime(void);

_Bool NET_DEVICE_Exist(void);

_Bool NET_DEVICE_Init(void);

int NET_DEVICE_GetSocket(void);

_Bool NET_DEVICE_Connect(int socket_fd, const unsigned char *addr, short port);

int NET_DEVICE_Close(int *socket_fd);

int NET_DEVICE_SendData(int socket_fd, char* buffer, unsigned short len);

int NET_DEVICE_GetData(int socket_fd, void *buf, unsigned int len, int flags);

unsigned char NET_DEVICE_Check(void);


#endif

