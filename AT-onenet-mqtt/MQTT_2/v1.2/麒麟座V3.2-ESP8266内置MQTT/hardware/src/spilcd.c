/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	spilcd.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2017-11-23
	*
	*	�汾�� 		V1.1
	*
	*	˵���� 		SPI TFT-LCD����
	*
	*	�޸ļ�¼��	V1.1�������Դ���ƣ�ͨ��spilcd.h��ĺ����ʹ�ܻ�ʹ��
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"

//Ӳ������
#include "delay.h"
#include "spi.h"
#include "spilcd.h"
#include "hwtimer.h"
#include "light.h"

//C��
#include <stdarg.h>
#include <stdio.h>


SPILCD_INFO spilcd_info;


/*
************************************************************
*	�������ƣ�	SPILCD_Delay
*
*	�������ܣ�	��ʱ
*
*	��ڲ�����	time����ʱʱ��
*
*	���ز�����	��
*
*	˵����		���ڵ�ǰ��ʱʱ��
************************************************************
*/
__inline static void SPILCD_Delay(unsigned int time)
{

	RTOS_TimeDly(time);

}

/*
************************************************************
*	�������ƣ�	SPILCD_SendByte
*
*	�������ܣ�	����һ���ֽ�
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
static void SPILCD_SendByte(unsigned char byte)
{

	SPIx_WriteByte(SPILCD_IO, byte);

}

static void SPILCD_WriteData(unsigned char data)
{

	LCD_CS_L;
	LCD_AO_H;
	SPILCD_SendByte(data);
	LCD_CS_H;

}

static void SPILCD_WriteData16(unsigned short data)
{

	LCD_CS_L;
	LCD_AO_H;
	SPILCD_SendByte(data >> 8);
	SPILCD_SendByte(data & 0xff);
	LCD_CS_H;

}

static void SPILCD_WriteReg(unsigned char reg)
{

	LCD_CS_L;
	LCD_AO_L;
	SPILCD_SendByte(reg);
	LCD_CS_H;

}

static void SPILCD_WriteRegData(unsigned char reg, unsigned short data)
{

	SPILCD_WriteReg(reg);
	SPILCD_WriteData16(data);

}

/*
************************************************************
*	�������ƣ�	SPILCD_SetAddress
*
*	�������ܣ�	������ʾ��ַ����
*
*	��ڲ�����	x_start��X����ʼ��ַ
*				y_start��Y����ʼ��ַ
*				x_end��X�������ַ
*				y_end��Y�������ַ
*
*	���ز�����	��
*
*	˵����		�ڴ�������д����ʱ����ַ�Զ�����
************************************************************
*/
static void SPILCD_SetAddress(unsigned short x_start, unsigned short y_start, unsigned short x_end, unsigned short y_end)
{

	SPILCD_WriteReg(0x2a);
	SPILCD_WriteData(x_start >> 8);
	SPILCD_WriteData(0x00FF & x_start);
	SPILCD_WriteData(x_end >> 8);
	SPILCD_WriteData(0x00FF & x_end);
  
	SPILCD_WriteReg(0x2b);
	SPILCD_WriteData(y_start >> 8);
	SPILCD_WriteData(0x00FF & y_start);
	SPILCD_WriteData(y_end >> 8);
	SPILCD_WriteData(0x00FF & y_end);

	SPILCD_WriteReg(0x2C);

}

void SPILCD_Rst(void)
{

	LCD_RST_ON;
	SPILCD_Delay(20);
	LCD_RST_OFF;
	SPILCD_Delay(10);

}

