#ifndef _ONENET_H_
#define _ONENET_H_


#include "dstream.h"


typedef struct
{

    char dev_id[16];
    char api_key[32];					//如果要采用自动创建设备的方式，apikey必须为master-key
	
	char pro_id[10];
	char auth_info[32];
	
	char reg_code[20];					//正式环境注册码
	
	char ip[16];
	short port;

	const unsigned char protocol;		//协议类型号		1-edp	2-nwx	3-jtext		4-Hiscmd
										//				5-jt808			6-modbus	7-mqtt
										//				8-gr20			9-reg		10-HTTP(自定义)
	
	unsigned char send_data;
	
	unsigned short net_work : 1;		//1-OneNET接入成功		0-OneNET接入失败
	unsigned short err_count : 3;		//错误计数
	unsigned short heart_beat : 1;		//心跳
	unsigned short reinit_count : 2;
	unsigned short connect_ip : 1;		//
	unsigned short send_ok : 1;			
	unsigned short send_count : 3;		//发送计数
	unsigned short reconnect : 1;		//
	unsigned short reverse : 3;

} ONETNET_INFO;

extern ONETNET_INFO onenet_info;


typedef struct
{

	char lon[24];
	char lat[24];
	
	_Bool flag;

} GPS_INFO;

extern GPS_INFO gps_info;


#define SEND_TYPE_OK			(1 << 0)	//
#define SEND_TYPE_DATA			(1 << 1)	//
#define SEND_TYPE_HEART			(1 << 2)	//
#define SEND_TYPE_PUBLISH		(1 << 3)	//
#define SEND_TYPE_SUBSCRIBE		(1 << 4)	//
#define SEND_TYPE_UNSUBSCRIBE	(1 << 5)	//
#define SEND_TYPE_GPS			(1 << 6)	//


_Bool OneNET_RepetitionCreateFlag(const char *apikey);

_Bool OneNET_CreateDevice(const char *reg_code, const char *dev_name, const char *auth_info, char *devid, char *apikey);

unsigned char OneNET_GetLocation(const char *device_id, const char *api_key, char *lon, char *lat);

_Bool OneNET_ConnectIP(const unsigned char *addr, short port);

void OneNET_DevLink(const char* devid, const char *proid, const char* auth_info);

_Bool OneNET_DisConnect(void);

unsigned char OneNET_SendData(FORMAT_TYPE type, char *devid, char *apikey, DATA_STREAM *streamArray, unsigned short streamArrayCnt);

unsigned char OneNET_Subscribe(const signed char *topics);

unsigned char OneNET_UnSubscribe(const signed char *topics);

unsigned char OneNET_Publish(const char *topic, const char *msg);

unsigned char OneNET_SendData_Heart(void);

_Bool OneNET_Check_Heart(void);

void OneNET_CmdHandle(void);

void OneNET_RevPro(unsigned char *dataPtr);

#endif
