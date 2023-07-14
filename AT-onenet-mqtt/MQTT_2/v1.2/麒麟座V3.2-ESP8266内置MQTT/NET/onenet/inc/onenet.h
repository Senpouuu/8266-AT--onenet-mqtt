#ifndef _ONENET_H_
#define _ONENET_H_


#include "dstream.h"


#define LBS_WIFI_EN				1			//1-获取WIFI信息		0-不获取


typedef struct
{

    char dev_id[16];
	char pro_id[16];
	char auth_info[32];
	char api_key[32];
	
	unsigned char send_data;
	
	unsigned char status;

} ONETNET_INFO;

extern ONETNET_INFO onenet_info;


typedef struct
{

	char lon[24];
	char lat[24];
	
	_Bool flag;

} GPS_INFO;

extern GPS_INFO gps;


#define ONENET_STATUS_OK		0			//在线
#define ONENET_STATUS_DIS		1			//未在线
#define ONENET_STATUS_AP		2			//未接入AP


#define SEND_TYPE_OK			(1 << 0)	//
#define SEND_TYPE_DATA			(1 << 1)	//
#define SEND_TYPE_HEART			(1 << 2)	//
#define SEND_TYPE_PUBLISH		(1 << 3)	//
#define SEND_TYPE_SUBSCRIBE		(1 << 4)	//
#define SEND_TYPE_UNSUBSCRIBE	(1 << 5)	//
#define SEND_TYPE_BINFILE		(1 << 6)	//


void OneNET_DevLink(const char* devid, const char *proid, const char* auth_info);

unsigned char OneNET_SendData(DATA_STREAM *streamArray, unsigned short streamArrayCnt);

unsigned char OneNET_Subscribe(const char *topics[], unsigned char topic_cnt);

unsigned char OneNET_UnSubscribe(const char *topics[], unsigned char topic_cnt);

unsigned char OneNET_Publish(const char *topic, const char *msg);

void OneNET_CmdHandle(void);

unsigned char OneNET_GetLocation(const char *api_key, char *lon, char *lat);

unsigned int OneNET_CheckStatus(void);

#endif
