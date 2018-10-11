#ifndef _NET_IO_H_
#define _NET_IO_H_


struct NET_SEND_LIST
{

	unsigned short dataLen;			//���ݳ���
	unsigned char *buf;				//����ָ��
	
	struct NET_SEND_LIST *next;		//��һ��

};

typedef struct
{
	
/*************************���ջ���*************************/
	unsigned short dataLen;			  //�������ݳ���
	unsigned short dataLenPre;		//��һ�εĳ������ݣ����ڱȽ�
	_Bool rev_idle;
	unsigned char buf[256];			  //���ջ���
	
/*************************���Ͷ���*************************/
	struct NET_SEND_LIST *head, *end;

} NET_IO_INFO;

#define REV_OK		1	//������ɱ�־
#define REV_WAIT	0	//����δ��ɱ�־

#define NET_IO		USART3

extern NET_IO_INFO netIOInfo;

void NET_IO_Init(void);

void NET_IO_Send(unsigned char *str, unsigned short len);

_Bool NET_IO_WaitRecive(void);

void NET_IO_ClearRecive(void);


#endif
