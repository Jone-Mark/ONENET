/**************************************************************
*	�ļ����� 	hwtimer.c
*	���ߣ� 		�ż���
*	���ڣ� 		2016-11-23
*	�汾�� 		V1.0
*	˵���� 		��Ƭ����ʱ����ʼ��
*	�޸ļ�¼��	
**************************************************************/

//Э���
#include "onenet.h"

//�����豸
#include "net_device.h"

//Ӳ������
#include "hwtimer.h"
#include "selfcheck.h"

unsigned short timerCount = 0;	//ʱ�����--��λ��

TIM_INFO timInfo = {0};

/************************************************************
*	�������ƣ�	Timer3_4_Init
*	�������ܣ�	Timer3��4�Ķ�ʱ����
*	��ڲ�����	TIMx��TIM3 ���� TIM4
*				arr������ֵ
*				psc��Ƶֵ
*
*	���ز�����	��
*	˵����		timer3��timer4ֻ���и����жϹ���
************************************************************/
void Timer3_4_Init(TIM_TypeDef * TIMx, unsigned short arr, unsigned short psc)
{

	TIM_TimeBaseInitTypeDef timerInitStruct;
	NVIC_InitTypeDef nvicInitStruct;
	
	if(TIMx == TIM3)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
		nvicInitStruct.NVIC_IRQChannel = TIM3_IRQn;
	}
	else
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
		nvicInitStruct.NVIC_IRQChannel = TIM4_IRQn;
	}
	
	timerInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	timerInitStruct.TIM_Period = arr;
	timerInitStruct.TIM_Prescaler = psc;
	
	TIM_TimeBaseInit(TIMx, &timerInitStruct);
	
	TIM_ITConfig(TIMx, TIM_IT_Update, ENABLE); //ʹ�ܸ����ж�
	
	nvicInitStruct.NVIC_IRQChannelCmd = ENABLE;
	nvicInitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	nvicInitStruct.NVIC_IRQChannelSubPriority = 1;
	
	NVIC_Init(&nvicInitStruct);
	
	TIM_Cmd(TIMx, ENABLE); //ʹ�ܶ�ʱ��
}

/************************************************************
*	�������ƣ�	TIM3_IRQHandler
*	�������ܣ�	RTOS��������ʱ�ж�
*	��ڲ�����	��
*	���ز�����	��
*	˵����		
************************************************************/
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
	{
		//do something...
		if(++timInfo.timer3Out >= 4294967290UL)
			timInfo.timer3Out = 0;
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}
}

/************************************************************
*	�������ƣ�	TIM4_IRQHandler
*	�������ܣ�	Timer7�����жϷ�����
*	��ڲ�����	��
*	���ز�����	��
*	˵����		
************************************************************/
void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4, TIM_IT_Update) == SET)
	{
		if(oneNetInfo.netWork == 0)											//�������Ͽ�
		{
			if(++timerCount >= NET_TIME) 									//�������Ͽ���ʱ
			{	
				checkInfo.NET_DEVICE_OK = 0;								//���豸δ����־
				
				NET_DEVICE_ReConfig(0);										//�豸��ʼ����������Ϊ��ʼ״̬
				
				oneNetInfo.netWork = 0;
			}
		}
		else
		{
			timerCount = 0;													//�������
		}
		
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
	}

}
