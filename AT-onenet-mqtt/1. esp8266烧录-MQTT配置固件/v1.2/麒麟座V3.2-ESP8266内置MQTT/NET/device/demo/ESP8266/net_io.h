#ifndef _NET_IO_H_
#define _NET_IO_H_





#define RB_BUFFER_SIZE		512

#define RB_BUFFER2_SIZE		512


typedef struct
{

/*************************ѭ������*************************/
	unsigned char buf[RB_BUFFER_SIZE];		//���ջ���
	unsigned char buf_tp[RB_BUFFER2_SIZE];	//�������棬һ֡���ݳ�����rbĩβ���׵�ʱ�򣬰����ݸ��Ƶ��������棬�Է����ϲ����������
	unsigned short write_pos;				//д
	unsigned short read_pos;				//��
	unsigned short data_packet;				//���ݰ�

} NET_IO_INFO;


#define NET_IO		USART2


void NET_IO_Init(void);

void NET_IO_Send(unsigned char *str, unsigned short len);

unsigned char *NET_IO_Read(void);


#endif
