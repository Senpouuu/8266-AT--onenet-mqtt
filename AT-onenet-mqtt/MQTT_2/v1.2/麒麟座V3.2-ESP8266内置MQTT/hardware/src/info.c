/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	info.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2017-02-23
	*
	*	�汾�� 		V1.1
	*
	*	˵���� 		V1.0��SSID��PSWD��DEVID��APIKEY���漰��ȡ��
	*				V1.1��ȡ����SSID��PSWD�ı���Ͷ�д���滻Ϊ������������wifi���͵������豸�����Զ����档
	*
	*				��Ҫ��ֻ�е��ⲿ�洢������ʱ���Ŵ��ж�ȡ��Ϣ
	*					  �������ڣ����ȡ�̻��ڴ��������Ϣ
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//Ӳ������
#include "info.h"
#include "at24c02.h"
#include "delay.h"
#include "usart.h"

//Э��
#include "onenet.h"

//C��
#include <string.h>
#include <stdlib.h>


/*
************************************************************
*	�������ƣ�	Info_Check
*
*	�������ܣ�	�����Ϣ�Ƿ����
*
*	��ڲ�����	��
*
*	���ز�����	0-��	1-��
*
*	˵����		�ж�EEPROM�Ƿ�洢�˵�¼��Ϣ
************************************************************
*/
_Bool Info_Check(void)
{
	
	unsigned char r_data = 0;
	_Bool result = 0;
	
	AT24C02_ReadByte(DEVID_ADDRESS, &r_data);	//��ȡ����ֵ
	if(r_data == 0 || r_data >= 10)				//���Ϊ0�򳬳�
		result = 1;
	
	AT24C02_ReadByte(AKEY_ADDRESS, &r_data);	//��ȡ����ֵ
	if(r_data == 0 || r_data >= 30)				//���Ϊ0�򳬳�
		result = 1;
	
	AT24C02_ReadByte(PROID_ADDRESS, &r_data);	//��ȡ����ֵ
	if(r_data == 0 || r_data >= 10)				//���Ϊ0�򳬳�
		result = 1;
	
	AT24C02_ReadByte(AUIF_ADDRESS, &r_data);	//��ȡ����ֵ
	if(r_data == 0 || r_data >= 50)				//���Ϊ0�򳬳�
		result = 1;
        
	return result;

}

/*
************************************************************
*	�������ƣ�	Info_WifiLen
*
*	�������ܣ�	��ȡ��Ϣ����
*
*	��ڲ�����	sp����Ҫ������Ϣ-��˵��
*
*	���ز�����	�����
*
*	˵����		0-ʧ��
*
*	˵����		0-devid����		1-apikey����
*				1-proid����		2-auth_info����
************************************************************
*/
unsigned char Info_WifiLen(unsigned char sp)
{
	
	unsigned char len = 0;
    
	switch(sp)
	{
		case 0:
			AT24C02_ReadByte(DEVID_ADDRESS, &len);		//��ȡ����ֵ
			if(len == 0 || len >= 10)					//���Ϊ0�򳬳�
				return 0;
		break;

		case 1:
			AT24C02_ReadByte(AKEY_ADDRESS, &len);		//��ȡ����ֵ
			if(len == 0 || len >= 30)					//���Ϊ0�򳬳�
				return 0;
		break;
			
		case 2:
            AT24C02_ReadByte(PROID_ADDRESS, &len);		//��ȡ����ֵ
			if(len == 0 || len >= 10)					//���Ϊ0�򳬳�
				return 0;
        break;
			
		case 3:
            AT24C02_ReadByte(AUIF_ADDRESS, &len);		//��ȡ����ֵ
			if(len == 0 || len >= 50)					//���Ϊ0�򳬳�
				return 0;
        break;
	}

	return len;

}

/*
************************************************************
*	�������ƣ�	Info_CountLen
*
*	�������ܣ�	�����ֶγ���
*
*	��ڲ�����	info����Ҫ�����ֶ�
*
*	���ز�����	�ֶγ���
*
*	˵����		���㴮1���������ֶγ���   ��"\r\n"��β
************************************************************
*/
unsigned char Info_CountLen(char *info)
{

	unsigned char len = 0;

	char *data_ptr = strchr(info, ':');		//�ҵ�':'

	data_ptr++;								//ƫ�Ƶ���һ���ֽڣ������ֶ���Ϣ��ʼ
	while(1)
	{
		if(*data_ptr == '\r')				//ֱ��'\r'Ϊֹ
			return len;
		
		if(*data_ptr == '\0')				//�Ҳ������з�,����Ϊ��Ч
			return 0;

		data_ptr++;
		len++;
	}

}

