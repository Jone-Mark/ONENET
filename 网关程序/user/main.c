//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"
#include "sys.h"
//����Э���
#include "onenet.h"
#include "fault.h"

//�����豸
#include "net_device.h"

//Ӳ������
#include "sys.h"
#include "lcd.h"
#include "gpio.h"
#include "delay.h"
#include "usart.h"
#include "hwtimer.h"
#include "selfcheck.h"
#include "info.h"
#include "rtc.h"
//����������
#include "dataStreamName.h"

//C��
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
//ͼƬ�����ļ�
#include "pic.h"
char myTime[24];

//������
typedef struct//���������ݽṹ��
{
	float temp;  //�¶�
	float humi;  //ʪ��
	float g_humi;//����ʪ��
	float co2;   //������̼
 	int lux;     //����ǿ��
	float atmos; //����ѹ
} SENSOR_DATA;
SENSOR_DATA sensordata;

DATA_STREAM dataStream[] = {
									{FARM_TEMP, &sensordata.temp, TYPE_FLOAT, 1},         //�¶������ϴ�
									{FARM_HUMI, &sensordata.humi, TYPE_FLOAT, 1},         //ʪ�������ϴ�
									{FARM_G_HUMI, &sensordata.g_humi, TYPE_FLOAT, 1},     //����ʪ�������ϴ�
									{FARM_CO2, &sensordata.co2, TYPE_FLOAT, 1},           //������̼�����ϴ�
									{FARM_LUX, &sensordata.lux, TYPE_INT, 1},             //�����������ϴ�
									{FARM_ATMOS, &sensordata.atmos, TYPE_FLOAT, 1},       //����ѹ�����ϴ�
									{FARM_TIME, myTime, TYPE_STRING, 1},
  								{FARM_ERRTYPE, &faultTypeReport, TYPE_UCHAR, 1},
							};
unsigned char dataStreamCnt = sizeof(dataStream) / sizeof(dataStream[0]);

/*************************************************************
*	�������ƣ�	LCD_Init SHOW
* �������ܣ�	LCD��ʼ����ʾ
************************************************************/							
void LCD_Init_Show(void)
{
	POINT_COLOR=BLUE;//��������Ϊ��ɫ
	LCD_ShowString(60,50 ,400,48,48 ,(u8 *)"   Smart Farm       ");
	LCD_ShowString(60,100,380,24,24 ,(u8 *)"-------------------------------");
	LCD_ShowString(60,125,380,24,24,(u8 *)"| BUU Demonstration Center of |");
  LCD_ShowString(60,150,380,24,24,(u8 *)"| Experimental Teaching in CE |");	
  LCD_ShowString(60,175,380,24,24,(u8 *)"| and Smart City College      |");	
	LCD_ShowString(60,200,380,24,24,(u8 *)"-------------------------------");
//LCD_ShowString(60,175,380,24,24,"Hardware Init OK");
//LCD_ShowString(60,200,380,24,24,"NET Device:OK");
	LCD_ShowString(60,275,380,24,24,(u8 *)"DATA:");
	LCD_ShowString(60,300,380,24,24,(u8 *)"-------------------------------");
	LCD_ShowString(60,325,380,24,24,(u8 *)"|TEMP :                       |");	
	LCD_ShowString(60,350,380,24,24,(u8 *)"|HUMI :                       |");
  LCD_ShowString(60,375,380,24,24,(u8 *)"|LIGHT:                       |");		
	LCD_ShowString(60,400,380,24,24,(u8 *)"|CO2  :                       |");	
	LCD_ShowString(60,425,380,24,24,(u8 *)"|SOIL :                       |");		
  LCD_ShowString(60,450,380,24,24,(u8 *)"-------------------------------");
	LCD_ShowString(60,475,380,24,24,(u8 *)"CTRL:");
	LCD_ShowString(60,500,380,24,24,(u8 *)"-------------------------------");
	LCD_ShowString(60,525,380,24,24,(u8 *)"|Fan   state:                 |");	
	LCD_ShowString(60,550,380,24,24,(u8 *)"|Light state:                 |");
  LCD_ShowString(60,575,380,24,24,(u8 *)"|Water2 state:                |");		
	LCD_ShowString(60,600,380,24,24,(u8 *)"|Water2 state:                |");		
  LCD_ShowString(60,625,380,24,24,(u8 *)"-------------------------------");

	//LCD_ShowString(60,600,380,96,96,(u8 *)" ");
	Picture_Draw(190,650,gImage_pic);
	POINT_COLOR=RED;//��������Ϊ��ɫ 
}	

