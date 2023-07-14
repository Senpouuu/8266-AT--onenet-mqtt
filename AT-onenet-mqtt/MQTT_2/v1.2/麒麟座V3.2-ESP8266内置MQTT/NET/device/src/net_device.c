/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	net_device.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2019-12-20
	*
	*	�汾�� 		V1.6
	*
	*	˵���� 		�����豸Ӧ�ò�
	*
	*	�޸ļ�¼��	V1.1��1.ƽ̨IP��PORTͨ����������ķ�ʽȷ��������˲�ͬЭ�������豸������ͨ�õ����⡣
	*					  2.ȡ�����ֶ��������磬�ϵ�ȴ�wifiģ���Զ����ӣ������ɹ���ʹ��OneNET���ںŽ���������
	*					  3.NET_DEVICE_SendCmd����������mode���������Ƿ������������ķ���ֵ��
	*				V1.2��1.ȡ��V1.1�еĹ���3��
	*					  2.���������ݻ�ȡ�ӿڣ�����rb���ơ�
	*					  3.ȡ����͸��ģʽ��
	*				V1.3��1.���ӡ�æ����־�жϡ�
	*					  2.����ģ�鷢��ʱ����������ģ�����ܶ�����
	*					  3.�޸��� NET_DEVICE_Check �������ơ�
	*				V1.4��1.���������ѡ������ͷ���β��
	*				V1.5��1.�޸� NET_DEVICE_AddDataSendList ������ӵ�����ͷ���Ұ�ڴ����⡣
	*				V1.6��1.�޸� NET_DEVICE_Connect ��������֤���ӵ��ȶ��ԡ�
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"

//��Ƭ��������
#include "mcu_gpio.h"

//�����豸
#include "net_device.h"
#include "net_io.h"

//Ӳ������
#include "delay.h"
#include "usart.h"

//C��
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


static _Bool nd_busy = 0;


NET_DEVICE_INFO net_device_info = {NULL, NULL, NULL, 0, 0, 0};


//Ϊ��ͨ���ԣ�gpio�豸�б����name�̶���ôд
const static GPIO_LIST net_device_gpio_list[] = {
													{GPIOC, GPIO_Pin_14, "nd_rst"},
												};


static _Bool cmd_resp_err_flag = 0;				//1-��������м�������ERROR��


/*
************************************************************
*	�������ƣ�	NET_DEVCIE_IsBusReady
*
*	�������ܣ�	��ѯ�����Ƿ����
*
*	��ڲ�����	
*
*	���ز�����	0-����	1-δ����
*
*	˵����		
************************************************************
*/
__inline static _Bool NET_DEVCIE_IsBusReady(void)
{
	
	_Bool result = 1;

	RTOS_ENTER_CRITICAL();
	
	if(nd_busy == 0)
	{
		nd_busy = 1;
		
		result = 0;
	}
	
	RTOS_EXIT_CRITICAL();
	
	return result;

}

/*
************************************************************
*	�������ƣ�	NET_DEVCIE_FreeBus
*
*	�������ܣ�	�ͷ�����
*
*	��ڲ�����	
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
__inline static void NET_DEVCIE_FreeBus(void)
{

	RTOS_ENTER_CRITICAL();
	
	nd_busy = 0;
	
	RTOS_EXIT_CRITICAL();

}

/*
************************************************************
*	�������ƣ�	NET_DEVICE_Delay
*
*	�������ܣ�	��ʱ
*
*	��ڲ�����	time����ʱʱ��
*
*	���ز�����	��
*
*	˵����		���ڵ�ǰ��ʱʱ��
************************************************************
*/
__inline static void NET_DEVICE_Delay(unsigned int time)
{

	RTOS_TimeDly(time);

}

//==========================================================
//	�������ƣ�	NET_DEVICE_PowerCtl
//
//	�������ܣ�	�����豸��Դ����
//
//	��ڲ�����	flag��1-�򿪵�Դ		0-�ر�
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void NET_DEVICE_PowerCtl(_Bool flag)
{

	

}