/*
************************************************************
*	�������ƣ�	SPILCD_Init
*
*	�������ܣ�	SPILCD��ʼ��
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		PA8-����	PB12-CS		PB14-MISO	PB15-MOSI
*				PC9-RST		PB13-CLK	
************************************************************
*/
void SPILCD_Init(void)
{

	GPIO_InitTypeDef gpio_initstruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);				//��ֹJTAG����
	
	//CS
	gpio_initstruct.GPIO_Mode = GPIO_Mode_Out_PP;

	SPIx_TX_Init(SPILCD_IO);
	
	gpio_initstruct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_14;
	gpio_initstruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpio_initstruct);
	
	//��λ
	gpio_initstruct.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOB, &gpio_initstruct);
	
	//����
	Timer_X_PWM_Init(TIM3, 8, TIM_OCMode_PWM2, GPIOB, GPIO_Pin_1, 1799, 249);
	
	SPILCD_Rst();
	
	SPILCD_WriteReg(0x11);		//sleep out
	SPILCD_Delay(25);

	//------------------------------------ST7735S Frame Rate-----------------------------------------// 
	SPILCD_WriteReg(0xB1); 
	SPILCD_WriteData(0x05); 
	SPILCD_WriteData(0x3C); 
	SPILCD_WriteData(0x3C); 
	SPILCD_WriteReg(0xB2); 
	SPILCD_WriteData(0x05); 
	SPILCD_WriteData(0x3C); 
	SPILCD_WriteData(0x3C); 
	SPILCD_WriteReg(0xB3); 
	SPILCD_WriteData(0x05); 
	SPILCD_WriteData(0x3C); 
	SPILCD_WriteData(0x3C); 
	SPILCD_WriteData(0x05); 
	SPILCD_WriteData(0x3C); 
	SPILCD_WriteData(0x3C); 
	//------------------------------------End ST7735S Frame Rate-----------------------------------------// 
	SPILCD_WriteReg(0xB4); //Dot inversion 
	SPILCD_WriteData(0x03); 

	//------------------------------------ST7735S Power Sequence-----------------------------------------// 
	SPILCD_WriteReg(0xC0); 
	SPILCD_WriteData(0x28); 
	SPILCD_WriteData(0x08); 
	SPILCD_WriteData(0x04); 
	SPILCD_WriteReg(0xC1); 
	SPILCD_WriteData(0XC0); 
	SPILCD_WriteReg(0xC2); 
	SPILCD_WriteData(0x0D); 
	SPILCD_WriteData(0x00); 
	SPILCD_WriteReg(0xC3); 
	SPILCD_WriteData(0x8D); 
	SPILCD_WriteData(0x2A); 
	SPILCD_WriteReg(0xC4); 
	SPILCD_WriteData(0x8D); 
	SPILCD_WriteData(0xEE); 
	//---------------------------------End ST7735S Power Sequence-------------------------------------// 
	 
	SPILCD_WriteReg(0xC5); //VCOM 
	SPILCD_WriteData(0x1A); 
	 
	SPILCD_WriteReg(0x36); //MX, MY, RGB mode 
	SPILCD_WriteData(0x08); 
	//------------------------------------ST7735S Gamma Sequence-----------------------------------------// 
	 
	SPILCD_WriteReg(0xE0); 
	SPILCD_WriteData(0x04); 
	SPILCD_WriteData(0x22); 
	SPILCD_WriteData(0x07); 
	SPILCD_WriteData(0x0A); 
	SPILCD_WriteData(0x2E); 
	SPILCD_WriteData(0x30); 
	SPILCD_WriteData(0x25); 
	SPILCD_WriteData(0x2A); 
	SPILCD_WriteData(0x28); 
	SPILCD_WriteData(0x26); 
	SPILCD_WriteData(0x2E); 
	SPILCD_WriteData(0x3A); 
	SPILCD_WriteData(0x00); 

	SPILCD_WriteData(0x01); 
	SPILCD_WriteData(0x03); 
	SPILCD_WriteData(0x13); 
	SPILCD_WriteReg(0xE1); 
	SPILCD_WriteData(0x04); 
	SPILCD_WriteData(0x16); 
	SPILCD_WriteData(0x06); 
	SPILCD_WriteData(0x0D); 
	SPILCD_WriteData(0x2D); 
	SPILCD_WriteData(0x26); 
	SPILCD_WriteData(0x23); 
	SPILCD_WriteData(0x27); 
	SPILCD_WriteData(0x27); 
	SPILCD_WriteData(0x25); 
	SPILCD_WriteData(0x2D); 
	SPILCD_WriteData(0x3B); 
	SPILCD_WriteData(0x00); 
	SPILCD_WriteData(0x01); 
	SPILCD_WriteData(0x04); 
	SPILCD_WriteData(0x13); 
	//------------------------------------End ST7735S Gamma Sequence-----------------------------------------// 

	SPILCD_WriteReg(0x3A); //65k mode 
	SPILCD_WriteData(0x05); 
	SPILCD_WriteReg(0x29); //Display on 
	SPILCD_WriteReg(0x2c);
	
	SPILCD_BL_Ctl(250);

}

