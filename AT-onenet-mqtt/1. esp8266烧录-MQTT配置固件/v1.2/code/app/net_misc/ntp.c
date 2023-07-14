/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	ntp.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2018-01-22
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		��ȡNTP����������
	*
	*	�޸ļ�¼��	
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

//C��
#include <string.h>
#include <time.h>


NTP_TIME_INFO ntp_time_info = {0, 0};


//==========================================================
//	�������ƣ�	NTP_ResetStatus
//
//	�������ܣ�	����״̬��־
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void NTP_ResetStatus(void)
{

	ntp_time_info.ntp_time = 0;
	ntp_time_info.ntp_time_ok = 0;

}

//==========================================================
//	�������ƣ�	NTP_GetTime
//
//	�������ܣ�	��ȡ����ʱ��
//
//	��ڲ�����	��
//
//	���ز�����	1-�ɹ�	0-ʧ��
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
	
	if(!net_device_info.net_work)									//����ģ��߱�������������
		return ntp_time_info.ntp_time_ok;
	
	if(udp_connect(&pcb, "edu.ntp.org.cn", 123) == ERR_OK)
	{
		memset(time_buffer, 0, sizeof(time_buffer));
		
		time_buffer[0] = 0xE3;										//LI, Version, Mode
		time_buffer[1] = 0;											//��ʾ����ʱ�ӵĲ��ˮƽ
		time_buffer[2] = 6;											//��λsigned integer����ʾ������Ϣ֮��������
		time_buffer[3] = 0xEB;										//��ʾ����ʱ�Ӿ��ȣ���ȷ�����ƽ����
		
		//udp_send(&pcb, 
	}
#endif
	
	return ntp_time_info.ntp_time_ok;

}
