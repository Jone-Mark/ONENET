//单片机头文件
#include "stm32f10x.h"
#include "sys.h"
//网络协议层
#include "onenet.h"
#include "fault.h"

//网络设备
#include "net_device.h"

//硬件驱动
#include "sys.h"
#include "lcd.h"
#include "gpio.h"
#include "delay.h"
#include "usart.h"
#include "hwtimer.h"
#include "selfcheck.h"
#include "info.h"
#include "rtc.h"
//中文数据流
#include "dataStreamName.h"

//C库
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
//图片数据文件
#include "pic.h"
char myTime[24];

//数据流
typedef struct//传感器数据结构体
{
	float temp;  //温度
	float humi;  //湿度
	float g_humi;//土壤湿度
	float co2;   //二氧化碳
 	int lux;     //光照强度
	float atmos; //大气压
} SENSOR_DATA;
SENSOR_DATA sensordata;

DATA_STREAM dataStream[] = {
									{FARM_TEMP, &sensordata.temp, TYPE_FLOAT, 1},         //温度数据上传
									{FARM_HUMI, &sensordata.humi, TYPE_FLOAT, 1},         //湿度数据上传
									{FARM_G_HUMI, &sensordata.g_humi, TYPE_FLOAT, 1},     //土壤湿度数据上传
									{FARM_CO2, &sensordata.co2, TYPE_FLOAT, 1},           //二氧化碳数据上传
									{FARM_LUX, &sensordata.lux, TYPE_INT, 1},             //光照照数据上穿
									{FARM_ATMOS, &sensordata.atmos, TYPE_FLOAT, 1},       //大气压数据上传
									{FARM_TIME, myTime, TYPE_STRING, 1},
  								{FARM_ERRTYPE, &faultTypeReport, TYPE_UCHAR, 1},
							};
unsigned char dataStreamCnt = sizeof(dataStream) / sizeof(dataStream[0]);

/*************************************************************
*	函数名称：	LCD_Init SHOW
* 函数功能：	LCD初始化显示
************************************************************/							
void LCD_Init_Show(void)
{
	POINT_COLOR=BLUE;//设置字体为蓝色
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
	POINT_COLOR=RED;//设置字体为红色 
}	

/*************************************************************
*	函数名称：	Hardware_Init
* 函数功能：	硬件初始化
*	说明：		初始化单片机功能以及外接设备
************************************************************/
void Hardware_Init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//中断控制器分组设置

	Delay_Init();																    //systick初始化
	Led_Init();		    															//LED初始化
	Usart1_Init(115200); 														//初始化串口   115200bps
#if(USART_DMA_RX_EN)
	USARTx_ResetMemoryBaseAddr(USART_DEBUG, (unsigned int)alterInfo.alterBuf, sizeof(alterInfo.alterBuf), USART_RX_TYPE);
#endif
	
	RTC_Init();					    												//初始化RTC
	LCD_Init();	
	LCD_Init_Show();
	UsartPrintf(USART_DEBUG, "DEVID: %s,  APIKEY: %s\r\n", oneNetInfo.devID, oneNetInfo.apiKey);
	netDeviceInfo.reboot = 0;
	
	Timer3_4_Init(TIM3, 49, 35999);												//72MHz，36000分频-500us，50重载值。则中断周期为500us * 50 = 25ms
	Timer3_4_Init(TIM4, 1999, 35999);											//72MHz，36000分频-500us，2000重载值。则中断周期为500us * 2000 = 1s
	
	UsartPrintf(USART_DEBUG, "3.Hardware init OK\r\n");							//提示初始化完成
  
	LCD_ShowString(60,225,380,24,24,(u8 *)"Hardware Init OK");
}