/*************************************************************
*	�������ƣ�	Hardware_Init
* �������ܣ�	Ӳ����ʼ��
*	˵����		��ʼ����Ƭ�������Լ�����豸
************************************************************/
void Hardware_Init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//�жϿ�������������

	Delay_Init();																    //systick��ʼ��
	Led_Init();		    															//LED��ʼ��
	Usart1_Init(115200); 														//��ʼ������   115200bps
#if(USART_DMA_RX_EN)
	USARTx_ResetMemoryBaseAddr(USART_DEBUG, (unsigned int)alterInfo.alterBuf, sizeof(alterInfo.alterBuf), USART_RX_TYPE);
#endif
	
	RTC_Init();					    												//��ʼ��RTC
	LCD_Init();	
	LCD_Init_Show();
	UsartPrintf(USART_DEBUG, "DEVID: %s,  APIKEY: %s\r\n", oneNetInfo.devID, oneNetInfo.apiKey);
	netDeviceInfo.reboot = 0;
	
	Timer3_4_Init(TIM3, 49, 35999);												//72MHz��36000��Ƶ-500us��50����ֵ�����ж�����Ϊ500us * 50 = 25ms
	Timer3_4_Init(TIM4, 1999, 35999);											//72MHz��36000��Ƶ-500us��2000����ֵ�����ж�����Ϊ500us * 2000 = 1s
	
	UsartPrintf(USART_DEBUG, "3.Hardware init OK\r\n");							//��ʾ��ʼ�����
  
	LCD_ShowString(60,225,380,24,24,(u8 *)"Hardware Init OK");
}

