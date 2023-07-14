/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	fault.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2016-11-23
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		���������
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//Э��
#include "onenet.h"
#include "fault.h"

//�����豸
#include "net_device.h"

//Ӳ������



NET_FAULT_INFO net_fault_info;


//==========================================================
//	�������ƣ�	Fault_Process
//
//	�������ܣ�	������
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void NET_Fault_Process(void)
{
	
	if(net_fault_info.net_fault_flag)
	{
		net_fault_info.net_fault_flag = 0;
		net_fault_info.net_fault_count++;
	}

	switch(net_fault_info.net_fault_level)
	{
		case NET_FAULT_LEVEL_0:
		
//			printf("WARN:	NET_FAULT_LEVEL_0\r\n");
		
		break;
		
		case NET_FAULT_LEVEL_1:											//����ȼ�1-��������IP
		
//			printf("WARN:	NET_FAULT_LEVEL_1\r\n");
			
			NET_DEVICE_Close(&net_device_info.socket_fd);
			
			onenet_info.net_work = 0;
			onenet_info.connect_ip = 0;
			
			if(++onenet_info.reinit_count >= 5)
			{
				onenet_info.reinit_count = 0;
				net_fault_info.net_fault_level = NET_FAULT_LEVEL_3;		//����ȼ�3
			}
			else
				net_fault_info.net_fault_level = NET_FAULT_LEVEL_0;		//������ȼ��������
		
		break;
		
		case NET_FAULT_LEVEL_2:											//����ȼ�2-��������ע������
		
//			printf("WARN:	NET_FAULT_LEVEL_2\r\n");
			
			NET_DEVICE_Close(&net_device_info.socket_fd);
			
			net_device_info.net_work = 0;
			net_device_info.init_step = 0;
			
			onenet_info.net_work = 0;
			onenet_info.connect_ip = 0;
			
			net_fault_info.net_fault_level = NET_FAULT_LEVEL_0;			//������ȼ��������
		
		break;
		
		case NET_FAULT_LEVEL_3:											//����ȼ�3-����λ�����豸
		
//			printf("WARN:	NET_FAULT_LEVEL_3\r\n");
			
			
			
			net_fault_info.net_fault_level = NET_FAULT_LEVEL_2;			//������ȼ�����
		
		break;
		
		case NET_FAULT_LEVEL_4:											//����ȼ�4-����������豸�����ϵ�
		
//			printf("WARN:	NET_FAULT_LEVEL_4\r\n");
		
			//power off
			
			//power on
		
			net_fault_info.net_fault_level = NET_FAULT_LEVEL_3;			//������ȼ�����
		
		break;
		
		case NET_FAULT_LEVEL_5:
		
//			printf("WARN:	NET_FAULT_LEVEL_5\r\n");
		
			net_fault_info.net_fault_level = NET_FAULT_LEVEL_0;
		
		break;
		
		default:
		break;
	}

}
