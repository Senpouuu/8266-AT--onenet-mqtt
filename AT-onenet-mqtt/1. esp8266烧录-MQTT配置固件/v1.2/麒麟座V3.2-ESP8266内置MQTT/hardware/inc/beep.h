#ifndef _BEEP_H_
#define _BEEP_H_





#define BEEP_NUM		1

#define BEEP_ALL		255


typedef struct
{

	_Bool beep_status[BEEP_NUM];

} BEEP_INFO;

extern BEEP_INFO beep_info;

typedef enum
{

	BEEP_OFF = 0,
	BEEP_ON

} BEEP_ENUM;


void BEEP_Init(void);

void BEEP_Ctl(unsigned char num, BEEP_ENUM status);


#endif
