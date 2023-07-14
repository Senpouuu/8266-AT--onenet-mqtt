/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	info.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-02-23
	*
	*	版本： 		V1.1
	*
	*	说明： 		V1.0：SSID、PSWD、DEVID、APIKEY保存及读取。
	*				V1.1：取消了SSID和PSWD的保存和读写，替换为了智能配网，wifi类型的网络设备可以自动保存。
	*
	*				重要：只有当外部存储器存在时，才从中读取信息
	*					  若不存在，会读取固化在代码里的信息
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//硬件驱动
#include "info.h"
#include "at24c02.h"
#include "delay.h"
#include "usart.h"

//协议
#include "onenet.h"

//C库
#include <string.h>
#include <stdlib.h>


/*
************************************************************
*	函数名称：	Info_Check
*
*	函数功能：	检查信息是否存在
*
*	入口参数：	无
*
*	返回参数：	0-有	1-无
*
*	说明：		判断EEPROM是否存储了登录信息
************************************************************
*/
_Bool Info_Check(void)
{
	
	unsigned char r_data = 0;
	_Bool result = 0;
	
	AT24C02_ReadByte(DEVID_ADDRESS, &r_data);	//读取长度值
	if(r_data == 0 || r_data >= 10)				//如果为0或超出
		result = 1;
	
	AT24C02_ReadByte(AKEY_ADDRESS, &r_data);	//读取长度值
	if(r_data == 0 || r_data >= 30)				//如果为0或超出
		result = 1;
	
	AT24C02_ReadByte(PROID_ADDRESS, &r_data);	//读取长度值
	if(r_data == 0 || r_data >= 10)				//如果为0或超出
		result = 1;
	
	AT24C02_ReadByte(AUIF_ADDRESS, &r_data);	//读取长度值
	if(r_data == 0 || r_data >= 50)				//如果为0或超出
		result = 1;
        
	return result;

}

/*
************************************************************
*	函数名称：	Info_WifiLen
*
*	函数功能：	获取信息长度
*
*	入口参数：	sp：需要检查的信息-见说明
*
*	返回参数：	检查结果
*
*	说明：		0-失败
*
*	说明：		0-devid长度		1-apikey长度
*				1-proid长度		2-auth_info长度
************************************************************
*/
unsigned char Info_WifiLen(unsigned char sp)
{
	
	unsigned char len = 0;
    
	switch(sp)
	{
		case 0:
			AT24C02_ReadByte(DEVID_ADDRESS, &len);		//读取长度值
			if(len == 0 || len >= 10)					//如果为0或超出
				return 0;
		break;

		case 1:
			AT24C02_ReadByte(AKEY_ADDRESS, &len);		//读取长度值
			if(len == 0 || len >= 30)					//如果为0或超出
				return 0;
		break;
			
		case 2:
            AT24C02_ReadByte(PROID_ADDRESS, &len);		//读取长度值
			if(len == 0 || len >= 10)					//如果为0或超出
				return 0;
        break;
			
		case 3:
            AT24C02_ReadByte(AUIF_ADDRESS, &len);		//读取长度值
			if(len == 0 || len >= 50)					//如果为0或超出
				return 0;
        break;
	}

	return len;

}

/*
************************************************************
*	函数名称：	Info_CountLen
*
*	函数功能：	计算字段长度
*
*	入口参数：	info：需要检查的字段
*
*	返回参数：	字段长度
*
*	说明：		计算串1发过来的字段长度   以"\r\n"结尾
************************************************************
*/
unsigned char Info_CountLen(char *info)
{

	unsigned char len = 0;

	char *data_ptr = strchr(info, ':');		//找到':'

	data_ptr++;								//偏移到下一个字节，代表字段信息开始
	while(1)
	{
		if(*data_ptr == '\r')				//直到'\r'为止
			return len;
		
		if(*data_ptr == '\0')				//找不到换行符,则认为无效
			return 0;

		data_ptr++;
		len++;
	}

}

/*
************************************************************
*	函数名称：	Info_Read
*
*	函数功能：	读取ssid、pswd、devid、apikey
*
*	入口参数：	无
*
*	返回参数：	读取结果
*
*	说明：		0-成功		1-失败
************************************************************
*/
_Bool Info_Read(void)
{
	
	unsigned char len = 0;

	len = Info_WifiLen(0);RTOS_TimeDly(1);
	if(len)
	{
		memset(onenet_info.dev_id, 0, sizeof(onenet_info.dev_id));						//清除之前的内容
		AT24C02_ReadBytes(DEVID_ADDRESS + 1, (unsigned char *)onenet_info.dev_id, len);	//获取devid长度  读devid
	}
	RTOS_TimeDly(2);																	//延时

	len = Info_WifiLen(1);RTOS_TimeDly(1);
	if(len)
	{
		memset(onenet_info.api_key, 0, sizeof(onenet_info.api_key));					//清除之前的内容
		AT24C02_ReadBytes(AKEY_ADDRESS + 1, (unsigned char *)onenet_info.api_key, len);	//获取apikey长度  读apikey
	}
	RTOS_TimeDly(2);																	//延时

	len = Info_WifiLen(2);RTOS_TimeDly(1);
	if(len)
	{
		memset(onenet_info.pro_id, 0, sizeof(onenet_info.pro_id));						//清除之前的内容
		AT24C02_ReadBytes(PROID_ADDRESS + 1, (unsigned char *)onenet_info.pro_id, len);	//获取proid长度  读proid
	}
	RTOS_TimeDly(2);																	//延时

	len = Info_WifiLen(3);RTOS_TimeDly(1);
	if(len)
	{
		memset(onenet_info.auth_info, 0, sizeof(onenet_info.auth_info));				//清除之前的内容
		AT24C02_ReadBytes(AUIF_ADDRESS + 1, (unsigned char *)onenet_info.auth_info, len);//获取auif长度  读auif
	}

	return 0;

}

