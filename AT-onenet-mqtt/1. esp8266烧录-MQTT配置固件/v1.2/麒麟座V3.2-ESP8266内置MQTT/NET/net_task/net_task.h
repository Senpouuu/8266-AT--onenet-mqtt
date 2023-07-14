#ifndef _NET_TASK_H_
#define _NET_TASK_H_





typedef enum
{

	NET_EVENT_Timer_Check_Err = 0,			//���綨ʱ��鳬ʱ����
	NET_EVENT_Timer_Send_Err,				//���緢��ʧ�ܴ���
	
	NET_EVENT_Send_HeartBeat,				//��������������
	NET_EVENT_Send_Data,					//�����������ݵ�
	NET_EVENT_Send_Subscribe,				//�������Ͷ�������
	NET_EVENT_Send_UnSubscribe,				//��������ȡ����������
	NET_EVENT_Send_Publish,					//����������������
	
	NET_EVENT_Send,							//��ʼ��������
	NET_EVENT_Recv,							//Modbus��-�յ����ݲ�ѯָ��
	
	NET_EVENT_Check_Status,					//��������ģ��״̬���
	
	NET_EVENT_Device_Ok,					//����ģ����Ok
	NET_EVENT_Device_Err,					//����ģ�������
	
	NET_EVENT_Initialize,					//���ڳ�ʼ������ģ��
	NET_EVENT_Init_Ok,						//����ģ���ʼ���ɹ�
	
	NET_EVENT_Auto_Create_Ok,				//�Զ������豸�ɹ�
	NET_EVENT_Auto_Create_Err,				//�Զ������豸ʧ��
	
	NET_EVENT_Connect,						//�������ӡ���¼OneNET
	NET_EVENT_Connect_Ok,					//���ӡ���¼�ɹ�
	NET_EVENT_Connect_Err,					//���ӡ���¼����
	
	NET_EVENT_Fault_Process,				//���������

} NET_EVENT;


__weak void NET_Event_CallBack(NET_EVENT net_event);

void NET_Task_Init(void);


#endif
