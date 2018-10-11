/**************************************************************
*	�ļ����� 	net_IO.c
*	���ߣ� 		�ż���
*	���ڣ� 		2017-06-25
*	�汾�� 		V1.2
*	˵���� 		�����豸����IO��
*	�޸ļ�¼��	V1.1������DMA���͹���
*				V1.2������DMA���չ��ܡ�IDLE�ж�
************************************************************/

//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"

//�����豸����IO
#include "net_io.h"

//Ӳ������
#include "delay.h"
#include "usart.h"

NET_IO_INFO netIOInfo = {0, 0, REV_WAIT, {0}, (void *)0, (void *)0};

/*************************************************************
*	�������ƣ�	NET_IO_Init
*	�������ܣ�	��ʼ�������豸IO������
*	��ڲ�����	��
*	���ز�����	��
*	˵����		�ײ�������շ�����
************************************************************/
void NET_IO_Init(void)
{
	Usart3_Init(115200);
	USARTx_ResetMemoryBaseAddr(NET_IO, (unsigned int)netIOInfo.buf, sizeof(netIOInfo.buf), USART_RX_TYPE);
	
	NET_IO_ClearRecive();
}

/*************************************************************
*	�������ƣ�	NET_IO_Send
*	�������ܣ�	��������
*	��ڲ�����	str����Ҫ���͵�����
*				len�����ݳ���
*	���ز�����	��
*	˵����		�ײ�����ݷ�������
************************************************************/
void NET_IO_Send(unsigned char *str, unsigned short len)
{
	
#if(USART_DMA_TX_EN == 0)
	unsigned short count = 0;
	
	for(; count < len; count++)
	{
		USART_SendData(NET_IO, *str++);									//��������
		while(USART_GetFlagStatus(NET_IO, USART_FLAG_TC) == RESET);		//�ȴ��������
	}
#else
	unsigned int mAddr = (unsigned int)str;
	
	while(DMA_GetFlagStatus(DMA1_FLAG_TC2) == RESET);					//�ȴ�ͨ��2�������
	DMA_ClearFlag(DMA1_FLAG_TC2);										//���ͨ��2������ɱ�־
	
	USARTx_ResetMemoryBaseAddr(NET_IO, mAddr, len, USART_TX_TYPE);
#endif

}

/************************************************************
*	�������ƣ�	NET_IO_WaitRecive
*	�������ܣ�	�ȴ��������
*	��ڲ�����	��
*	���ز�����	REV_OK-�������		REV_WAIT-���ճ�ʱδ���
*	˵����		ѭ�����ü���Ƿ�������
************************************************************/
_Bool NET_IO_WaitRecive(void)
{

#if(USART_DMA_RX_EN == 0)
	if(netIOInfo.dataLen == 0) 						//������ռ���Ϊ0 ��˵��û�д��ڽ��������У�����ֱ����������������
		return REV_WAIT;
		
	if(netIOInfo.dataLen == netIOInfo.dataLenPre)	//�����һ�ε�ֵ�������ͬ����˵���������
	{
		netIOInfo.dataLen = 0;						//��0���ռ���
			
		return REV_OK;								//���ؽ�����ɱ�־
	}
		
	netIOInfo.dataLenPre = netIOInfo.dataLen;		//��Ϊ��ͬ
	
	return REV_WAIT;								//���ؽ���δ��ɱ�־
#else
	_Bool status = netIOInfo.rev_idle;
	
	netIOInfo.rev_idle = REV_WAIT;
	
	return status;
#endif

}

/************************************************************
*	�������ƣ�	NET_IO_ClearRecive
*	�������ܣ�	��ջ���
*	��ڲ�����	��
*	���ز�����	��
*	˵����		
************************************************************/
void NET_IO_ClearRecive(void)
{
	unsigned short i = 0;

	netIOInfo.dataLen = 0;
	
	for(; i < sizeof(netIOInfo.buf); i++)
		netIOInfo.buf[i] = 0;
}

/************************************************************
*	�������ƣ�	USART3_IRQHandler
*	�������ܣ�	�����ж�
*	��ڲ�����	��
*	���ز�����	��
*	˵����		
************************************************************/
void USART3_IRQHandler(void)
{
	
#if(USART_DMA_RX_EN == 0)
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) //�����ж�
	{
		if(netIOInfo.dataLen >= sizeof(netIOInfo.buf))	netIOInfo.dataLen = 0;	//��ֹ���ڱ�ˢ��
		netIOInfo.buf[netIOInfo.dataLen++] = USART3->DR;
		
		USART_ClearFlag(USART3, USART_FLAG_RXNE);
	}
#else
	if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)
	{
		netIOInfo.rev_idle = REV_OK;
		
		USART3->DR;																//��ȡ����ע�⣺������Ҫ�������ܹ�����жϱ�־λ
		USART_ClearFlag(USART3, USART_IT_IDLE);
		
		DMA_Cmd(DMA1_Channel3, DISABLE);
		
		DMA1_Channel3->CNDTR = sizeof(netIOInfo.buf);							//���������´ν��յĳ��ȣ������޷������´�DMA����
		
		DMA_Cmd(DMA1_Channel3, ENABLE);
	}
#endif
}
