#ifndef _AT24C02_H_
#define _AT24C02_H_





#define AT24C02_IO			I2C2


#define AT24C02_ADDRESS		0x50			//AT24C02µÿ÷∑


typedef struct
{

	_Bool device_ok;

} EEPROM_INFO;

extern EEPROM_INFO eeprom_info;


_Bool AT24C02_Exist(void);

_Bool AT24C02_WriteByte(unsigned char reg_addr, unsigned char byte);

_Bool AT24C02_WriteBytes(unsigned char reg_addr, unsigned char *byte, unsigned char len);

_Bool AT24C02_ReadByte(unsigned char reg_addr, unsigned char *byte);

_Bool AT24C02_ReadBytes(unsigned char reg_addr, unsigned char *byte, unsigned char len);

_Bool AT24C02_Clear(unsigned char start_addr, unsigned char byte, unsigned short len);


#endif
