#ifndef _NET_DEVICE_H_
#define _NET_DEVICE_H_


//=============================配置==============================
//===========可以提供RTOS的内存管理方案，也可以使用C库的=========
#include <rtthread.h>

#define NET_MallocBuffer	rt_malloc

#define NET_FreeBuffer		rt_free
//==========================================================


#define PHONE_AP_MODE			0			//1-使用手机热点模式。当周围无路由时，AirKiss无法使用，则使用固定方式配置
											//0-使用AirKiss的方式配


#define NET_TIME_EN				1			//1-获取网络时间		0-不获取


struct NET_SEND_LIST
{

	unsigned short dataLen;					//数据长度
	unsigned char *buf;						//数据指针
	
	struct NET_SEND_LIST *next;				//下一个

};


typedef struct
{
	
	char *cmd_resp;							//命令回复指针。比如获取命令返回的数据，可以提取此指针内的数据
	char *cmd_ipd;							//在接入onenet前通过网络获取的数据，比如网络时间、获取接入ip等
	char *cmd_hdl;							//命令处理。在发送命令后，会在返回内容里搜索用户指定的返回数据
	
	unsigned char init_step;
	
	unsigned int network_delay_time;
	
	unsigned int net_time;

} NET_DEVICE_INFO;

extern NET_DEVICE_INFO net_device_info;


#define NET_DEVICE_INITIAL			0		//初始化状态
#define NET_DEVICE_CONNECTING		1		//连接中
#define NET_DEVICE_PSWD_ERR			2		//密码错误
#define NET_DEVICE_NO_AP			3		//无AP
#define NET_DEVICE_CONNCET_ERR		4		//连接失败
#define NET_DEVICE_GOT_IP			5		//获取IP
#define NET_DEVICE_NO_DEVICE		254		//
#define NET_DEVICE_NO_ERR			255	 	//无错误


void NET_DEVICE_IO_Init(void);

void NET_DEVICE_PowerCtl(_Bool flag);

_Bool NET_DEVICE_Exist(void);

_Bool NET_DEVICE_Init(void);

void NET_DEVICE_Reset(void);

void NET_DEVICE_CmdHandle(char *cmd);

_Bool NET_DEVICE_SendCmd(char *cmd, char *res, unsigned short time_out);

unsigned char *NET_DEVICE_Read(void);

unsigned char NET_DEVICE_Check(void);

unsigned int NET_DEVICE_GetTime(void);

unsigned short NET_DEVICE_GetNWD(void);

#endif
