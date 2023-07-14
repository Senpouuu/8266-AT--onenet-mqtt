/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	mcu_dma.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2018-02-25
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		��Ƭ��DMA
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��������
#include "mcu_rcc.h"
#include "mcu_dma.h"


/*
************************************************************
*	�������ƣ�	MCU_DMA_Init
*
*	�������ܣ�	��Ƭ��DMA��ʼ��
*
*	��ڲ�����	dma_group��DMA��
*				dma_channel��DMAͨ��
*				dma_pba���������ַ
*				dma_mba���ڴ����ַ
*				dma_dir�����䷽��
*				dma_buffersize�������С
*				dma_pi�������ַ�Ĵ����Ƿ����
*				dma_mi���ڴ��ַ�Ĵ����Ƿ����
*				dma_pds���������ݿ��
*				dma_mds���ڴ����ݿ��
*				dma_mode������ģʽ
*				dma_priority��DMAͨ�����ȼ�
*				dma_m2m��DMAͨ���Ƿ��ڴ浽�ڴ洫��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void MCU_DMA_Init(DMA_TypeDef *dma_group, DMA_Channel_TypeDef *dma_channel, unsigned int dma_pba, unsigned int dma_mba, unsigned int dma_dir,
					unsigned int dma_buffersize,unsigned int dma_pi, unsigned int dma_mi, unsigned int dma_pds, unsigned int dma_mds,
					unsigned int dma_mode, unsigned int dma_priority, unsigned int dma_m2m)
{

	DMA_InitTypeDef dma_initstruct;
	
	MCU_RCC(dma_group, ENABLE);								//ʹ��ʱ��
	
	DMA_DeInit(dma_channel);								//��DMA��ͨ��1�Ĵ�������Ϊȱʡֵ
	
	dma_initstruct.DMA_PeripheralBaseAddr = dma_pba;		//DMA�������ַ
	dma_initstruct.DMA_MemoryBaseAddr = dma_mba;			//DMA�ڴ����ַ
	dma_initstruct.DMA_DIR = dma_dir;						//���ݴ��䷽��
	dma_initstruct.DMA_BufferSize = dma_buffersize;			//DMAͨ����DMA����Ĵ�С
	dma_initstruct.DMA_PeripheralInc = dma_pi;				//�����ַ�Ĵ����Ƿ����
	dma_initstruct.DMA_MemoryInc = dma_mi;					//�ڴ��ַ�Ĵ����Ƿ����
	dma_initstruct.DMA_PeripheralDataSize = dma_pds;		//�������ݿ��
	dma_initstruct.DMA_MemoryDataSize = dma_mds;			//�ڴ����ݿ��
	dma_initstruct.DMA_Mode = dma_mode;						//����ģʽ
	dma_initstruct.DMA_Priority = dma_priority;				//DMAͨ�����ȼ�
	dma_initstruct.DMA_M2M = dma_m2m;						//DMAͨ���Ƿ��ڴ浽�ڴ洫��
	
	DMA_Init(dma_channel, &dma_initstruct);					//��ʼ��

}

/*
************************************************************
*	�������ƣ�	MCU_DMA_ResetMemoryBaseAddr
*
*	�������ܣ�	��Ƭ��DMA�����ڴ��ַ
*
*	��ڲ�����	dma_channel��DMAͨ��
*				mAddr���ڴ����ַ
*				num�������С
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void MCU_DMA_ResetMemoryBaseAddr(DMA_Channel_TypeDef *dma_channel, unsigned int mAddr, unsigned short num)
{

	dma_channel->CCR &= (unsigned short)(~DMA_CCR1_EN);	//�ر�DMA1��ָʾ��ͨ��
			
	dma_channel->CMAR = mAddr;							//DMAͨ�����ڴ��ַ
	dma_channel->CNDTR = num;							//DMAͨ����DMA����Ĵ�С
	
	dma_channel->CCR |= DMA_CCR1_EN;

}
