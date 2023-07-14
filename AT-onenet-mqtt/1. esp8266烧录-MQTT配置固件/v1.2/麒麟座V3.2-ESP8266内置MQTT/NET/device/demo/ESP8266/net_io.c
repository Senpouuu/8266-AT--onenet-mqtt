/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	net_io.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-08-05
	*
	*	版本： 		V1.3
	*
	*	说明： 		网络设备数据IO层
	*
	*	修改记录：	V1.1：增加DMA发送功能
	*				V1.2：增加DMA接收功能、IDLE中断
	*				V1.3：引入RingBuffer机制
	*						说明：	rb写入：在串口接收里边循环写入。
	*								rb读取：核心思想是在一帧完整数
	*										据尾添加结束符，上层应
	*										用根据结束符来获取完整
	*										的数据。
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f10x.h"

//网络设备数据IO
#include "net_io.h"

//硬件驱动
#include "delay.h"
#include "usart.h"


NET_IO_INFO net_io_info = {{0}, {0}, 0, 0, 0};


const unsigned char tag[] = {0, 127, 255};			//自定义结束符
const unsigned char tag_num = sizeof(tag);


/*
************************************************************
*	函数名称：	NET_IO_Init
*
*	函数功能：	初始化网络设备IO驱动层
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		底层的数据收发驱动
************************************************************
*/
void NET_IO_Init(void)
{

	Usart2_Init(115200);
#if(USART_DMA_RX_EN == 1)
	USARTx_ResetMemoryBaseAddr(NET_IO, (unsigned int)net_io_info.buf, RB_BUFFER_SIZE, USART_RX_TYPE);
#endif

}

/*
************************************************************
*	函数名称：	NET_IO_AddTag2End
*
*	函数功能：	在每一帧数据末尾加上自定义的结束符
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		添加结束符，以方便上传处理一帧完整数据
************************************************************
*/
void NET_IO_AddTag2End(void)
{

	unsigned char i = 0;
	
	for(; i < tag_num; i++)								//在一帧数据尾添加结束符
	{
		net_io_info.buf[net_io_info.write_pos++] = tag[i];
		net_io_info.write_pos %= RB_BUFFER_SIZE;
	}
	
#if(USART_DMA_RX_EN == 1)
	DMA_Cmd(DMA1_Channel6, DISABLE);												//关闭DMA

	DMA_SetCurrDataCounter(DMA1_Channel6, RB_BUFFER_SIZE - net_io_info.write_pos);	//重新设置剩余字节
	DMA1_Channel6->CMAR = (unsigned int)(net_io_info.buf + net_io_info.write_pos);	//重新设置当前DMA指向内存的指针
	
	DMA_Cmd(DMA1_Channel6, ENABLE);													//使能MDA
#endif
	
	net_io_info.data_packet++;							//数据包指针增加

}

/*
************************************************************
*	函数名称：	NET_IO_CheckTag
*
*	函数功能：	获取完整的一帧数据大小
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		根据结束符来判断一帧数据的长度
************************************************************
*/
unsigned short NET_IO_CheckTag(void)
{
	
	unsigned short bytes = 0, read_pos = net_io_info.read_pos;
	unsigned char i = 0;
	
	while(i < tag_num)											//如果未找到完整结束符
	{
		if(net_io_info.buf[read_pos] == tag[i])					//找到了第一个
			i++;												//标记
		else													//如果和结束符不匹配
		{
			i = 0;												//从头检查
			if(net_io_info.buf[read_pos] == tag[i])				//比较当前这个是不是第一个结束符
				i++;
		}
		
		bytes++;
		read_pos++;
		
		bytes %= RB_BUFFER_SIZE;								//不超过数组范围
		read_pos %= RB_BUFFER_SIZE;
	}
	
	return bytes;

}

/*
************************************************************
*	函数名称：	NET_IO_Send
*
*	函数功能：	发送数据
*
*	入口参数：	str：需要发送的数据
*				len：数据长度
*
*	返回参数：	无
*
*	说明：		底层的数据发送驱动
*
************************************************************
*/
void NET_IO_Send(unsigned char *str, unsigned short len)
{
	
#if(USART_DMA_TX_EN == 0)
	unsigned short count = 0;
	
	for(; count < len; count++)
	{
		USART_SendData(NET_IO, *str++);									//发送数据
		while(USART_GetFlagStatus(NET_IO, USART_FLAG_TC) == RESET);		//等待发送完成
	}
#else
	unsigned int mAddr = (unsigned int)str;
	
	while(DMA_GetFlagStatus(DMA1_FLAG_TC7) == RESET);					//等待通道7传输完成
	DMA_ClearFlag(DMA1_FLAG_TC7);										//清除通道7传输完成标志
	
	USARTx_ResetMemoryBaseAddr(NET_IO, mAddr, len, USART_TX_TYPE);
#endif

}

