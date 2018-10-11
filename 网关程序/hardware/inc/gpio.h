#ifndef _LED_H_
#define _LED_H_

#define LED0_Pin GPIO_Pin_5
#define LED0_Port GPIOB

#define LED1_Pin GPIO_Pin_5
#define LED1_Port GPIOE

#define BEEP_Pin GPIO_Pin_8
#define BEEP_Port GPIOB

#define LED0_ON  GPIO_WriteBit(LED0_Port, LED0_Pin, Bit_RESET)		 
#define LED0_OFF GPIO_WriteBit(LED0_Port, LED0_Pin, Bit_SET)	

#define LED1_ON  GPIO_WriteBit(LED1_Port, LED1_Pin, Bit_RESET)		 //低电平为高
#define LED1_OFF GPIO_WriteBit(LED1_Port, LED1_Pin, Bit_SET)

void Led_Init(void);

#define BEEP_ON  GPIO_WriteBit(BEEP_Port, BEEP_Pin,Bit_SET)		 
#define BEEP_OFF GPIO_WriteBit(BEEP_Port, BEEP_Pin,Bit_RESET)	

void Beep_Init(void);

void Beep_Set(_Bool status);
#endif