void SPILCD_BL_Ctl(unsigned char value)
{

	if(value > 250)
		value = 250;
	
	LCD_BL_SET(value);
	
	spilcd_info.blSta = value;

}

/*
************************************************************
*	�������ƣ�	SPILCD_BL_Ctl_Auto
*
*	�������ܣ�	���ݻ�����ǿ�Զ����ȵ���
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void SPILCD_BL_Ctl_Auto(void)
{

	unsigned char count = 0;
	unsigned char pwm = 0;
	unsigned char temp = 0;
	
	pwm = 75 * light_info.voltag;		//���������������

	if(spilcd_info.blSta > pwm)			//��С����
	{
		temp = spilcd_info.blSta - pwm;
		
		if(temp > 15)					//������15���ڵĲ�����
		{
			for(count = 0; count < temp; count++)	//��һ���������ȵĽ������
			{
				SPILCD_BL_Ctl(spilcd_info.blSta);	//����һ�ε�ֵ��ʼ��������
				if(--spilcd_info.blSta <= 5)		//���С�����������������
					break;
					
				SPILCD_Delay(3);
			}
		}
	}
	else if(spilcd_info.blSta < pwm)	//��������
	{
		temp = pwm - spilcd_info.blSta;
		
		if(temp > 15)					//������15���ڵĲ�����
		{
			for(count = 0; count < temp; count++)	//��һ���������ȵĽ������
			{
				SPILCD_BL_Ctl(spilcd_info.blSta);	//����һ�ε�ֵ��ʼ��������
				if(++spilcd_info.blSta >= 250)		//��������������������
					break;
				
				SPILCD_Delay(3);
			}
		}
	}
	
	SPILCD_BL_Ctl(spilcd_info.blSta);

}

/*
************************************************************
*	�������ƣ�	LCD_DrawPoint
*
*	�������ܣ�	����
*
*	��ڲ�����	x��X��λ��
*				y��Y��λ��
*				color����ɫֵ
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void SPILCD_DrawPoint(unsigned short x, unsigned short y, unsigned short color)
{
	
	if(x < TFTLCD_X_SIZE && y < TFTLCD_Y_SIZE)
	{
		SPILCD_SetAddress(x, y, x, y);
		SPILCD_WriteData16(color);
	}

}

/*
************************************************************
*	�������ƣ�	SPILCD_FillRect
*
*	�������ܣ�	��Rect
*
*	��ڲ�����	x_start��X����ʼ��ַ
*				y_start��Y����ʼ��ַ
*				x_end��X�������ַ
*				y_end��Y�������ַ
*				color����ɫֵ
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void SPILCD_FillRect(unsigned short x_start, unsigned short y_start, unsigned short x_end, unsigned short y_end, unsigned short color)
{
	
	while(SPIx_IsBusReady(SPILCD_IO) == SPI_Err)
		SPILCD_Delay(1);
	
	if(x_end < TFTLCD_X_SIZE && y_end < TFTLCD_Y_SIZE)
	{
		unsigned short i = 0, j = 0;
		
		SPILCD_SetAddress(x_start, y_start, x_end, y_end);
		
		for(; i <= y_end; i++)
		{
			for(j = 0; j <= x_end; j++)
			{
				SPILCD_WriteData16(color);
			}
		}
	}
	
	SPIx_FreeBus(SPILCD_IO);

}

/*
************************************************************
*	�������ƣ�	SPILCD_Draw16BPP
*
*	�������ܣ�	��ͼ
*
*	��ڲ�����	x��X��λ��
*				y��Y��λ��
*				color����ɫֵ
*				xy_size���������
*
*	���ز�����	��
*
*	˵����		ר��GUIʹ��
************************************************************
*/
void SPILCD_Draw16BPP(unsigned short x, unsigned short y, const unsigned short *color, const unsigned short *pTrans,
						int xy_size, unsigned char mode)
{
	
	while(SPIx_IsBusReady(SPILCD_IO) == SPI_Err)
		SPILCD_Delay(1);
	
	SPILCD_SetAddress(x, y, x + xy_size, y);
	
	switch(mode)
	{
		case 0:
			for(; xy_size > 0; xy_size--)
				SPILCD_WriteData16(*(*color++ + pTrans));
		
		break;
		
		case 1:
			
			for(; xy_size > 0; xy_size--)
				SPILCD_WriteData16(*color++);
		
		break;
		
		case 2:

			for(; xy_size > 0; xy_size--)
			{
				if(*color)
					SPILCD_WriteData16(*(*color + pTrans));
				
				color++;
			}
		
		break;
			
		case 3:
			
			for(; xy_size > 0; xy_size--)
			{
				if(*color)
					SPILCD_WriteData16(*color);
				
				color++;
			}
		
		break;
	}
	
	SPIx_FreeBus(SPILCD_IO);

}

