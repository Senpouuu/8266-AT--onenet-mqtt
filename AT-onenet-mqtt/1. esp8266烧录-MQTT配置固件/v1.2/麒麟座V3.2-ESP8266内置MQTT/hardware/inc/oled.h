#ifndef _OLED_H_
#define _OLED_H_





#define OLED_ADDRESS			0x3C


#define OLED_IO					I2C2


typedef struct
{

	_Bool device_ok;

} OLED_INFO;

extern OLED_INFO oled_info;


void OLED_Init(void);

_Bool OLED_Exist(void);

void OLED_ClearScreen(void);

void OLED_ClearAt(unsigned char x);

void OLED_Dis12864_Pic(const unsigned char *dp);

void OLED_DisChar16x16(unsigned short page, unsigned short column, const unsigned char *dp);

void OLED_DisString6x8(unsigned char x, unsigned char y, char *fmt, ...);

void OLED_DisString8x16(unsigned char x, unsigned char y, char *fmt, ...);


#endif