/************************************************************
*	�������ƣ�	main
*	�������ܣ�	
*	��ڲ�����	��
*	���ز�����	0
*	˵����		
************************************************************/
int main(void)
{				
	unsigned char *dataPtr,i;
 	unsigned int  send_time = 0, sensor_time = 0, heart_time = 0, list_time = 0;
	unsigned char sendFlag = 0;
	
#if(NET_TIME_EN == 1)
	unsigned int second = 0, secondCmp = 0;	//second��ʵʱʱ�䣬secondCmp�ǱȽ�У׼ʱ��
	struct tm *time;
#endif

	Hardware_Init();									  //Ӳ����ʼ��
	
	NET_DEVICE_IO_Init();								//�����豸IO��ʼ��
	NET_DEVICE_Reset();									//����ɡ�豸��λ
	while(1)
	{
		//С����ʾ��ȡȡ����ȡ���²���;alterInfo.alterBuf[0]��������uart1������
		//EE CC 00 00 00 79 6F 00 00 00 00 01 0B 0A 11 22 33 44 55 66 77 88 00 00 00 00
		//
		
		
		for(i=0;i<28;i++)
		{
			if((alterInfo.alterBuf[i] == 0x0B) && (alterInfo.alterBuf[i+1] == 0x0A))//�յ�0b 0a˵���յ�һ֡
			{
				if(alterInfo.alterBuf[i+2] == 0x01 && alterInfo.alterBuf[i+5] == 0x01)//�����յ�DHT11������
				{
					LCD_ShowxNum(156,325,alterInfo.alterBuf[i+3],2,24,0X80);//��ʾ�¶�
					LCD_ShowxNum(156,350,alterInfo.alterBuf[i+4],2,24,0X80);//��ʾʪ��
					sensordata.humi = alterInfo.alterBuf[i+3];
		      sensordata.temp = alterInfo.alterBuf[i+4];
					memset(alterInfo.alterBuf,NULL,sizeof(alterInfo.alterBuf));
				}
				
				if(alterInfo.alterBuf[i+2] == 0x02 && alterInfo.alterBuf[i+5] == 0x02)//�����յ����մ�����
				{
					LCD_ShowxNum(156,375,alterInfo.alterBuf[i+4],3,24,0X80);//��ʾ����
					sensordata.lux = alterInfo.alterBuf[i+4];
					memset(alterInfo.alterBuf,NULL,sizeof(alterInfo.alterBuf));
				}
				
				if(alterInfo.alterBuf[i+2] == 0x03 && alterInfo.alterBuf[i+5] == 0x03)//�����յ�CO2������
				{
					LCD_ShowxNum(156,400,alterInfo.alterBuf[i+3]*256+alterInfo.alterBuf[i+4],3,24,0X80);//��ʾCO2 �������߰�λ�͵Ͱ�λ����
					sensordata.co2 = alterInfo.alterBuf[i+3]*256+alterInfo.alterBuf[i+4];
					memset(alterInfo.alterBuf,NULL,sizeof(alterInfo.alterBuf));
				}
				
				if(alterInfo.alterBuf[i+2] == 0x04 && alterInfo.alterBuf[i+5] == 0x04)//�����յ�����ʪ�ȴ�����
				{
					LCD_ShowxNum(156,425,alterInfo.alterBuf[i+4],2,24,0X80);//��ʾ����ʪ��
					sensordata.atmos = alterInfo.alterBuf[i+4];
					memset(alterInfo.alterBuf,NULL,sizeof(alterInfo.alterBuf));
				}
			}
		}
		
		if(oneNetInfo.netWork == 1)
		{
/******************************************************************************
			����������
******************************************************************************/
			if(timInfo.timer3Out - send_time >= 3000)								//25sһ��(25ms�ж�)
			{
				send_time = timInfo.timer3Out;
				
				if(sendFlag)
				{
					OneNet_SendData_Heart();										        //��������
				}
				else
				{
					oneNetInfo.sendData = OneNet_SendData(FORMAT_TYPE3, NULL, NULL, dataStream, dataStreamCnt);//�ϴ����ݵ�ƽ̨
				}
		
				sendFlag = !sendFlag;
			}
			
			if(timInfo.timer3Out - heart_time >= 2)									//50msһ��(25ms�ж�)
			{
				heart_time = timInfo.timer3Out;
				OneNet_Check_Heart();
			}
			
/******************************************************************************
			ƽ̨�·��������
******************************************************************************/
			if(oneNetInfo.netWork && netDeviceInfo.dataType)						//�������� �� ���������ģʽʱ
			{
				dataPtr = NET_DEVICE_GetIPD(0);										//���ȴ�����ȡƽ̨�·�������
				if(dataPtr != NULL)													//�������ָ�벻Ϊ�գ�������յ�������
				{
					OneNet_RevPro(dataPtr);											//���д���
				}
			}
			
/******************************************************************************
			������
******************************************************************************/
			if(timInfo.timer3Out - sensor_time >= 400)								//ԭ��80   80*0.025  2sһ��(25ms�ж�) 4000*0.025=100
			{		
				sensor_time = timInfo.timer3Out;
				sensordata.humi=33.2;
				sensordata.co2++;
				sensordata.temp=45.2;
				sensordata.g_humi=63;
				sensordata.lux=18;
				sensordata.atmos=78;
			}
/******************************************************************************
			ͼ����
******************************************************************************/

/******************************************************************************
			������
******************************************************************************/
			if(faultType != FAULT_NONE)												//��������־������
			{
				UsartPrintf(USART_DEBUG, "WARN:	Fault Process\r\n");
				Fault_Process();													//�����������
			}
			
/******************************************************************************
			���ݷ���
******************************************************************************/
			if(oneNetInfo.sendData)
			{
				oneNetInfo.sendData = OneNet_SendData(FORMAT_TYPE3, NULL, NULL, dataStream, dataStreamCnt);//�ϴ����ݵ�ƽ̨
			}
			
/******************************************************************************
			���ݷ�������
******************************************************************************/
			if(timInfo.timer3Out - list_time >= 20)								//500msһ��(25ms�ж�)
			{
				list_time = timInfo.timer3Out;
				if(NET_DEVICE_CheckListHead())
				{
					NET_DEVICE_SendData(NET_DEVICE_GetListHeadBuf(), NET_DEVICE_GetListHeadLen());
					NET_DEVICE_DeleteDataSendList();
				}
			}
			
/******************************************************************************
			��ʾʱ��
******************************************************************************/
#if(NET_TIME_EN == 1)
			if(second == 0)														//��secondΪ0ʱ
			{
				dataStream[6].flag = 0;											//���ϴ�ʱ��

				if(netDeviceInfo.net_time)
				{
					second = netDeviceInfo.net_time;
					
					RTC_SetTime(second + 4);									//����RTCʱ�䣬��4�ǲ��ϴ�ŵ�ʱ���
					
					dataStream[6].flag = 1;										//�ϴ�ʱ��
				}
			}
			else																//��������
			{
				secondCmp = second;
				second = RTC_GetCounter();										//��ȡ��ֵ
				
				if(second > secondCmp)
				{
					time = localtime((const time_t *)&second);					//����ֵתΪtm�ṹ����ʾ��ʱ��
					
					memset(myTime, 0, sizeof(myTime));
					snprintf(myTime, sizeof(myTime), "%d-%d-%d %d:%d:%d",
									time->tm_year + 1900, time->tm_mon + 1, time->tm_mday,
									time->tm_hour, time->tm_min, time->tm_sec);
				
					if(time->tm_hour == 0 && time->tm_min == 0 && time->tm_sec == 0)//ÿ��0��ʱ������һ��ʱ��
					{
						second = 0;
						netDeviceInfo.net_time = 0;
						oneNetInfo.netWork = 0;
						NET_DEVICE_ReConfig(0);
					}
				}
			}
#endif
		}
		else
		{
/******************************************************************************
			��ʼ�������豸������ƽ̨
******************************************************************************/
			if(!oneNetInfo.netWork && (checkInfo.NET_DEVICE_OK == DEV_OK))			//��û������ �� ����ģ���⵽ʱ
			{
				if(!NET_DEVICE_Init(oneNetInfo.ip, oneNetInfo.port))				//��ʼ�������豸������������
				{
					OneNet_DevLink(oneNetInfo.devID, oneNetInfo.apiKey);			//����ƽ̨
					
					if(oneNetInfo.netWork)
					{
						BEEP_ON;											//�̽���ʾ�ɹ�
						DelayXms(200);
						BEEP_OFF;
						send_time = timInfo.timer3Out;								//����ʱ��
						sensor_time = timInfo.timer3Out;							//����ʱ��
						heart_time = timInfo.timer3Out;								//����ʱ��
						list_time = timInfo.timer3Out;								//����ʱ��
					}
				}
			}
			
/******************************************************************************
			�����豸���
******************************************************************************/
			if(checkInfo.NET_DEVICE_OK == DEV_ERR) 									//�������豸δ�����
			{
				if(timerCount >= NET_TIME) 											//����������ӳ�ʱ
				{
					UsartPrintf(USART_DEBUG, "Tips:		Timer Check Err\r\n");
					
					NET_DEVICE_Reset();												//��λ�����豸
					timerCount = 0;													//�������ӳ�ʱ����
					faultType = FAULT_NONE;											//��������־
					
				}
				
				if(!NET_DEVICE_Exist())												//�����豸���
				{
					UsartPrintf(USART_DEBUG, "NET Device :Ok\r\n");
					checkInfo.NET_DEVICE_OK = DEV_OK;								//��⵽�����豸�����
          POINT_COLOR=RED;//��������Ϊ��ɫ 
		      LCD_ShowString(60,250,380,24,24,(u8 *)"NET Device:OK");
				}
				else
				{
					UsartPrintf(USART_DEBUG, "NET Device :Error\r\n");
					POINT_COLOR=RED;//��������Ϊ��ɫ 
					LCD_ShowString(60,250,380,24,24,(u8 *)"NET Device:NO"); 		
				}
			}
		}
	}

}
