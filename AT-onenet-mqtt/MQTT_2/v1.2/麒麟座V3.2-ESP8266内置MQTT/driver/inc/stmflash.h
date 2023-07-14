#ifndef _STMFLASH_H_
#define _STMFLASH_H_





#define W_G_ADDR		0x0807E800 //����ʱ��ѡ��ʽ							��ַ������2�ı���


unsigned int FLASH_GetStartAddr(unsigned char page, _Bool flash_type);

void FLASH_Erase(unsigned char page, unsigned char num, _Bool flash_type);

void Flash_Read(unsigned char page, unsigned short *rBuf, unsigned short len, _Bool flash_type);

void Flash_Write(unsigned char page, unsigned short *wBuf, unsigned short len, _Bool flash_type);
void Write_UP_Data_Flag(void);


#endif
