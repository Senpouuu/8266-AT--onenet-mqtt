#ifndef _DSTREAM_H_
#define _DSTREAM_H_





typedef enum
{

	TYPE_BOOL = 0,
	
	TYPE_CHAR,
	TYPE_UCHAR,
	
	TYPE_SHORT,
	TYPE_USHORT,
	
	TYPE_INT,
	TYPE_UINT,
	
	TYPE_LONG,
	TYPE_ULONG,
	
	TYPE_FLOAT,
	TYPE_DOUBLE,
	
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


_Bool DSTREAM_GetDataStream_Body(char *buf, unsigned short bufLen, DATA_TYPE type, char *name, void *dataPoint);


#endif
