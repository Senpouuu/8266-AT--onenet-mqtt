#ifndef _PRO_KIT_H_
#define _PRO_KIT_H_





typedef enum
{

	TYPE_NUM = 0,
	
	TYPE_GPS,
	
	TYPE_STRING,

} DATA_TYPE;

typedef struct
{

	char *name;
	void *dataPoint;
	DATA_TYPE dataType;
	_Bool flag;

} DATA_STREAM;

typedef enum
{

	FORMAT_TYPE1 = 1,
	FORMAT_TYPE2,
	FORMAT_TYPE3,
	FORMAT_TYPE4,
	FORMAT_TYPE5

} FORMAT_TYPE;


short DSTREAM_GetDataStream_Body(unsigned char type, DATA_STREAM *streamArray, unsigned short streamArrayCnt, unsigned char *buffer, short maxLen, short offset);

short DSTREAM_GetDataStream_Body_Measure(unsigned char type, DATA_STREAM *streamArray, unsigned short streamArrayCnt, _Bool flag);

#endif
