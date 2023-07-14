/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	spi.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2017-02-27
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		SPI����
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��������
#include "mcu_spi.h"
#include "mcu_dma.h"

//Ӳ������
#include "delay.h"
#include "spi.h"


static _Bool spi_busy[3] = {SPI_OK, SPI_OK, SPI_OK};


/*
************************************************************
*	�������ƣ�	SPIx_IsBusReady
*
*	�������ܣ�	��ѯ�����Ƿ����
*
*	��ڲ�����	spi_x��SPIx
*
*	���ز�����	0-����	1-δ����
*
*	˵����		
************************************************************
*/
_Bool SPIx_IsBusReady(SPI_TypeDef *spi_x)
{

	_Bool result = SPI_Err;

	RTOS_ENTER_CRITICAL();
	
	if(spi_x == SPI1 && spi_busy[0] == SPI_OK)
	{
		spi_busy[0] = SPI_Err;
		
		result = SPI_OK;
	}
	else if(spi_x == SPI2 && spi_busy[1] == SPI_OK)
	{
		spi_busy[1] = SPI_Err;
		
		result = SPI_OK;
	}
	else if(spi_x == SPI3 && spi_busy[2] == SPI_OK)
	{
		spi_busy[2] = SPI_Err;
		
		result = SPI_OK;
	}
	
	RTOS_EXIT_CRITICAL();
	
	return result;

}

/*
************************************************************
*	�������ƣ�	SPI_FreeBus
*
*	�������ܣ�	�ͷ�����
*
*	��ڲ�����	spi_x��SPIx
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void SPIx_FreeBus(SPI_TypeDef *spi_x)
{

	RTOS_ENTER_CRITICAL();
	
	if(spi_x == SPI1)
		spi_busy[0] = SPI_OK;
	else if(spi_x == SPI2)
		spi_busy[1] = SPI_OK;
	else if(spi_x == SPI3)
		spi_busy[2] = SPI_OK;
	
	RTOS_EXIT_CRITICAL();

}

/*
SPI1:	NSS---PA4
		SCK---PA5
		MISO---PA6
		MOSI---PA7

SPI2:	NSS---PB12
		SCK---PB13
		MISO---PB14
		MOSI---PB15
*/

