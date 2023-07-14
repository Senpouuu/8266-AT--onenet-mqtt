#ifndef _NTP_H_
#define _NTP_H_





#define NTP_TIME_EN				0	//1-获取网络时间		0-不获取


typedef struct
{

	_Bool ntp_time_ok;
	
	unsigned int ntp_time;

} NTP_TIME_INFO;

extern NTP_TIME_INFO ntp_time_info;


void NTP_ResetStatus(void);

_Bool NTP_GetTime(void);


#endif
