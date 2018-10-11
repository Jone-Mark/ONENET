#ifndef _HWTIMER_H_
#define _HWTIMER_H_

#include "stm32f10x.h"

typedef struct
{

	unsigned int timer3Out;

} TIM_INFO;

extern TIM_INFO timInfo;

#define OS_TIMER	TIM3


#define NET_TIME	60			//设定时间--单位秒
extern unsigned short timerCount;

void Timer3_4_Init(TIM_TypeDef * TIMx, unsigned short arr, unsigned short psc);

void RTOS_TimerInit(void);


#endif
