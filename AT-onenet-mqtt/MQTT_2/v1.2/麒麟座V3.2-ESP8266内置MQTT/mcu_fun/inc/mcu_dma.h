#ifndef _MCU_DMA_H_
#define _MCU_DMA_H_


//单片机头文件
#include "stm32f10x.h"


void MCU_DMA_Init(DMA_TypeDef *dma_group, DMA_Channel_TypeDef *dma_channel, unsigned int dma_pba, unsigned int dma_mba, unsigned int dma_dir,
					unsigned int dma_buffersize,unsigned int dma_pi, unsigned int dma_mi, unsigned int dma_pds, unsigned int dma_mds,
					unsigned int dma_mode, unsigned int dma_priority, unsigned int dma_m2m);

void MCU_DMA_ResetMemoryBaseAddr(DMA_Channel_TypeDef *dma_channel, unsigned int mAddr, unsigned short num);


#endif