/*
************************************************************
*	�������ƣ�	SPIx_Init
*
*	�������ܣ�	SPIx��ʼ��
*
*	��ڲ�����	spi_x��SPIx
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void SPIx_Init(SPI_TypeDef *spi_x)
{
	
	MCU_SPIx_Init(spi_x, SPI_Direction_2Lines_FullDuplex, SPI_Mode_Master, SPI_DataSize_8b,
					SPI_CPOL_High, SPI_CPHA_2Edge, SPI_NSS_Soft, SPI_BaudRatePrescaler_2, SPI_FirstBit_MSB, 7);
 
	SPI_Cmd(spi_x, ENABLE);												//ʹ��SPI����
	
#if(SPI_DMA_TX_EN == 1)
	MCU_DMA_Init(DMA1, DMA1_Channel3, (unsigned int)&spi_x->DR, 0, DMA_DIR_PeripheralDST, 0,
					DMA_PeripheralInc_Disable, DMA_MemoryInc_Enable, DMA_PeripheralDataSize_Byte,
					DMA_MemoryDataSize_Byte, DMA_Mode_Normal, DMA_Priority_Medium, DMA_M2M_Disable);
	
	SPI_I2S_DMACmd(spi_x, SPI_I2S_DMAReq_Tx, DISABLE);					//ʧ��SPI-DMA
#endif

#if(SPI_DMA_RX_EN == 1)
	MCU_DMA_Init(DMA1, DMA1_Channel2, (unsigned int)&spi_x->DR, 0, DMA_DIR_PeripheralSRC, 0,
					DMA_PeripheralInc_Disable, DMA_MemoryInc_Enable, DMA_PeripheralDataSize_Byte,
					DMA_MemoryDataSize_Byte, DMA_Mode_Normal, DMA_Priority_Medium, DMA_M2M_Disable);
	
	SPI_I2S_DMACmd(spi_x, SPI_I2S_DMAReq_Rx, DISABLE);					//ʧ��SPI-DMA
#endif

}

/*
************************************************************
*	�������ƣ�	SPIx_TX_Init
*
*	�������ܣ�	SPIx��ʼ��
*
*	��ڲ�����	spi_x��SPIx
*
*	���ز�����	��
*
*	˵����		ֻ����ģʽ
************************************************************
*/
void SPIx_TX_Init(SPI_TypeDef *spi_x)
{
	
	MCU_SPIx_Init(spi_x, SPI_Direction_1Line_Tx, SPI_Mode_Master, SPI_DataSize_8b,
					SPI_CPOL_High, SPI_CPHA_2Edge, SPI_NSS_Soft, SPI_BaudRatePrescaler_2, SPI_FirstBit_MSB, 7);
 
	SPI_Cmd(spi_x, ENABLE);												//ʹ��SPI����
	
//#if(SPI_DMA_TX_EN == 1)
//	MCU_DMA_Init(DMA1, DMA1_Channel5, (unsigned int)&spi_x->DR, 0, DMA_DIR_PeripheralDST, 0,
//					DMA_PeripheralInc_Disable, DMA_MemoryInc_Enable, DMA_PeripheralDataSize_Byte,
//					DMA_MemoryDataSize_Byte, DMA_Mode_Circular, DMA_Priority_Medium, DMA_M2M_Disable);
//	
//	SPI_I2S_DMACmd(SPIx, SPI_I2S_DMAReq_Tx, DISABLE);					//ʧ��SPI-DMA
//#endif

}

/*
************************************************************
*	�������ƣ�	SPIx_ResetMemoryBaseAddr
*
*	�������ܣ�	����DMA�ڴ��ַ��ʹ�ܷ���
*
*	��ڲ�����	spi_x��SPI��
*				mem_addr���ڴ��ֵַ
*				num�����η��͵����ݳ���(�ֽ�)
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void SPIx_ResetMemoryBaseAddr(SPI_TypeDef *spi_x, unsigned int mem_addr, unsigned short num, _Bool type)
{

#if(SPI_DMA_TX_EN == 1)
	if(type == SPI_TX_TYPE)
	{
		if(spi_x == SPI1)
			MCU_DMA_ResetMemoryBaseAddr(DMA1_Channel3, mem_addr, num);
		else if(spi_x == SPI2)
			MCU_DMA_ResetMemoryBaseAddr(DMA1_Channel5, mem_addr, num);
	}
#endif
	
#if(SPI_DMA_RX_EN == 1)
	if(type == SPI_RX_TYPE)
	{
		if(spi_x == SPI1)
			MCU_DMA_ResetMemoryBaseAddr(DMA1_Channel2, mem_addr, num);
		else if(spi_x == SPI2)
			MCU_DMA_ResetMemoryBaseAddr(DMA1_Channel4, mem_addr, num);
	}
#endif

}

/*
************************************************************
*	�������ƣ�	SPIx_SetSpeed
*
*	�������ܣ�	SPI �ٶ����ú���
*
*	��ڲ�����	spi_x��SPI1����SPI2
*				speed���ٶȣ���˵��
*
*	���ز�����	��
*
*	˵����		SPI_BaudRatePrescaler_2   2��Ƶ   (SPI 36M@sys 72M)
*				SPI_BaudRatePrescaler_8   8��Ƶ   (SPI 9M@sys 72M)
*				SPI_BaudRatePrescaler_16  16��Ƶ  (SPI 4.5M@sys 72M)
*				SPI_BaudRatePrescaler_256 256��Ƶ (SPI 281.25K@sys 72M)
************************************************************
*/
void SPIx_SetSpeed(SPI_TypeDef *spi_x, unsigned char speed)
{
	
	
	
}

