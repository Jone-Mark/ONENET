#include "stm32f10x.h"
#include "gpio.h"

void Led_Init(void)
{
	GPIO_InitTypeDef gpioInitStrcut;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOB, ENABLE);							//��GPIOB��ʱ��
	
	gpioInitStrcut.GPIO_Mode = GPIO_Mode_Out_PP;			//����Ϊ�������ģʽ
	gpioInitStrcut.GPIO_Pin = LED0_Pin;		          //��ʼ��PE5 
	gpioInitStrcut.GPIO_Speed = GPIO_Speed_50MHz;									//���ص����Ƶ��
	GPIO_Init(LED0_Port, &gpioInitStrcut);												//��ʼ��GPIOB
	
	gpioInitStrcut.GPIO_Pin = LED1_Pin;		
  GPIO_Init(LED1_Port, &gpioInitStrcut);												//��ʼ��GPIOB
  
  LED0_OFF;
  LED1_OFF;
}

void Beep_Init(void)
{

	GPIO_InitTypeDef gpioInitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		//��GPIOA��ʱ��
	
	gpioInitStruct.GPIO_Mode = GPIO_Mode_Out_PP;				//����Ϊ���
	gpioInitStruct.GPIO_Pin = GPIO_Pin_8;						//����ʼ����Pin��
	gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;				//�ɳ��ص����Ƶ��
	
	GPIO_Init(GPIOB, &gpioInitStruct);							//��ʼ��GPIO
	
	BEEP_OFF;											//��ʼ����ɺ󣬹رշ�����
}

