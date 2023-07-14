#ifndef _AT_H_
#define _AT_H_





typedef void (*CMD_CALL_BACK)(char *cmd, char *para);


typedef struct
{

	unsigned char recv_buf[64];
	unsigned short recv_len;
	
	unsigned char recv_flag : 1;
	unsigned char busy : 1;
	unsigned char reverse : 6;

} AT_INFO;

extern AT_INFO at_info;


typedef struct
{

	char *cmd;
	
	CMD_CALL_BACK cmd_cb;

} AT_LIST_INFO;


_Bool AT_Recv_Pro(void);


#endif
