/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	dStream.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2020-10-09
	*
	*	版本： 		V1.0
	*
	*	说明： 		
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//协议封装文件
#include "dStream.h"

//C库
#include <string.h>
#include <stdio.h>


//==========================================================
//	函数名称：	 DSTREAM_GetDataStream_Body
//
//	函数功能：	获取数据流格式消息体
//
//	入口参数：	buf：缓存
//				bufLen：buf长度
//				type：格式类型
//				name：数据流名
//				dataPoint：数据流值
//
//	返回参数：	0-成功		1-失败
//
//	说明：		
//==========================================================
_Bool DSTREAM_GetDataStream_Body(char *buf, unsigned short bufLen, DATA_TYPE type, char *name, void *dataPoint)
{
	
	if(buf == NULL || name == NULL || dataPoint == NULL)
		return 1;
	
	switch((unsigned char)type)
	{
		case TYPE_BOOL:
			snprintf(buf, bufLen, "AT+IOTSEND=%d,%s,%d\r\n", 0, name, *(_Bool *)dataPoint);
		break;
		
		case TYPE_CHAR:
			snprintf(buf, bufLen, "AT+IOTSEND=%d,%s,%d\r\n", 0, name, *(signed char *)dataPoint);
		break;
		
		case TYPE_UCHAR:
			snprintf(buf, bufLen, "AT+IOTSEND=%d,%s,%d\r\n", 0, name, *(unsigned char *)dataPoint);
		break;
		
		case TYPE_SHORT:
			snprintf(buf, bufLen, "AT+IOTSEND=%d,%s,%d\r\n", 0, name, *(signed short *)dataPoint);
		break;
		
		case TYPE_USHORT:
			snprintf(buf, bufLen, "AT+IOTSEND=%d,%s,%d\r\n", 0, name, *(unsigned short *)dataPoint);
		break;
		
		case TYPE_INT:
			snprintf(buf, bufLen, "AT+IOTSEND=%d,%s,%d\r\n", 0, name, *(signed int *)dataPoint);
		break;
		
		case TYPE_UINT:
			snprintf(buf, bufLen, "AT+IOTSEND=%d,%s,%d\r\n", 0, name, *(unsigned int *)dataPoint);
		break;
		
		case TYPE_LONG:
			snprintf(buf, bufLen, "AT+IOTSEND=%d,%s,%ld\r\n", 0, name, *(signed long *)dataPoint);
		break;
		
		case TYPE_ULONG:
			snprintf(buf, bufLen, "AT+IOTSEND=%d,%s,%ld\r\n", 0, name, *(unsigned long *)dataPoint);
		break;
			
		case TYPE_FLOAT:
			snprintf(buf, bufLen, "AT+IOTSEND=%d,%s,%f\r\n", 0, name, *(float *)dataPoint);
		break;
		
		case TYPE_DOUBLE:
			snprintf(buf, bufLen, "AT+IOTSEND=%d,%s,%lf\r\n", 0, name, *(double *)dataPoint);
		break;
		
		case TYPE_GPS:
			snprintf(buf, bufLen, "AT+IOTSEND=%d,%s,%s,%s\r\n", 2, name, (char *)dataPoint, (char *)(dataPoint) + 24);
		break;
		
		case TYPE_STRING:
			snprintf(buf, bufLen, "AT+IOTSEND=%d,%s,%s\r\n", 1, name, (char *)dataPoint);
		break;
	}
	
	return 0;

}