/*
************************************************************
*	函数名称：	NET_IO_Read
*
*	函数功能：	读取接收的数据
*
*	入口参数：	无
*
*	返回参数：	获取本帧数据指针
*
*	说明：		
************************************************************
*/
unsigned char *NET_IO_Read(void)
{
	
	unsigned short read_num = 0, i = 0;
	unsigned char *buf = NULL;
	
	if(net_io_info.data_packet > 0)													//当有完整数据包可读时
	{
		if(--net_io_info.data_packet == 65535)
			net_io_info.data_packet = 0;
		
		read_num = NET_IO_CheckTag();												//计算需要读取的数据长度
		
		if(net_io_info.write_pos <= net_io_info.read_pos)							//有新数据、且循环了一圈*********************
		{
			if(net_io_info.read_pos + read_num < RB_BUFFER_SIZE)					//当前这帧数据还未超过rb
			{
				buf = net_io_info.buf + net_io_info.read_pos;
			}
			else																	//如果数据前段出现在rb尾，后段出现在rb首
			{
				unsigned short num = RB_BUFFER_SIZE - net_io_info.read_pos;			//计算rb尾需要读的数据长度
				
				if(read_num >= RB_BUFFER2_SIZE)										//如果大于二级缓存容量
					return NULL;
				
				for(i = 0; i < num; i++)											//复制rb尾的有效数据到二级缓存
					net_io_info.buf_tp[i] = net_io_info.buf[i + net_io_info.read_pos];
				
				for(i = 0; i < read_num - num; i++)									//复制rb首的有效数据到二级缓存
					net_io_info.buf_tp[i + num] = net_io_info.buf[i];
				
				buf = net_io_info.buf_tp;
			}
		}
		else																		//有新数据、且还未循环一圈*******************
		{
			buf = net_io_info.buf + net_io_info.read_pos;
		}
		
		net_io_info.read_pos += read_num;											//移动读指针
		net_io_info.read_pos %= RB_BUFFER_SIZE;										//确保读指针不超过该rb范围
	}
	
	return buf;

}

/*
************************************************************
*	函数名称：	DMA1_Channel6_IRQHandler
*
*	函数功能：	DMA1_通道6_中断函数
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void DMA1_Channel6_IRQHandler(void)
{

	if(DMA_GetITStatus(DMA1_IT_TC6) == SET)
	{
#if(USART_DMA_RX_EN == 1)
		net_io_info.write_pos = 0;
		USARTx_ResetMemoryBaseAddr(NET_IO, (unsigned int)net_io_info.buf, RB_BUFFER_SIZE, USART_RX_TYPE);
#endif
		DMA_ClearFlag(DMA1_IT_TC6);
	}

}

/*
************************************************************
*	函数名称：	USART2_IRQHandler
*
*	函数功能：	USART2中断
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void USART2_IRQHandler(void)
{
	
	RTOS_EnterInt();
	
#if(USART_DMA_RX_EN == 0)
	if(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) != RESET)			//接收中断
	{
		USART_ClearFlag(USART2, USART_FLAG_RXNE);
		
		net_io_info.buf[net_io_info.write_pos++] = USART2->DR;			//写入rb
		net_io_info.write_pos %= RB_BUFFER_SIZE;						//控制在rb范围内
	}
#endif
	
	if(USART_GetFlagStatus(USART2, USART_FLAG_IDLE) != RESET)			//空闲中断
	{
		USART2->DR;														//读取数据注意：这句必须要，否则不能够清除中断标志位
		USART_ClearFlag(USART2, USART_IT_IDLE);
		
#if(USART_DMA_RX_EN == 1)
		net_io_info.write_pos = RB_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA1_Channel6);
#endif
		
		NET_IO_AddTag2End();											//收到一帧完整数据，在数据尾添加结束符
	}
	
	RTOS_ExitInt();

}
