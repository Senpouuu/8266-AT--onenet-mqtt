#ifndef _CMD_CALLBACK_H_
#define _CMD_CALLBACK_H_





typedef void (*CALL_BACK)(char *msg);


typedef struct
{

	char *cmd_name;
	
	CALL_BACK call_back;
	

} CMD_CALLBACK_INFO;


_Bool CALLBACK_Execute(char *cmd);


#endif