/*
************************************************************
*	�������ƣ�	Info_Read
*
*	�������ܣ�	��ȡssid��pswd��devid��apikey
*
*	��ڲ�����	��
*
*	���ز�����	��ȡ���
*
*	˵����		0-�ɹ�		1-ʧ��
************************************************************
*/
_Bool Info_Read(void)
{
	
	unsigned char len = 0;

	len = Info_WifiLen(0);RTOS_TimeDly(1);
	if(len)
	{
		memset(onenet_info.dev_id, 0, sizeof(onenet_info.dev_id));						//���֮ǰ������
		AT24C02_ReadBytes(DEVID_ADDRESS + 1, (unsigned char *)onenet_info.dev_id, len);	//��ȡdevid����  ��devid
	}
	RTOS_TimeDly(2);																	//��ʱ

	len = Info_WifiLen(1);RTOS_TimeDly(1);
	if(len)
	{
		memset(onenet_info.api_key, 0, sizeof(onenet_info.api_key));					//���֮ǰ������
		AT24C02_ReadBytes(AKEY_ADDRESS + 1, (unsigned char *)onenet_info.api_key, len);	//��ȡapikey����  ��apikey
	}
	RTOS_TimeDly(2);																	//��ʱ

	len = Info_WifiLen(2);RTOS_TimeDly(1);
	if(len)
	{
		memset(onenet_info.pro_id, 0, sizeof(onenet_info.pro_id));						//���֮ǰ������
		AT24C02_ReadBytes(PROID_ADDRESS + 1, (unsigned char *)onenet_info.pro_id, len);	//��ȡproid����  ��proid
	}
	RTOS_TimeDly(2);																	//��ʱ

	len = Info_WifiLen(3);RTOS_TimeDly(1);
	if(len)
	{
		memset(onenet_info.auth_info, 0, sizeof(onenet_info.auth_info));				//���֮ǰ������
		AT24C02_ReadBytes(AUIF_ADDRESS + 1, (unsigned char *)onenet_info.auth_info, len);//��ȡauif����  ��auif
	}

	return 0;

}

/*
************************************************************
*	�������ƣ�	Info_Alter
*
*	�������ܣ�	����wifi��Ϣ����Ŀ��Ϣ
*
*	��ڲ�����	��Ҫ������ֶ�
*
*	���ز�����	������
*
*	˵����		0-����Ҫ��������		1-��Ҫ��������
************************************************************
*/
_Bool Info_Alter(char *info)
{
    
	char *data_ptr = NULL;
	unsigned char data_len = 0;
	_Bool flag = 0;

	if((data_ptr = strstr(info, "DEVID:")) != (void *)0)								//��ȡdevid
	{
		data_len = Info_CountLen(data_ptr);												//���㳤��
		if(data_len > 0)
		{
			memset(onenet_info.dev_id, 0, sizeof(onenet_info.dev_id));					//���֮ǰ������
			strncpy(onenet_info.dev_id, data_ptr + 6, data_len);
			UsartPrintf(USART_DEBUG, "Tips:	Save DEVID: %s\r\n", onenet_info.dev_id);

			RTOS_TimeDly(2);
			AT24C02_WriteByte(DEVID_ADDRESS, strlen(onenet_info.dev_id));				//����devid����
			RTOS_TimeDly(2);
			AT24C02_WriteBytes(DEVID_ADDRESS + 1,										//����ssid
			(unsigned char *)onenet_info.dev_id,
			strlen(onenet_info.dev_id));

			flag = 1;
		}
	}

	if((data_ptr = strstr(info, "APIKEY:")) != (void *)0)								//��ȡapikey
	{
		data_len = Info_CountLen(data_ptr);												//���㳤��
		if(data_len > 0)
		{
			memset(onenet_info.api_key, 0, sizeof(onenet_info.api_key));				//���֮ǰ������
			strncpy(onenet_info.api_key, data_ptr + 7, data_len);
			UsartPrintf(USART_DEBUG, "Tips:	Save APIKEY: %s\r\n", onenet_info.api_key);

			RTOS_TimeDly(2);
			AT24C02_WriteByte(AKEY_ADDRESS, strlen(onenet_info.api_key));				//����apikey����
			RTOS_TimeDly(2);
			AT24C02_WriteBytes(AKEY_ADDRESS + 1,										//����ssid
			(unsigned char *)onenet_info.api_key,
			strlen(onenet_info.api_key));

			flag = 1;
		}
	}
	
	if((data_ptr = strstr(info, "PROID:")) != (void *)0)								//��ȡproID
	{
		data_len = Info_CountLen(data_ptr);												//���㳤��
		if(data_len > 0)
		{
			memset(onenet_info.pro_id, 0, sizeof(onenet_info.pro_id));					//���֮ǰ������
			strncpy(onenet_info.pro_id, data_ptr + 6, data_len);
			UsartPrintf(USART_DEBUG, "Tips:	Save PROID: %s\r\n", onenet_info.pro_id);

			RTOS_TimeDly(2);
			AT24C02_WriteByte(PROID_ADDRESS, strlen(onenet_info.pro_id));				//����proID����
			RTOS_TimeDly(2);
			AT24C02_WriteBytes(PROID_ADDRESS + 1,										//����proID
			(unsigned char *)onenet_info.pro_id,
			strlen(onenet_info.pro_id));

			flag = 1;
		}
	}

	if((data_ptr = strstr(info, "AUIF:")) != (void *)0)									//��ȡauif
	{
		data_len = Info_CountLen(data_ptr);												//���㳤��
		if(data_len > 0)
		{
			memset(onenet_info.auth_info, 0, sizeof(onenet_info.auth_info));			//���֮ǰ������
			strncpy(onenet_info.auth_info, data_ptr + 5, data_len);
			UsartPrintf(USART_DEBUG, "Tips:	Save AUIF: %s\r\n", onenet_info.auth_info);

			RTOS_TimeDly(2);
			AT24C02_WriteByte(AUIF_ADDRESS, strlen(onenet_info.auth_info));				//����auif����
			RTOS_TimeDly(2);
			AT24C02_WriteBytes(AUIF_ADDRESS + 1,										//����auif
			(unsigned char *)onenet_info.auth_info,
			strlen(onenet_info.auth_info));

			flag = 1;
		}
	}

	return flag;

}
