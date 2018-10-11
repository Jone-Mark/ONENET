#include "stm32f10x.h"
#include "gpio.h"

void Led_Init(void)
{
	GPIO_InitTypeDef gpioInitStrcut;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOB, ENABLE);							//打开GPIOB的时钟
	
	gpioInitStrcut.GPIO_Mode = GPIO_Mode_Out_PP;			//设置为推挽输出模式
	gpioInitStrcut.GPIO_Pin = LED0_Pin;		          //初始化PE5 
	gpioInitStrcut.GPIO_Speed = GPIO_Speed_50MHz;									//承载的最大频率
	GPIO_Init(LED0_Port, &gpioInitStrcut);												//初始化GPIOB
	
	gpioInitStrcut.GPIO_Pin = LED1_Pin;		
  GPIO_Init(LED1_Port, &gpioInitStrcut);												//初始化GPIOB
  
  LED0_OFF;
  LED1_OFF;
}

void Beep_Init(void)
{

	GPIO_InitTypeDef gpioInitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		//打开GPIOA的时钟
	
	gpioInitStruct.GPIO_Mode = GPIO_Mode_Out_PP;				//设置为输出
	gpioInitStruct.GPIO_Pin = GPIO_Pin_8;						//将初始化的Pin脚
	gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;				//可承载的最大频率
	
	GPIO_Init(GPIOB, &gpioInitStruct);							//初始化GPIO
	
	BEEP_OFF;											//初始化完成后，关闭蜂鸣器
}

