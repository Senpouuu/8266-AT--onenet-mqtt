#ifndef _LCD1602_H_
#define _LCD1602_H_





void LCD1602_Init(void);

void LCD1602_Clear(unsigned char pos);

void LCD1602_DisString(unsigned char pos, char *fmt,...);


#endif
