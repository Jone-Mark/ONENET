#include "main.h"

//�޸Ĵ˴���ֵ����ͬ�ڵ����ؽ��� //0-DHT11  1-���մ�����   2-CO2    3-����ʪ��  
#define SENSOR_TYPE 2

/*Zigbee����Э�� 
 0,1  ��ͷ                0xEE 0xCC
 2    �ڵ�����            0-���ƽڵ�  1-�������ڵ�  
 3    ����������          0-DHT11     1-���մ�����    2-������̼    3-����ʪ��  
 4    ���ݸ߰�λ
 5    ���ݵͰ�λ
 6,7  ��β                0x0D 0x0A
*/                        ///2
u8 SEND_BUFF[]={0xEE,0xCC,0xFF,0xFF,0xFF,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

int main(void)
{	
	delay_init();	    	 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	//LED_Init();
	uart_init(115200);
	TIM3_PWM_Init(40,1000);       	
	SystemInit();	
	
  #if(SENSOR_TYPE==1)	 //������ʪ��
	DHT11_Init();
	SEND_BUFF[2]=0x01;
	SEND_BUFF[5]=0x01;
	#elif(SENSOR_TYPE==2)	 //����ǿ��
	GPIOConfig();
	Init_BH1750(); 
	SEND_BUFF[2]=0x02;
	SEND_BUFF[5]=0x02;
	#elif(SENSOR_TYPE==3)	     //CO2�ɼ�
	USART2_Init(9600);
  SEND_BUFF[2]=0x03;
  SEND_BUFF[5]=0x03;	
  #elif(SENSOR_TYPE==4)  //�ɼ�����ʪ��
	Adc_Init();
  SEND_BUFF[2]=0x04;	
	SEND_BUFF[5]=0x04;
  #endif	
 	
	while(1)
	{
    pwm_on();
  #if(SENSOR_TYPE==1)	 //������ʪ��
		get_temp_humi_data();
	#elif(SENSOR_TYPE==2)	 //����ǿ��
		get_light_data();
	#elif(SENSOR_TYPE==3)	     //CO2�ɼ�
		get_co2_data();
	#elif(SENSOR_TYPE==4)  //�ɼ�����ʪ��
		get_soilhumi_data();
  #endif	
	}
}

void get_temp_humi_data(void)
{
	  DHT11_Read_Data(&temperature,&humidity);		
	  SEND_BUFF[3]=temperature;	
	  SEND_BUFF[4]=humidity;	
	  USART1_Send((char *)SEND_BUFF);
}

void get_co2_data(void)
{
		for (i=0;i<9;i++)//����CMDָ��
		{
			while((USART2->SR&0x40)==0);
			USART2->DR = Read_CO2[i];
		}
		delay_ms(1000);
		if(USART2_RX_STA&0X8000)		
    {
			i=1;
			   if( USART2_RX_BUF[0] == 0xFF && USART2_RX_BUF[1] == 0x86 )
			   {
					 value_co2=USART2_RX_BUF[2] * 256 + USART2_RX_BUF[3];
					 SEND_BUFF[4]=USART2_RX_BUF[3];	// co2 �Ͱ�λ
					 SEND_BUFF[3]=USART2_RX_BUF[2];	// co2 �߰�λ
	         USART1_Send((const char *)SEND_BUFF);
					 delay_ms(40);	
				 }
         USART2_RX_STA=0;		   	
    }
		//����Ũ��ֵ= Ũ��ֵ��λ * 256 + Ũ��ֵ��λ
		//USART2_TX_BUF[3]Ϊ��λ��[2]Ϊ��λ
}
void get_light_data(void)
{
		Single_Write_BH1750(0x01);   // power on
    Single_Write_BH1750(0x10);   // H- resolution mode	
		delay_ms(8);
		mread();      
    dis_data=BUF[0];
    dis_data=(dis_data<<8)+BUF[1];  
		SEND_BUFF[4]=dis_data;	
	  USART1_Send((const char *)SEND_BUFF);
	  delay_ms(40);	
}
void get_soilhumi_data(void)
{
		adcx=Get_Adc_Average(ADC_Channel_6,10);
		temp=(float)adcx*(3.3/4096);
		SEND_BUFF[4]=temp;	
	  USART1_Send((const char *)SEND_BUFF);
	delay_ms(40);	
}
void pwm_on(void)
{
		if(dir)led0pwmval++;
		else led0pwmval--;
 		if(led0pwmval>40)dir=0;
		if(led0pwmval==0)dir=1;										 
		TIM_SetCompare2(TIM3,led0pwmval);
    delay_ms(50);	
}
	
