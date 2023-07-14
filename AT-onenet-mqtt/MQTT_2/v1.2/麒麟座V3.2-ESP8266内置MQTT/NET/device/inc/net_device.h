#ifndef _NET_DEVICE_H_
#define _NET_DEVICE_H_


//=============================����==============================
//===========�����ṩRTOS���ڴ��������Ҳ����ʹ��C���=========
#include <rtthread.h>

#define NET_MallocBuffer	rt_malloc

#define NET_FreeBuffer		rt_free
//==========================================================


#define PHONE_AP_MODE			0			//1-ʹ���ֻ��ȵ�ģʽ������Χ��·��ʱ��AirKiss�޷�ʹ�ã���ʹ�ù̶���ʽ����
											//0-ʹ��AirKiss�ķ�ʽ��


#define NET_TIME_EN				1			//1-��ȡ����ʱ��		0-����ȡ


struct NET_SEND_LIST
{

	unsigned short dataLen;					//���ݳ���
	unsigned char *buf;						//����ָ��
	
	struct NET_SEND_LIST *next;				//��һ��

};


typedef struct
{
	
	char *cmd_resp;							//����ظ�ָ�롣�����ȡ����ص����ݣ�������ȡ��ָ���ڵ�����
	char *cmd_ipd;							//�ڽ���onenetǰͨ�������ȡ�����ݣ���������ʱ�䡢��ȡ����ip��
	char *cmd_hdl;							//������ڷ�������󣬻��ڷ��������������û�ָ���ķ�������
	
	unsigned char init_step;
	
	unsigned int network_delay_time;
	
	unsigned int net_time;

} NET_DEVICE_INFO;

extern NET_DEVICE_INFO net_device_info;


#define NET_DEVICE_INITIAL			0		//��ʼ��״̬
#define NET_DEVICE_CONNECTING		1		//������
#define NET_DEVICE_PSWD_ERR			2		//�������
#define NET_DEVICE_NO_AP			3		//��AP
#define NET_DEVICE_CONNCET_ERR		4		//����ʧ��
#define NET_DEVICE_GOT_IP			5		//��ȡIP
#define NET_DEVICE_NO_DEVICE		254		//
#define NET_DEVICE_NO_ERR			255	 	//�޴���


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
