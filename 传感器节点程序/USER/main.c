#include "main.h"

//修改此处数值给不同节点下载节能 //0-DHT11  1-光照传感器   2-CO2    3-土壤湿度  
#define SENSOR_TYPE 2

/*Zigbee传输协议 
 0,1  包头                0xEE 0xCC
 2    节点类型            0-控制节点  1-传感器节点  
 3    传感器类型          0-DHT11     1-光照传感器    2-二氧化碳    3-土壤湿度  
 4    数据高八位
 5    数据低八位
 6,7  包尾                0x0D 0x0A
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
	
  #if(SENSOR_TYPE==1)	 //空气温湿度
	DHT11_Init();
	SEND_BUFF[2]=0x01;
	SEND_BUFF[5]=0x01;
	#elif(SENSOR_TYPE==2)	 //光照强度
	GPIOConfig();
	Init_BH1750(); 
	SEND_BUFF[2]=0x02;
	SEND_BUFF[5]=0x02;
	#elif(SENSOR_TYPE==3)	     //CO2采集
	USART2_Init(9600);
  SEND_BUFF[2]=0x03;
  SEND_BUFF[5]=0x03;	
  #elif(SENSOR_TYPE==4)  //采集土壤湿度
	Adc_Init();
  SEND_BUFF[2]=0x04;	
	SEND_BUFF[5]=0x04;
  #endif	
 	
	while(1)
	{
    pwm_on();
  #if(SENSOR_TYPE==1)	 //空气温湿度
		get_temp_humi_data();
	#elif(SENSOR_TYPE==2)	 //光照强度
		get_light_data();
	#elif(SENSOR_TYPE==3)	     //CO2采集
		get_co2_data();
	#elif(SENSOR_TYPE==4)  //采集土壤湿度
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
		for (i=0;i<9;i++)//发送CMD指令
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
					 SEND_BUFF[4]=USART2_RX_BUF[3];	// co2 低八位
					 SEND_BUFF[3]=USART2_RX_BUF[2];	// co2 高八位
	         USART1_Send((const char *)SEND_BUFF);
					 delay_ms(40);	
				 }
         USART2_RX_STA=0;		   	
    }
		//气体浓度值= 浓度值高位 * 256 + 浓度值低位
		//USART2_TX_BUF[3]为低位，[2]为高位
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
	
