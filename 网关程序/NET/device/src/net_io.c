/**************************************************************
*	文件名： 	net_IO.c
*	作者： 		张继瑞
*	日期： 		2017-06-25
*	版本： 		V1.2
*	说明： 		网络设备数据IO层
*	修改记录：	V1.1：增加DMA发送功能
*				V1.2：增加DMA接收功能、IDLE中断
************************************************************/

//单片机头文件
#include "stm32f10x.h"

//网络设备数据IO
#include "net_io.h"

//硬件驱动
#include "delay.h"
#include "usart.h"

NET_IO_INFO netIOInfo = {0, 0, REV_WAIT, {0}, (void *)0, (void *)0};

/*************************************************************
*	函数名称：	NET_IO_Init
*	函数功能：	初始化网络设备IO驱动层
*	入口参数：	无
*	返回参数：	无
*	说明：		底层的数据收发驱动
************************************************************/
void NET_IO_Init(void)
{
	Usart3_Init(115200);
	USARTx_ResetMemoryBaseAddr(NET_IO, (unsigned int)netIOInfo.buf, sizeof(netIOInfo.buf), USART_RX_TYPE);
	
	NET_IO_ClearRecive();
}

/*************************************************************
*	函数名称：	NET_IO_Send
*	函数功能：	发送数据
*	入口参数：	str：需要发送的数据
*				len：数据长度
*	返回参数：	无
*	说明：		底层的数据发送驱动
************************************************************/
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
	
	while(DMA_GetFlagStatus(DMA1_FLAG_TC2) == RESET);					//等待通道2传输完成
	DMA_ClearFlag(DMA1_FLAG_TC2);										//清除通道2传输完成标志
	
	USARTx_ResetMemoryBaseAddr(NET_IO, mAddr, len, USART_TX_TYPE);
#endif

}

/************************************************************
*	函数名称：	NET_IO_WaitRecive
*	函数功能：	等待接收完成
*	入口参数：	无
*	返回参数：	REV_OK-接收完成		REV_WAIT-接收超时未完成
*	说明：		循环调用检测是否接收完成
************************************************************/
_Bool NET_IO_WaitRecive(void)
{

#if(USART_DMA_RX_EN == 0)
	if(netIOInfo.dataLen == 0) 						//如果接收计数为0 则说明没有处于接收数据中，所以直接跳出，结束函数
		return REV_WAIT;
		
	if(netIOInfo.dataLen == netIOInfo.dataLenPre)	//如果上一次的值和这次相同，则说明接收完毕
	{
		netIOInfo.dataLen = 0;						//清0接收计数
			
		return REV_OK;								//返回接收完成标志
	}
		
	netIOInfo.dataLenPre = netIOInfo.dataLen;		//置为相同
	
	return REV_WAIT;								//返回接收未完成标志
#else
	_Bool status = netIOInfo.rev_idle;
	
	netIOInfo.rev_idle = REV_WAIT;
	
	return status;
#endif

}

/************************************************************
*	函数名称：	NET_IO_ClearRecive
*	函数功能：	清空缓存
*	入口参数：	无
*	返回参数：	无
*	说明：		
************************************************************/
void NET_IO_ClearRecive(void)
{
	unsigned short i = 0;

	netIOInfo.dataLen = 0;
	
	for(; i < sizeof(netIOInfo.buf); i++)
		netIOInfo.buf[i] = 0;
}

/************************************************************
*	函数名称：	USART3_IRQHandler
*	函数功能：	接收中断
*	入口参数：	无
*	返回参数：	无
*	说明：		
************************************************************/
void USART3_IRQHandler(void)
{
	
#if(USART_DMA_RX_EN == 0)
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) //接收中断
	{
		if(netIOInfo.dataLen >= sizeof(netIOInfo.buf))	netIOInfo.dataLen = 0;	//防止串口被刷爆
		netIOInfo.buf[netIOInfo.dataLen++] = USART3->DR;
		
		USART_ClearFlag(USART3, USART_FLAG_RXNE);
	}
#else
	if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)
	{
		netIOInfo.rev_idle = REV_OK;
		
		USART3->DR;																//读取数据注意：这句必须要，否则不能够清除中断标志位
		USART_ClearFlag(USART3, USART_IT_IDLE);
		
		DMA_Cmd(DMA1_Channel3, DISABLE);
		
		DMA1_Channel3->CNDTR = sizeof(netIOInfo.buf);							//重新设置下次接收的长度，否则无法启动下次DMA接收
		
		DMA_Cmd(DMA1_Channel3, ENABLE);
	}
#endif
}