/*
************************************************************
*	�������ƣ�	SPIx_ReadWriteByte
*
*	�������ܣ�	��дһ���ֽ�
*
*	��ڲ�����	spi_x��SPI1����SPI2
*				tx_data:Ҫд����ֽ�
*
*	���ز�����	��ȡ�����ֽ�
*
*	˵����		
************************************************************
*/
unsigned char SPIx_ReadWriteByte(SPI_TypeDef *spi_x, unsigned char tx_data)
{
	
	unsigned char retry = 0;
	
	while(SPI_I2S_GetFlagStatus(spi_x, SPI_I2S_FLAG_TXE) == RESET)		//���ָ����SPI��־λ�������:���ͻ���ձ�־λ
	{
		retry++;
		if(retry > 200)
			return 0;
	}
	
	SPI_I2S_SendData(spi_x, tx_data);									//ͨ������SPIx����һ������
	retry = 0;

	while(SPI_I2S_GetFlagStatus(spi_x, SPI_I2S_FLAG_RXNE) == RESET)		//���ָ����SPI��־λ�������:���ܻ���ǿձ�־λ
	{
		retry++;
		if(retry > 200)
			return 0;
	}
	
	return SPI_I2S_ReceiveData(spi_x);									//����ͨ��SPIx������յ�����
		
}

/*
************************************************************
*	�������ƣ�	SPIx_DMA_ReadWriteByte
*
*	�������ܣ�	��дһ���ֽ�
*
*	��ڲ�����	spi_x��SPIx
*				tx_buf�����ͻ���ָ��
*				rx_buf�����ջ���ָ��
*				tx_size�����ͻ����С
*				rx_size�����ջ����С
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void SPIx_DMA_ReadWriteByte(SPI_TypeDef *spi_x, unsigned char *tx_buf, unsigned char *rx_buf, unsigned short tx_size, unsigned short rx_size)
{
	
	SPIx_ResetMemoryBaseAddr(spi_x, (unsigned int)tx_buf, tx_size, SPI_TX_TYPE);
	SPIx_ResetMemoryBaseAddr(spi_x, (unsigned int)rx_buf, rx_size, SPI_RX_TYPE);
	
	SPI_I2S_DMACmd(spi_x, SPI_I2S_DMAReq_Tx, ENABLE);
	SPI_I2S_DMACmd(spi_x, SPI_I2S_DMAReq_Rx, ENABLE);
	
	if(spi_x == SPI1)										//�ȴ��������
	{
		while(DMA_GetFlagStatus(DMA1_IT_TC2) == RESET);
		DMA_ClearFlag(DMA1_IT_TC2);
	}
	else if(spi_x == SPI2)
	{
		while(DMA_GetFlagStatus(DMA1_IT_TC4) == RESET);
		DMA_ClearFlag(DMA1_IT_TC4);
	}
	
	SPI_I2S_DMACmd(spi_x, SPI_I2S_DMAReq_Tx, DISABLE);
	SPI_I2S_DMACmd(spi_x, SPI_I2S_DMAReq_Rx, DISABLE);
		
}

/*
************************************************************
*	�������ƣ�	SPIx_WriteByte
*
*	�������ܣ�	дһ���ֽ�
*
*	��ڲ�����	spi_x��SPIx
*				tx_data:Ҫд����ֽ�
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void SPIx_WriteByte(SPI_TypeDef *spi_x, unsigned char tx_data)
{
	
	SPI_I2S_SendData(spi_x, tx_data);									//ͨ������SPIx����һ������
	
	while(SPI_I2S_GetFlagStatus(spi_x, SPI_I2S_FLAG_TXE) == RESET);		//���ָ����SPI��־λ�������:���ͻ���ձ�־λ
		
}
