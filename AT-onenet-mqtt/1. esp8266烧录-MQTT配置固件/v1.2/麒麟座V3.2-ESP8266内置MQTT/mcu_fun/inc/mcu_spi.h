#ifndef _MCU_SPI_H_
#define _MCU_SPI_H_


#include "stm32f10x.h"


_Bool MCU_SPIx_Init(SPI_TypeDef *SPI_Group, unsigned short SPI_Direction, unsigned short SPI_Mode, unsigned short SPI_DataSize,
					unsigned short SPI_CPOL, unsigned short SPI_CPHA, unsigned short SPI_NSS, unsigned short SPI_BaudRatePrescaler,
					unsigned short SPI_FirstBit, unsigned short SPI_CRCPolynomial);

_Bool MCU_SPIx_TX_Init(SPI_TypeDef *SPI_Group, unsigned short SPI_Direction, unsigned short SPI_Mode, unsigned short SPI_DataSize,
					unsigned short SPI_CPOL, unsigned short SPI_CPHA, unsigned short SPI_NSS, unsigned short SPI_BaudRatePrescaler,
					unsigned short SPI_FirstBit, unsigned short SPI_CRCPolynomial);


#endif
