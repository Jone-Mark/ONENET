#ifndef _NET_IO_H_
#define _NET_IO_H_


struct NET_SEND_LIST
{

	unsigned short dataLen;			//数据长度
	unsigned char *buf;				//数据指针
	
	struct NET_SEND_LIST *next;		//下一个

};

typedef struct
{
	
/*************************接收缓存*************************/
	unsigned short dataLen;			  //接收数据长度
	unsigned short dataLenPre;		//上一次的长度数据，用于比较
	_Bool rev_idle;
	unsigned char buf[256];			  //接收缓存
	
/*************************发送队列*************************/
	struct NET_SEND_LIST *head, *end;

} NET_IO_INFO;

#define REV_OK		1	//接收完成标志
#define REV_WAIT	0	//接收未完成标志

#define NET_IO		USART3

extern NET_IO_INFO netIOInfo;

void NET_IO_Init(void);

void NET_IO_Send(unsigned char *str, unsigned short len);

_Bool NET_IO_WaitRecive(void);

void NET_IO_ClearRecive(void);


#endif
