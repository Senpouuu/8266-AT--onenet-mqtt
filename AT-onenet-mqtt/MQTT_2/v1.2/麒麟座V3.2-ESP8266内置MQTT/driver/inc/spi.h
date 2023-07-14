#ifndef _SPI_H_
#define _SPI_H_


#include "stm32f10x.h"


#define SPI_DMA_TX_EN		0		//1-使能DMA发送		0-失能DMA发送

#define SPI_DMA_RX_EN		0		//1-使能DMA接收		0-失能DMA接收


#define SPI_TX_TYPE			0

#define SPI_RX_TYPE			1


#define SPI_OK				0

#define SPI_Err				1


_Bool SPIx_IsBusReady(SPI_TypeDef *spi_x);

void SPIx_FreeBus(SPI_TypeDef *spi_x);

void SPIx_Init(SPI_TypeDef *spi_x);

void SPIx_TX_Init(SPI_TypeDef *spi_x);

void SPIx_ResetMemoryBaseAddr(SPI_TypeDef *spi_x, unsigned int mem_addr, unsigned short num, _Bool type);

void SPIx_SetSpeed(SPI_TypeDef *spi_x, unsigned char speed);

unsigned char SPIx_ReadWriteByte(SPI_TypeDef *spi_x, unsigned char tx_data);

void SPIx_DMA_ReadWriteByte(SPI_TypeDef *spi_x, unsigned char *tx_buf, unsigned char *rx_buf, unsigned short tx_size, unsigned short rx_size);

void SPIx_WriteByte(SPI_TypeDef *spi_x, unsigned char TxData);


#endif