/************************************************************
*	函数名称：	main
*	函数功能：	
*	入口参数：	无
*	返回参数：	0
*	说明：		
************************************************************/
int main(void)
{				
	unsigned char *dataPtr,i;
 	unsigned int  send_time = 0, sensor_time = 0, heart_time = 0, list_time = 0;
	unsigned char sendFlag = 0;
	
#if(NET_TIME_EN == 1)
	unsigned int second = 0, secondCmp = 0;	//second是实时时间，secondCmp是比较校准时间
	struct tm *time;
#endif

	Hardware_Init();									  //硬件初始化
	
	NET_DEVICE_IO_Init();								//网络设备IO初始化
	NET_DEVICE_Reset();									//网络伞よ备复位
	while(1)
	{
		//小数显示采取取整和取余下部分;alterInfo.alterBuf[0]接收来自uart1的数据
		//EE CC 00 00 00 79 6F 00 00 00 00 01 0B 0A 11 22 33 44 55 66 77 88 00 00 00 00
		//
		
		
		for(i=0;i<28;i++)
		{
			if((alterInfo.alterBuf[i] == 0x0B) && (alterInfo.alterBuf[i+1] == 0x0A))//收到0b 0a说明收到一帧
			{
				if(alterInfo.alterBuf[i+2] == 0x01 && alterInfo.alterBuf[i+5] == 0x01)//假如收到DHT11传感器
				{
					LCD_ShowxNum(156,325,alterInfo.alterBuf[i+3],2,24,0X80);//显示温度
					LCD_ShowxNum(156,350,alterInfo.alterBuf[i+4],2,24,0X80);//显示湿度
					sensordata.humi = alterInfo.alterBuf[i+3];
		      sensordata.temp = alterInfo.alterBuf[i+4];
					memset(alterInfo.alterBuf,NULL,sizeof(alterInfo.alterBuf));
				}
				
				if(alterInfo.alterBuf[i+2] == 0x02 && alterInfo.alterBuf[i+5] == 0x02)//假如收到光照传感器
				{
					LCD_ShowxNum(156,375,alterInfo.alterBuf[i+4],3,24,0X80);//显示光照
					sensordata.lux = alterInfo.alterBuf[i+4];
					memset(alterInfo.alterBuf,NULL,sizeof(alterInfo.alterBuf));
				}
				
				if(alterInfo.alterBuf[i+2] == 0x03 && alterInfo.alterBuf[i+5] == 0x03)//假如收到CO2传感器
				{
					LCD_ShowxNum(156,400,alterInfo.alterBuf[i+3]*256+alterInfo.alterBuf[i+4],3,24,0X80);//显示CO2 传感器高八位和低八位叠加
					sensordata.co2 = alterInfo.alterBuf[i+3]*256+alterInfo.alterBuf[i+4];
					memset(alterInfo.alterBuf,NULL,sizeof(alterInfo.alterBuf));
				}
				
				if(alterInfo.alterBuf[i+2] == 0x04 && alterInfo.alterBuf[i+5] == 0x04)//假如收到土壤湿度传感器
				{
					LCD_ShowxNum(156,425,alterInfo.alterBuf[i+4],2,24,0X80);//显示土壤湿度
					sensordata.atmos = alterInfo.alterBuf[i+4];
					memset(alterInfo.alterBuf,NULL,sizeof(alterInfo.alterBuf));
				}
			}
		}
		
		if(oneNetInfo.netWork == 1)
		{
/******************************************************************************
			数据与心跳
******************************************************************************/
			if(timInfo.timer3Out - send_time >= 3000)								//25s一次(25ms中断)
			{
				send_time = timInfo.timer3Out;
				
				if(sendFlag)
				{
					OneNet_SendData_Heart();										        //心跳连接
				}
				else
				{
					oneNetInfo.sendData = OneNet_SendData(FORMAT_TYPE3, NULL, NULL, dataStream, dataStreamCnt);//上传数据到平台
				}
		
				sendFlag = !sendFlag;
			}
			
			if(timInfo.timer3Out - heart_time >= 2)									//50ms一次(25ms中断)
			{
				heart_time = timInfo.timer3Out;
				OneNet_Check_Heart();
			}
			
/******************************************************************************
			平台下发命令解析
******************************************************************************/
			if(oneNetInfo.netWork && netDeviceInfo.dataType)						//当有网络 且 在命令接收模式时
			{
				dataPtr = NET_DEVICE_GetIPD(0);										//不等待，获取平台下发的数据
				if(dataPtr != NULL)													//如果数据指针不为空，则代表收到了数据
				{
					OneNet_RevPro(dataPtr);											//集中处理
				}
			}
			
/******************************************************************************
			传感器
******************************************************************************/
			if(timInfo.timer3Out - sensor_time >= 400)								//原来80   80*0.025  2s一次(25ms中断) 4000*0.025=100
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
			图像传输
******************************************************************************/

/******************************************************************************
			错误处理
******************************************************************************/
			if(faultType != FAULT_NONE)												//如果错误标志被设置
			{
				UsartPrintf(USART_DEBUG, "WARN:	Fault Process\r\n");
				Fault_Process();													//进入错误处理函数
			}
			
/******************************************************************************
			数据反馈
******************************************************************************/
			if(oneNetInfo.sendData)
			{
				oneNetInfo.sendData = OneNet_SendData(FORMAT_TYPE3, NULL, NULL, dataStream, dataStreamCnt);//上传数据到平台
			}
			
/******************************************************************************
			数据发送链表
******************************************************************************/
			if(timInfo.timer3Out - list_time >= 20)								//500ms一次(25ms中断)
			{
				list_time = timInfo.timer3Out;
				if(NET_DEVICE_CheckListHead())
				{
					NET_DEVICE_SendData(NET_DEVICE_GetListHeadBuf(), NET_DEVICE_GetListHeadLen());
					NET_DEVICE_DeleteDataSendList();
				}
			}
			
/******************************************************************************
			显示时间
******************************************************************************/
#if(NET_TIME_EN == 1)
			if(second == 0)														//当second为0时
			{
				dataStream[6].flag = 0;											//不上传时间

				if(netDeviceInfo.net_time)
				{
					second = netDeviceInfo.net_time;
					
					RTC_SetTime(second + 4);									//设置RTC时间，加4是补上大概的时间差
					
					dataStream[6].flag = 1;										//上传时间
				}
			}
			else																//正常运行
			{
				secondCmp = second;
				second = RTC_GetCounter();										//获取秒值
				
				if(second > secondCmp)
				{
					time = localtime((const time_t *)&second);					//将秒值转为tm结构所表示的时间
					
					memset(myTime, 0, sizeof(myTime));
					snprintf(myTime, sizeof(myTime), "%d-%d-%d %d:%d:%d",
									time->tm_year + 1900, time->tm_mon + 1, time->tm_mday,
									time->tm_hour, time->tm_min, time->tm_sec);
				
					if(time->tm_hour == 0 && time->tm_min == 0 && time->tm_sec == 0)//每天0点时，更新一次时间
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
			初始化网络设备、接入平台
******************************************************************************/
			if(!oneNetInfo.netWork && (checkInfo.NET_DEVICE_OK == DEV_OK))			//当没有网络 且 网络模块检测到时
			{
				if(!NET_DEVICE_Init(oneNetInfo.ip, oneNetInfo.port))				//初始化网络设备，能连入网络
				{
					OneNet_DevLink(oneNetInfo.devID, oneNetInfo.apiKey);			//接入平台
					
					if(oneNetInfo.netWork)
					{
						BEEP_ON;											//短叫提示成功
						DelayXms(200);
						BEEP_OFF;
						send_time = timInfo.timer3Out;								//更新时间
						sensor_time = timInfo.timer3Out;							//更新时间
						heart_time = timInfo.timer3Out;								//更新时间
						list_time = timInfo.timer3Out;								//更新时间
					}
				}
			}
			
/******************************************************************************
			网络设备检测
******************************************************************************/
			if(checkInfo.NET_DEVICE_OK == DEV_ERR) 									//当网络设备未做检测
			{
				if(timerCount >= NET_TIME) 											//如果网络连接超时
				{
					UsartPrintf(USART_DEBUG, "Tips:		Timer Check Err\r\n");
					
					NET_DEVICE_Reset();												//复位网络设备
					timerCount = 0;													//清零连接超时计数
					faultType = FAULT_NONE;											//清除错误标志
					
				}
				
				if(!NET_DEVICE_Exist())												//网络设备检测
				{
					UsartPrintf(USART_DEBUG, "NET Device :Ok\r\n");
					checkInfo.NET_DEVICE_OK = DEV_OK;								//检测到网络设备，标记
          POINT_COLOR=RED;//设置字体为红色 
		      LCD_ShowString(60,250,380,24,24,(u8 *)"NET Device:OK");
				}
				else
				{
					UsartPrintf(USART_DEBUG, "NET Device :Error\r\n");
					POINT_COLOR=RED;//设置字体为红色 
					LCD_ShowString(60,250,380,24,24,(u8 *)"NET Device:NO"); 		
				}
			}
		}
	}

}