//==========================================================
//	�������ƣ�	NET_DEVICE_IO_Init
//
//	�������ܣ�	��ʼ�������豸IO��
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		��ʼ�������豸�Ŀ������š������շ����ܵ�
//==========================================================
void NET_DEVICE_IO_Init(void)
{
	
	MCU_GPIO_Init(net_device_gpio_list[0].gpio_group, net_device_gpio_list[0].gpio_pin, GPIO_Mode_Out_PP, GPIO_Speed_50MHz, net_device_gpio_list[0].gpio_name);
	
	NET_IO_Init();											//�����豸����IO���ʼ��
	
	NET_DEVICE_PowerCtl(1);

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
//	˵����		
//==========================================================
unsigned int NET_DEVICE_GetTime(void)
{
	
	net_device_info.net_time = 0;

#if(NET_TIME_EN == 1)
	
	UsartPrintf(USART_DEBUG, "Tips:	Wait for NET Time...\r\n");
	
	if(NET_DEVICE_SendCmd("AT+NTP\r\n", "+Event:", 1200) == 0)
	{
		char time_str[24];
		
		if(sscanf(net_device_info.cmd_resp, "+Event:%[^,],%d", time_str, &net_device_info.net_time) == 2)
		{
			UsartPrintf(USART_DEBUG, "Time: %s, %d\r\n", time_str, net_device_info.net_time);
		}
	}
			
#endif
	
	return net_device_info.net_time;

}

//==========================================================
//	�������ƣ�	NET_DEVICE_GetNWD
//
//	�������ܣ�	��ȡ�����ӳ�
//
//	��ڲ�����	��
//
//	���ز�����	���ؽ��
//
//	˵����		
//==========================================================
unsigned short NET_DEVICE_GetNWD(void)
{

	if(NET_DEVICE_SendCmd("AT+IOTNWD\r\n", "+Event:", 200) == 0)
	{
		if(sscanf(net_device_info.cmd_resp, "+Event:network delay(ms): %d", &net_device_info.network_delay_time) == 1)
		{
			UsartPrintf(USART_DEBUG, "network delay(ms): %d\r\n", net_device_info.network_delay_time);
		}
	}
	
	return net_device_info.network_delay_time;

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

	unsigned char time_out = 10;
	unsigned char config_time = 0;
	_Bool status = 1;
	unsigned char key_count = 0, wait_count = 0, wifi_status = 0;
	
	while(time_out--)												//�ȴ�
	{
		NET_DEVICE_Delay(200);										//����ȴ�
		
		if(MCU_GPIO_Input_Read("key1") == 0)						//������������ģʽ
			key_count++;
		else
			key_count = 0;
		
		wifi_status = NET_DEVICE_Check();							//��ȡ״̬
		
		wait_count++;
		
		if(wifi_status || key_count)								//����յ�����
		{
			if(wifi_status == NET_DEVICE_GOT_IP && !key_count)
			{
				status = 0;
				break;
			}
			else if(wait_count >= 10 || key_count)
			{
				wait_count = 0;
				key_count = 0;
#if(PHONE_AP_MODE == 0)
				NET_DEVICE_SendCmd("AT+AIRKISS\r\n", "Ready", 200);
				UsartPrintf(USART_DEBUG, "��ʹ��OneNET΢�Ź��ں�����SSID��PSWD\r\n");
				
				while(1)
				{
					MCU_GPIO_Output_Ctl("led1", 0);MCU_GPIO_Output_Ctl("led2", 0);
					MCU_GPIO_Output_Ctl("led3", 0);MCU_GPIO_Output_Ctl("led4", 0);
					
					if(strstr((char *)net_device_info.cmd_resp, "Airkiss OK"))
					{
						UsartPrintf(USART_DEBUG, "�յ�:\r\n%s\r\n", strstr((char *)net_device_info.cmd_resp, "ssid:"));
						status = 1;
						
						MCU_GPIO_Output_Ctl("led1", 1);MCU_GPIO_Output_Ctl("led2", 1);
						MCU_GPIO_Output_Ctl("led3", 1);MCU_GPIO_Output_Ctl("led4", 1);
						
						break;
					}
					else
					{
						if(++config_time >= 30)													//��ʱʱ��--30s
						{
							config_time = 0;
							
							MCU_GPIO_Output_Ctl("led1", 1);MCU_GPIO_Output_Ctl("led2", 1);
							MCU_GPIO_Output_Ctl("led3", 1);MCU_GPIO_Output_Ctl("led4", 1);
							
							break;
						}
					}
					
					NET_DEVICE_Delay(100);
					
					MCU_GPIO_Output_Ctl("led1", 1);MCU_GPIO_Output_Ctl("led2", 1);
					MCU_GPIO_Output_Ctl("led3", 1);MCU_GPIO_Output_Ctl("led4", 1);
					
					NET_DEVICE_Delay(100);
				}
				
				if(config_time != 0)															//���Ϊ0�����ǳ�ʱ�˳�
				{
					config_time = 0;
					while(NET_DEVICE_Check() != NET_DEVICE_GOT_IP)								//�ȴ�WIFI����
					{
						if(++config_time >= 10)
						{
							UsartPrintf(USART_DEBUG, "���볬ʱ,����WIFI����\r\n");
							break;
						}
						
						NET_DEVICE_Delay(200);
					}
				}
#else
				UsartPrintf(USART_DEBUG, "STA Tips:	Link Wifi\r\n");
				
				while(NET_DEVICE_SendCmd("AT+CWJAP=ssid,password\r\n", "WIFI GOT IP", 800))
					NET_DEVICE_Delay(100);
				
				status = 0;
				
				break;
#endif
			}
			else
				status = 1;
		}
	}
	
	return status;

}

//==========================================================
//	�������ƣ�	NET_DEVICE_Init
//
//	�������ܣ�	�����豸��ʼ��
//
//	��ڲ�����	��
//
//	���ز�����	���س�ʼ�����
//
//	˵����		0-�ɹ�		1-ʧ��
//==========================================================
_Bool NET_DEVICE_Init(void)
{
	
	_Bool status = 1;

	switch(net_device_info.init_step)
	{
		case 0:
		
			if(NET_DEVICE_Exist() == 0)
			{
				net_device_info.init_step++;
			}
		
		break;
		
		case 1:
		
#if(NET_TIME_EN == 1)
			if(NET_DEVICE_GetTime())
#endif
			{
				net_device_info.init_step++;
			}
		
		break;
		
		default:
			status = 0;
		break;
	}
	
	return status;

}

//==========================================================
//	�������ƣ�	NET_DEVICE_Reset
//
//	�������ܣ�	�����豸��λ
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void NET_DEVICE_Reset(void)
{

	UsartPrintf(USART_DEBUG, "Tips:	NET_DEVICE_Reset\r\n");
	
	MCU_GPIO_Output_Ctl("nd_rst", 0);		//��λ
	NET_DEVICE_Delay(50);
	
	MCU_GPIO_Output_Ctl("nd_rst", 1);		//������λ
	NET_DEVICE_Delay(200);

}

//==========================================================
//	�������ƣ�	NET_DEVICE_CmdHandle
//
//	�������ܣ�	���������Ƿ���ȷ
//
//	��ڲ�����	cmd����Ҫ���͵�����
//
//	���ز�����	��
//
//	˵����		�����ɹ���ָ����NULL
//==========================================================
void NET_DEVICE_CmdHandle(char *cmd)
{
	
	if(strstr(cmd, net_device_info.cmd_hdl) != NULL)
		net_device_info.cmd_hdl = NULL;
	else if(strstr(cmd, "ERROR") != NULL)
	{
		cmd_resp_err_flag = 1;
		net_device_info.cmd_hdl = NULL;
	}
	
	net_device_info.cmd_resp = cmd;

}

//==========================================================
//	�������ƣ�	NET_DEVICE_SendCmd
//
//	�������ܣ�	�������豸����һ��������ȴ���ȷ����Ӧ
//
//	��ڲ�����	cmd����Ҫ���͵�����
//				res����Ҫ��������Ӧ
//				time_out���ȴ�ʱ��(��ϵͳʱ��Ϊ׼)
//
//	���ز�����	�������ӽ��
//
//	˵����		0-�ɹ�		1-ʧ��
//==========================================================
_Bool NET_DEVICE_SendCmd(char *cmd, char *res, unsigned short time_out)
{
	
	_Bool result = 0;
	
	if(NET_DEVCIE_IsBusReady() == 1)
		return 1;
	
	NET_IO_Send((unsigned char *)cmd, strlen(cmd));			//д��������豸
	
	if(res == NULL)											//���Ϊ�գ���ֻ�Ƿ���
	{
		NET_DEVCIE_FreeBus();
		
		return 0;
	}
	
	net_device_info.cmd_hdl = res;							//��Ҫ���ѵĹؼ���
	
	while((net_device_info.cmd_hdl != NULL) && --time_out)	//�ȴ�
		NET_DEVICE_Delay(2);
	
	if((time_out == 0) || (cmd_resp_err_flag == 1))
	{
		cmd_resp_err_flag = 0;
		result = 1;
	}
	
	NET_DEVCIE_FreeBus();
	
	return result;

}

//==========================================================
//	�������ƣ�	NET_DEVICE_Read
//
//	�������ܣ�	��ȡһ֡����
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
unsigned char *NET_DEVICE_Read(void)
{

	return NET_IO_Read();

}

//==========================================================
//	�������ƣ�	NET_DEVICE_Check
//
//	�������ܣ�	��������豸����״̬
//
//	��ڲ�����	��
//
//	���ز�����	����״̬
//
//	˵����		
//==========================================================
unsigned char NET_DEVICE_Check(void)
{
	
	unsigned char status = NET_DEVICE_NO_DEVICE, err_count = 3;
	int status_result = 255;
	char *data_ptr = NULL;
	
	while(--err_count)
	{
		if(NET_DEVICE_SendCmd("AT+CIPSTATUS\r\n", "+STATUS", 200) == 0)				//����״̬���
		{
			data_ptr = strstr(net_device_info.cmd_resp, "STATUS:");
			if(sscanf(data_ptr, "STATUS:%d", &status_result) == 1)
			{
				switch(status_result)
				{
					case 0:
						UsartPrintf(USART_DEBUG, "Tips:	NET_DEVICE IDLE\r\n");
						status = NET_DEVICE_INITIAL;
					break;
					
					case 1:
						//UsartPrintf(USART_DEBUG, "Tips:	NET_DEVICE Connecting\r\n");
						status = NET_DEVICE_CONNECTING;
					break;
					
					case 2:
						UsartPrintf(USART_DEBUG, "Tips:	NET_DEVICE Password Err\r\n");
						status = NET_DEVICE_PSWD_ERR;
					break;
					
					case 3:
						UsartPrintf(USART_DEBUG, "Tips:	NET_DEVICE No AP\r\n");
						status = NET_DEVICE_NO_AP;
					break;
					
					case 4:
						UsartPrintf(USART_DEBUG, "Tips:	NET_DEVICE Connect Err\r\n");
						status = NET_DEVICE_CONNCET_ERR;
					break;
					
					case 5:
						UsartPrintf(USART_DEBUG, "Tips:	NET_DEVICE Got IP\r\n");
						status = NET_DEVICE_GOT_IP;
					break;
				}
				
				break;
			}
		}
		
		NET_DEVICE_Delay(20);
	}
	
	return status;

}