#if(USE_GUI == 0)

/***********************************************************************************************************
************************************************************************************************************


									����Ϊ��ʹ��ͼ��ϵͳʱר�õĺ���


************************************************************************************************************
************************************************************************************************************/


/*
************************************************************
*	�������ƣ�	LCD_Clear
*
*	�������ܣ�	�����Ļ
*
*	��ڲ�����	color����ɫֵ
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void SPILCD_Clear(unsigned short color)
{

	unsigned short i = 0, j = 0;
	
	while(SPIx_IsBusReady(SPILCD_IO) == SPI_Err)
		SPILCD_Delay(1);
	
	SPILCD_SetAddress(0, 0, TFTLCD_X_SIZE - 1, TFTLCD_Y_SIZE - 1);
	
	for(; i < TFTLCD_X_SIZE; i++)
	{
		for(j = 0; j < TFTLCD_Y_SIZE; j++)
		{
			SPILCD_WriteData16(color);
		}
	}
	
	SPIx_FreeBus(SPILCD_IO);

}

/*
************************************************************
*	�������ƣ�	SPILCD_DrawLine
*
*	�������ܣ�	����Ļ�ϻ�һ����
*
*	��ڲ�����	x1��X����ʼ��
*				y1��Y����ʼ��
*				x2��X�������
*				y2��Y�������
*				color����ɫֵ
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void SPILCD_DrawLine(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned short color)
{
	
	unsigned short t;
	int xerr = 0, yerr = 0, delta_x, delta_y, distance;
	int incx, incy, uRow, uCol;
	
	while(SPIx_IsBusReady(SPILCD_IO) == SPI_Err)
		SPILCD_Delay(1);
	
	delta_x = x2 - x1;										//������������
	delta_y = y2 - y1;
	uRow = x1;
	uCol = y1;

	if(delta_x > 0)
		incx = 1;											//���õ�������
	else if(delta_x == 0)
		incx = 0;											//��ֱ��
	else
	{
		incx = -1;
		delta_x = -delta_x;
	}

	if(delta_y > 0)
		incy = 1;
	else if(delta_y == 0)
		incy = 0;											//ˮƽ��
	else
	{
		incy = -1;
		delta_y = -delta_y;
	}

	if(delta_x > delta_y)
		distance = delta_x;									//ѡȡ��������������
	else
		distance = delta_y;

	for(t = 0; t <= distance + 1; t++)						//�������
	{
		SPILCD_DrawPoint(uRow, uCol, color);				//����
		xerr += delta_x ;
		yerr += delta_y ;

		if(xerr > distance)
		{
			xerr -= distance;
			uRow += incx;
		}

		if(yerr > distance)
		{
			yerr -= distance;
			uCol += incy;
		}
	}
	
	SPIx_FreeBus(SPILCD_IO);
	
}

/*
************************************************************
*	�������ƣ�	SPILCD_DrawRectangle
*
*	�������ܣ�	����Ļ�ϻ�һ�����ľ���
*
*	��ڲ�����	x1��X����ʼ��
*				y1��Y����ʼ��
*				x2��X�������
*				y2��Y�������
*				color����ɫֵ
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void SPILCD_DrawRectangle(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned short color)
{
	
	SPILCD_DrawLine(x1, y1, x2, y1, color);
	SPILCD_DrawLine(x1, y1, x1, y2, color);
	SPILCD_DrawLine(x1, y2, x2, y2, color);
	SPILCD_DrawLine(x2, y1, x2, y2, color);
	
}

/*
************************************************************
*	�������ƣ�	SPILCD_DrawCircle
*
*	�������ܣ�	����Ļ�ϻ�һ��Բ��
*
*	��ڲ�����	x0��Բ��X������
*				y0��Բ��Y������
*				r���뾶
*				color����ɫֵ
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void SPILCD_DrawCircle(unsigned short x0, unsigned short y0, unsigned char r, unsigned short color)
{
	
	int a, b;
	int di;
	
	while(SPIx_IsBusReady(SPILCD_IO) == SPI_Err)
		SPILCD_Delay(1);
	
	a = 0;
	b = r;
	di = 3 - (r << 1);       //�ж��¸���λ�õı�־

	while(a <= b)
	{
		SPILCD_DrawPoint(x0 + a, y0 - b, color);        //5
		SPILCD_DrawPoint(x0 + b, y0 - a, color);        //0
		SPILCD_DrawPoint(x0 + b, y0 + a, color);        //4
		SPILCD_DrawPoint(x0 + a, y0 + b, color);        //6
		SPILCD_DrawPoint(x0 - a, y0 + b, color);        //1
		SPILCD_DrawPoint(x0 - b, y0 + a, color);
		SPILCD_DrawPoint(x0 - a, y0 - b, color);        //2
		SPILCD_DrawPoint(x0 - b, y0 - a, color);        //7
		a++;

		//ʹ��Bresenham�㷨��Բ
		if(di < 0)
			di += 4 * a + 6;
		else
		{
			di += 10 + 4 * (a - b);
			b--;
		}
	}
	
	SPIx_FreeBus(SPILCD_IO);
	
}

/*
************************************************************
*	�������ƣ�	SPILCD_DrawPic
*
*	�������ܣ�	��ͼ
*
*	��ڲ�����	pic������ͼƬ��ָ�뻭ͼ
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void SPILCD_DrawPic(const unsigned char *pic)
{

	unsigned short i = 0, j = 0;
	
	while(SPIx_IsBusReady(SPILCD_IO) == SPI_Err)
		SPILCD_Delay(1);
	
	SPILCD_SetAddress(0, 0, TFTLCD_X_SIZE - 1, TFTLCD_Y_SIZE - 1);
	
	for(; i < TFTLCD_Y_SIZE; i++)
	{
		for(j = 0; j < TFTLCD_X_SIZE; j++)
		{
			SPILCD_WriteData16(*pic | (unsigned short)*(++pic) << 8);
			
			pic++;
		}
	}
	
	SPIx_FreeBus(SPILCD_IO);

}

extern const unsigned char asc2_1206[95][12];
extern const unsigned char asc2_1608[95][16];
extern const unsigned char asc2_2412[95][36];
/*
************************************************************
*	�������ƣ�	SPILCD_DisChar
*
*	�������ܣ�	��ʾһ��ASCII�ַ�
*
*	��ڲ�����	x��X����
*				y��Y����
*				size���ַ��ߴ�
*				color����ʾ����ɫ
*				mode��0-������ʾ		1-��������ʾ
*				Char���ַ�����
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void SPILCD_DisChar(unsigned short x, unsigned short y, unsigned char size, unsigned short color, _Bool mode, unsigned char Char)    //����ʾ��̬����  һ���ַ�ռ��12���ؿ�  24���ظ�
{

	unsigned char temp, t1, t;
	unsigned short y0 = y;
	unsigned char csize = ((size >> 3) + ((size % 8) ? 1 : 0)) * (size >> 1);	//�õ�����һ���ַ���Ӧ������ռ���ֽ���  12  16��û��  24��36

	Char -= 32;//�õ�ƫ�ƺ��ֵ��ASCII�ֿ��Ǵӿո�ʼȡģ������-' '���Ƕ�Ӧ�ַ����ֿ⣩

	for(t = 0; t < csize; t++)
	{
		if(size == 12)
			temp = asc2_1206[Char][t]; 	 //����1206����
		else if(size == 16)
			temp = asc2_1608[Char][t];	//����1608����
		else if(size == 24)
			temp = asc2_2412[Char][t];	//����2412����
		else
			return;						//û�е��ֿ�

		for(t1 = 0; t1 < 8; t1++)
		{
			if(mode)
				SPILCD_DrawPoint(x, y, BGC);    //д����ɫ����

			if(temp & 0x80)
				SPILCD_DrawPoint(x, y, color);

			temp <<= 1;
			y++;

			if((y - y0) == size)
			{
				y = y0;
				x++;
				break;
			}
		}
	}

}

/*
************************************************************
*	�������ƣ�	SPILCD_DisString
*
*	�������ܣ�	��ʾһ���ַ���
*
*	��ڲ�����	x��X����
*				y��Y����
*				size���ַ��ߴ�
*				color����ʾ����ɫ
*				mode��0-������ʾ		1-��������ʾ
*				...
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void SPILCD_DisString(unsigned short x, unsigned short y, unsigned char size, unsigned short color, unsigned char mode, char *fmt, ...)
{

	char spilcd_printbuf[256];
	va_list ap;
	char *str = spilcd_printbuf;

	while(SPIx_IsBusReady(SPILCD_IO) == SPI_Err)
		SPILCD_Delay(1);
	
	va_start(ap, fmt);
	vsprintf(spilcd_printbuf, fmt, ap);
	va_end(ap);

	while(*str != 0)
	{
		SPILCD_DisChar(x, y, size, color, mode, *str++);

		x += size >> 1;
	}
	
	SPIx_FreeBus(SPILCD_IO);

}

/*
************************************************************
*	�������ƣ�	SPILCD_DisZW
*
*	�������ܣ�	��ʾһ���ֺ���
*
*	��ڲ�����	x��X����
*				y��Y����
*				color����ɫֵ
*				zw���ֿ�����ָ��
*
*	���ز�����	��
*
*	˵����		�˺���ֻ��ʾ16x16������ȡ���ĺ���(Ϊ�˺�OLEDʹ����ͬ���ֿ�����)
*				��ʾ��ʽ��16x16��2�ţ�ÿ��16�У�ÿ��ÿ��Ϊһ���ֽ�
*
*				B0								B0
*				B1								B1
*				B2 B							B2 B
*				B3 Y							B3 Y
*				B4 T	....................	B4 T
*				B5 E							B5 E
*				B6 0							B6 15
*				B7								B7
*				
*				B0								B0
*				B1								B1
*				B2 B							B2 B
*				B3 Y							B3 Y
*				B4 T	....................	B4 T
*				B5 E							B5 E
*				B6 16							B6 31
*				B7								B7
*
************************************************************
*/
void SPILCD_DisZW(unsigned short x, unsigned short y, unsigned short color, const unsigned char *zw)
{
	
	unsigned char i = 0, j = 0, k = 0;
	unsigned char byte = 0;
	
	while(SPIx_IsBusReady(SPILCD_IO) == SPI_Err)
		SPILCD_Delay(1);
	
	for(; i < 2; i++)													//����
	{
		for(j = 0; j < 16; j++)											//����
		{
			byte = zw[j + (i << 4)];									//ȡһ���ֽ���ʾ��LCD�ϣ�16x16���֣���32�ֽ�
			for(k = 0; k < 8; k++)										//һ�������ֽ�����8λ��ÿλ����һ����
			{
				SPILCD_DrawPoint(x + j, y + k + (i << 3), BGC);			//д����ɫ����
				
				if(byte & 0x01)											//�˵������ݣ�����ʾ
					SPILCD_DrawPoint(x + j, y + k + (i << 3), color);
				
				byte >>= 1;												//��λ��ǰ
			}
		}
	}
	
	SPIx_FreeBus(SPILCD_IO);

}

#endif
