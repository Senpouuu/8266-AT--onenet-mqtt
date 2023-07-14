/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	mcu_spi.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2018-02-26
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		��Ƭ��SPI
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��������
#include "mcu_rcc.h"
#include "mcu_gpio.h"
#include "mcu_spi.h"


/*
************************************************************
*	�������ƣ�	MCU_SPIx_Init
*
*	�������ܣ�	��Ƭ��SPI��ʼ��
*
*	��ڲ�����	SPI_Group��SPI��
*				SPI_Direction�����䷽��
*				SPI_Mode������ģʽ
*				SPI_DataSize�����ݿ��
*				SPI_CPOL������ʱ�ӵ���̬
*				SPI_CPHA�����ݲ���ʱ����
*				SPI_NSS��NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������
*				SPI_BaudRatePrescaler��������Ԥ��Ƶ��ֵ
*				SPI_FirstBit�����ݴ����MSBλ����LSBλ��ʼ
*				SPI_CRCPolynomial��CRCֵ����Ķ���ʽ
*
*	���ز�����	0-�ɹ�	1-ʧ��
*
*	˵����		
************************************************************
*/
_Bool MCU_SPIx_Init(SPI_TypeDef *SPI_Group, unsigned short SPI_Direction, unsigned short SPI_Mode, unsigned short SPI_DataSize,
					unsigned short SPI_CPOL, unsigned short SPI_CPHA, unsigned short SPI_NSS, unsigned short SPI_BaudRatePrescaler,
					unsigned short SPI_FirstBit, unsigned short SPI_CRCPolynomial)
{
	
	_Bool result = 1;
	SPI_InitTypeDef spi_initstruct;

	MCU_RCC(SPI_Group, ENABLE);
	
	if(SPI_Group == SPI1)
	{
		MCU_GPIO_Init(GPIOA, GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7, GPIO_Mode_AF_PP, GPIO_Speed_50MHz, (void *)0);
		
		GPIO_SetBits(GPIOA, GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);
		
		result = 0;
	}
	else if(SPI_Group == SPI2)
	{
		MCU_GPIO_Init(GPIOB, GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15, GPIO_Mode_AF_PP, GPIO_Speed_50MHz, (void *)0);
		
		GPIO_SetBits(GPIOB, GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
		
		result = 0;
	}
	else if(SPI_Group == SPI3)
	{
		MCU_GPIO_SWJ_JTAGDisable();
		
		MCU_GPIO_Init(GPIOB, GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5, GPIO_Mode_AF_PP, GPIO_Speed_50MHz, (void *)0);
		
		GPIO_SetBits(GPIOB, GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5);
		
		result = 0;
	}
	
	if(result == 0)
	{
		spi_initstruct.SPI_Direction = SPI_Direction;					//����SPI�������˫�������ģʽ
		spi_initstruct.SPI_Mode = SPI_Mode;								//����SPI����ģʽ
		spi_initstruct.SPI_DataSize = SPI_DataSize;						//����SPI�����ݴ�С
		spi_initstruct.SPI_CPOL = SPI_CPOL;								//���ô���ʱ�ӵ���̬
		spi_initstruct.SPI_CPHA = SPI_CPHA;								//�������ݲ���ʱ����
		spi_initstruct.SPI_NSS = SPI_NSS;								//NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������
		spi_initstruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler;	//���岨����Ԥ��Ƶ��ֵ
		spi_initstruct.SPI_FirstBit = SPI_FirstBit;						//ָ�����ݴ����MSBλ����LSBλ��ʼ
		spi_initstruct.SPI_CRCPolynomial = SPI_CRCPolynomial;			//CRCֵ����Ķ���ʽ
		
		SPI_Init(SPI_Group, &spi_initstruct);							//��ʼ��
	}
	
	return result;

}

/*
************************************************************
*	�������ƣ�	MCU_SPIx_TX_Init
*
*	�������ܣ�	��Ƭ��SPI_TX��ʼ��
*
*	��ڲ�����	SPI_Group��SPI��
*				SPI_Direction�����䷽��
*				SPI_Mode������ģʽ
*				SPI_DataSize�����ݿ��
*				SPI_CPOL������ʱ�ӵ���̬
*				SPI_CPHA�����ݲ���ʱ����
*				SPI_NSS��NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������
*				SPI_BaudRatePrescaler��������Ԥ��Ƶ��ֵ
*				SPI_FirstBit�����ݴ����MSBλ����LSBλ��ʼ
*				SPI_CRCPolynomial��CRCֵ����Ķ���ʽ
*
*	���ز�����	0-�ɹ�	1-ʧ��
*
*	˵����		
************************************************************
*/
_Bool MCU_SPIx_TX_Init(SPI_TypeDef *SPI_Group, unsigned short SPI_Direction, unsigned short SPI_Mode, unsigned short SPI_DataSize,
					unsigned short SPI_CPOL, unsigned short SPI_CPHA, unsigned short SPI_NSS, unsigned short SPI_BaudRatePrescaler,
					unsigned short SPI_FirstBit, unsigned short SPI_CRCPolynomial)
{
	
	_Bool result = 1;
	SPI_InitTypeDef spi_initstruct;

	MCU_RCC(SPI_Group, ENABLE);
	
	if(SPI_Group == SPI1)
	{
		MCU_GPIO_Init(GPIOA, GPIO_Pin_5 | GPIO_Pin_7, GPIO_Mode_AF_PP, GPIO_Speed_50MHz, (void *)0);
		
		GPIO_SetBits(GPIOA, GPIO_Pin_5 | GPIO_Pin_7);
		
		result = 0;
	}
	else if(SPI_Group == SPI2)
	{
		MCU_GPIO_Init(GPIOB, GPIO_Pin_13 | GPIO_Pin_15, GPIO_Mode_AF_PP, GPIO_Speed_50MHz, (void *)0);
		
		GPIO_SetBits(GPIOB, GPIO_Pin_13 | GPIO_Pin_15);
		
		result = 0;
	}
	else if(SPI_Group == SPI3)
	{
		MCU_GPIO_Init(GPIOB, GPIO_Pin_3 | GPIO_Pin_5, GPIO_Mode_AF_PP, GPIO_Speed_50MHz, (void *)0);
		
		GPIO_SetBits(GPIOB, GPIO_Pin_3 | GPIO_Pin_5);
		
		result = 0;
	}
	
	if(result == 0)
	{
		spi_initstruct.SPI_Direction = SPI_Direction;					//����SPI�������˫�������ģʽ
		spi_initstruct.SPI_Mode = SPI_Mode;								//����SPI����ģʽ
		spi_initstruct.SPI_DataSize = SPI_DataSize;						//����SPI�����ݴ�С
		spi_initstruct.SPI_CPOL = SPI_CPOL;								//���ô���ʱ�ӵ���̬
		spi_initstruct.SPI_CPHA = SPI_CPHA;								//�������ݲ���ʱ����
		spi_initstruct.SPI_NSS = SPI_NSS;								//NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������
		spi_initstruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler;	//���岨����Ԥ��Ƶ��ֵ
		spi_initstruct.SPI_FirstBit = SPI_FirstBit;						//ָ�����ݴ����MSBλ����LSBλ��ʼ
		spi_initstruct.SPI_CRCPolynomial = SPI_CRCPolynomial;			//CRCֵ����Ķ���ʽ
		
		SPI_Init(SPI_Group, &spi_initstruct);							//��ʼ��
	}
	
	return result;

}
