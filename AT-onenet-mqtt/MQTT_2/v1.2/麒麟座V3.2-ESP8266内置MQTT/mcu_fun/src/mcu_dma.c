/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	mcu_dma.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2018-02-25
	*
	*	版本： 		V1.0
	*
	*	说明： 		单片机DMA
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机相关组件
#include "mcu_rcc.h"
#include "mcu_dma.h"


/*
************************************************************
*	函数名称：	MCU_DMA_Init
*
*	函数功能：	单片机DMA初始化
*
*	入口参数：	dma_group：DMA组
*				dma_channel：DMA通道
*				dma_pba：外设基地址
*				dma_mba：内存基地址
*				dma_dir：传输方向
*				dma_buffersize：缓存大小
*				dma_pi：外设地址寄存器是否递增
*				dma_mi：内存地址寄存器是否递增
*				dma_pds：外设数据宽度
*				dma_mds：内存数据宽度
*				dma_mode：工作模式
*				dma_priority：DMA通道优先级
*				dma_m2m：DMA通道是否内存到内存传输
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void MCU_DMA_Init(DMA_TypeDef *dma_group, DMA_Channel_TypeDef *dma_channel, unsigned int dma_pba, unsigned int dma_mba, unsigned int dma_dir,
					unsigned int dma_buffersize,unsigned int dma_pi, unsigned int dma_mi, unsigned int dma_pds, unsigned int dma_mds,
					unsigned int dma_mode, unsigned int dma_priority, unsigned int dma_m2m)
{

	DMA_InitTypeDef dma_initstruct;
	
	MCU_RCC(dma_group, ENABLE);								//使能时钟
	
	DMA_DeInit(dma_channel);								//将DMA的通道1寄存器重设为缺省值
	
	dma_initstruct.DMA_PeripheralBaseAddr = dma_pba;		//DMA外设基地址
	dma_initstruct.DMA_MemoryBaseAddr = dma_mba;			//DMA内存基地址
	dma_initstruct.DMA_DIR = dma_dir;						//数据传输方向
	dma_initstruct.DMA_BufferSize = dma_buffersize;			//DMA通道的DMA缓存的大小
	dma_initstruct.DMA_PeripheralInc = dma_pi;				//外设地址寄存器是否递增
	dma_initstruct.DMA_MemoryInc = dma_mi;					//内存地址寄存器是否递增
	dma_initstruct.DMA_PeripheralDataSize = dma_pds;		//外设数据宽度
	dma_initstruct.DMA_MemoryDataSize = dma_mds;			//内存数据宽度
	dma_initstruct.DMA_Mode = dma_mode;						//工作模式
	dma_initstruct.DMA_Priority = dma_priority;				//DMA通道优先级
	dma_initstruct.DMA_M2M = dma_m2m;						//DMA通道是否内存到内存传输
	
	DMA_Init(dma_channel, &dma_initstruct);					//初始化

}

/*
************************************************************
*	函数名称：	MCU_DMA_ResetMemoryBaseAddr
*
*	函数功能：	单片机DMA重设内存地址
*
*	入口参数：	dma_channel：DMA通道
*				mAddr：内存基地址
*				num：缓存大小
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void MCU_DMA_ResetMemoryBaseAddr(DMA_Channel_TypeDef *dma_channel, unsigned int mAddr, unsigned short num)
{

	dma_channel->CCR &= (unsigned short)(~DMA_CCR1_EN);	//关闭DMA1所指示的通道
			
	dma_channel->CMAR = mAddr;							//DMA通道的内存地址
	dma_channel->CNDTR = num;							//DMA通道的DMA缓存的大小
	
	dma_channel->CCR |= DMA_CCR1_EN;

}