/*
************************************************************
*	函数名称：	Info_Alter
*
*	函数功能：	更改wifi信息和项目信息
*
*	入口参数：	需要保存的字段
*
*	返回参数：	保存结果
*
*	说明：		0-不需要重新连接		1-需要重新连接
************************************************************
*/
_Bool Info_Alter(char *info)
{
    
	char *data_ptr = NULL;
	unsigned char data_len = 0;
	_Bool flag = 0;

	if((data_ptr = strstr(info, "DEVID:")) != (void *)0)								//提取devid
	{
		data_len = Info_CountLen(data_ptr);												//计算长度
		if(data_len > 0)
		{
			memset(onenet_info.dev_id, 0, sizeof(onenet_info.dev_id));					//清除之前的内容
			strncpy(onenet_info.dev_id, data_ptr + 6, data_len);
			UsartPrintf(USART_DEBUG, "Tips:	Save DEVID: %s\r\n", onenet_info.dev_id);

			RTOS_TimeDly(2);
			AT24C02_WriteByte(DEVID_ADDRESS, strlen(onenet_info.dev_id));				//保存devid长度
			RTOS_TimeDly(2);
			AT24C02_WriteBytes(DEVID_ADDRESS + 1,										//保存ssid
			(unsigned char *)onenet_info.dev_id,
			strlen(onenet_info.dev_id));

			flag = 1;
		}
	}

	if((data_ptr = strstr(info, "APIKEY:")) != (void *)0)								//提取apikey
	{
		data_len = Info_CountLen(data_ptr);												//计算长度
		if(data_len > 0)
		{
			memset(onenet_info.api_key, 0, sizeof(onenet_info.api_key));				//清除之前的内容
			strncpy(onenet_info.api_key, data_ptr + 7, data_len);
			UsartPrintf(USART_DEBUG, "Tips:	Save APIKEY: %s\r\n", onenet_info.api_key);

			RTOS_TimeDly(2);
			AT24C02_WriteByte(AKEY_ADDRESS, strlen(onenet_info.api_key));				//保存apikey长度
			RTOS_TimeDly(2);
			AT24C02_WriteBytes(AKEY_ADDRESS + 1,										//保存ssid
			(unsigned char *)onenet_info.api_key,
			strlen(onenet_info.api_key));

			flag = 1;
		}
	}
	
	if((data_ptr = strstr(info, "PROID:")) != (void *)0)								//提取proID
	{
		data_len = Info_CountLen(data_ptr);												//计算长度
		if(data_len > 0)
		{
			memset(onenet_info.pro_id, 0, sizeof(onenet_info.pro_id));					//清除之前的内容
			strncpy(onenet_info.pro_id, data_ptr + 6, data_len);
			UsartPrintf(USART_DEBUG, "Tips:	Save PROID: %s\r\n", onenet_info.pro_id);

			RTOS_TimeDly(2);
			AT24C02_WriteByte(PROID_ADDRESS, strlen(onenet_info.pro_id));				//保存proID长度
			RTOS_TimeDly(2);
			AT24C02_WriteBytes(PROID_ADDRESS + 1,										//保存proID
			(unsigned char *)onenet_info.pro_id,
			strlen(onenet_info.pro_id));

			flag = 1;
		}
	}

	if((data_ptr = strstr(info, "AUIF:")) != (void *)0)									//提取auif
	{
		data_len = Info_CountLen(data_ptr);												//计算长度
		if(data_len > 0)
		{
			memset(onenet_info.auth_info, 0, sizeof(onenet_info.auth_info));			//清除之前的内容
			strncpy(onenet_info.auth_info, data_ptr + 5, data_len);
			UsartPrintf(USART_DEBUG, "Tips:	Save AUIF: %s\r\n", onenet_info.auth_info);

			RTOS_TimeDly(2);
			AT24C02_WriteByte(AUIF_ADDRESS, strlen(onenet_info.auth_info));				//保存auif长度
			RTOS_TimeDly(2);
			AT24C02_WriteBytes(AUIF_ADDRESS + 1,										//保存auif
			(unsigned char *)onenet_info.auth_info,
			strlen(onenet_info.auth_info));

			flag = 1;
		}
	}

	return flag;

}
