/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	net_device.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-03-02
	*
	*	版本： 		V1.1
	*
	*	说明： 		网络设备应用层
	*
	*	修改记录：	V1.1：1.平台IP和PORT通过参数传入的方式确定，解决了不同协议网络设备驱动不通用的问题。
	*					  2.取消了手动配置网络，上电等待wifi模块自动连接，若不成功则使用OneNET公众号进行配网。
	*					  3.NET_DEVICE_SendCmd新增参数“mode”，决定是否清除本次命令的返回值。
	************************************************************
	************************************************************
	************************************************************
**/

#include "stm32f10x.h"	//单片机头文件

#include "net_device.h"	//网络设备应用层
#include "net_io.h"		//网络设备数据IO层

//硬件驱动
#include "delay.h"
#include "led.h"
#include "usart.h"
#if(NET_TIME_EN == 1)
#include "clock.h"
#endif

//C库
#include <string.h>
#include <stdlib.h>
#include <stdio.h>




NET_DEVICE_INFO netDeviceInfo = {0, 0, 0, 0, 0, 0}; //





//==========================================================
//	函数名称：	NET_DEVICE_IO_Init
//
//	函数功能：	初始化网络设备IO层
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		初始化网络设备的控制引脚、数据收发功能等
//==========================================================
void NET_DEVICE_IO_Init(void)
{

	GPIO_InitTypeDef gpioInitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	//ESP8266复位引脚
	gpioInitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInitStruct.GPIO_Pin = GPIO_Pin_5;					//GPIOA0-复位
	gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpioInitStruct);
	
	NET_IO_Init();											//网络设备数据IO层初始化
	
	netDeviceInfo.reboot = 0;

}

#if(NET_DEVICE_TRANS == 1)
//==========================================================
//	函数名称：	ESP8266_QuitTrans
//
//	函数功能：	退出透传模式
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		连续发送三个‘+’，然后关闭透传模式
//==========================================================
void ESP8266_QuitTrans(void)
{

	while((NET_IO->SR & 0X40) == 0);	//等待发送空
	NET_IO->DR = '+';
	DelayXms(15); 					//大于串口组帧时间(10ms)
	
	while((NET_IO->SR & 0X40) == 0);	//等待发送空
	NET_IO->DR = '+';        
	DelayXms(15); 					//大于串口组帧时间(10ms)
	
	while((NET_IO->SR & 0X40) == 0);	//等待发送空
	NET_IO->DR = '+';        
	DelayXms(100);					//等待100ms
	
	NET_DEVICE_SendCmd("AT+CIPMODE=0\r\n", "OK", 1); //关闭透传模式

}

//==========================================================
//	函数名称：	ESP8266_EnterTrans
//
//	函数功能：	进入透传模式
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void ESP8266_EnterTrans(void)
{
	
	NET_DEVICE_SendCmd("AT+CIPMUX=0\r\n", "OK", 1);	//单链接模式

	NET_DEVICE_SendCmd("AT+CIPMODE=1\r\n", "OK", 1);//使能透传
	
	NET_DEVICE_SendCmd("AT+CIPSEND\r\n", ">", 1);	//发送数据
	
	DelayXms(100);									//等待100ms

}

#endif

//==========================================================
//	函数名称：	NET_DEVICE_GetTime
//
//	函数功能：	获取网络时间
//
//	入口参数：	无
//
//	返回参数：	UTC秒值
//
//	说明：		
//==========================================================
unsigned int NET_DEVICE_GetTime(void)
{
	
	unsigned int second = 0;
#if(NET_TIME_EN == 1)
	unsigned char *dataPtr;								//数据指针
	struct tm localTime;

	UsartPrintf(USART_DEBUG, "CIPCLOSE\r\n");
	NET_DEVICE_SendCmd("AT+CIPCLOSE\r\n", "OK", 1);
	
	DelayXms(200);										//等待
	
	UsartPrintf(USART_DEBUG, "AT+CIPSTART=\"TCP\",\"24.56.178.140\",13\r\n");
	if(NET_DEVICE_SendCmd("AT+CIPSTART=\"TCP\",\"24.56.178.140\",13\r\n", "CONNECT", 0) == 0)
	{
		dataPtr = NET_DEVICE_GetIPD(200);
		if(dataPtr == NULL)
			return 0;
		
		UsartPrintf(USART_DEBUG, "UTC Time: %s\r\n", dataPtr);
		CLOCK_GetTime(dataPtr, &localTime);				//提取时间数据
		
		second = mktime(&localTime) + 28800;			//小时加8，时区修正
		
		NET_DEVICE_SendCmd("AT+CIPCLOSE\r\n", "OK", 1);
		DelayXms(200);
		
		NET_DEVICE_ClrData();
	}
#endif
	
	return second;

}

//==========================================================
//	函数名称：	NET_DEVICE_Exist
//
//	函数功能：	网络设备存在检查
//
//	入口参数：	无
//
//	返回参数：	返回结果
//
//	说明：		0-成功		1-失败
//==========================================================
_Bool NET_DEVICE_Exist(void)
{

	unsigned char timeOut = 40;
	unsigned char cfgTimeOut = 0;
	_Bool status = 1;
	
	NET_DEVICE_ClrData();
	
	while(timeOut--)												//等待
	{
		DelayXms(100);												//挂起等待
		
		if(NET_IO_WaitRecive() == REV_OK)							//如果收到数据
		{
			if(strstr((char *)netIOInfo.buf, "WIFI GOT IP"))
			{
				status = 0;
			}
			else if(strstr((char *)netIOInfo.buf, "WIFI DISCONNECT"))
			{
#if(PHONE_AP_MODE == 0)
				NET_DEVICE_SendCmd("AT+CWSMARTSTART=2\r\n", "OK", 1);
				UsartPrintf(USART_DEBUG, "请使用OneNET微信公众号配置SSID和PSWD\r\n");
				NET_DEVICE_ClrData();
				
				while(1)
				{
					Led2_Set(LED_ON);Led3_Set(LED_ON);Led4_Set(LED_ON);Led5_Set(LED_ON);
					
					if(strstr((char *)netIOInfo.buf, "SMART SUCCESS"))
					{
						UsartPrintf(USART_DEBUG, "收到:\r\n%s\r\n", strstr((char *)netIOInfo.buf, "SSID:"));
						NET_DEVICE_ClrData();
						status = 1;
						Led2_Set(LED_OFF);Led3_Set(LED_OFF);Led4_Set(LED_OFF);Led5_Set(LED_OFF);
						break;
					}
					else
					{
						if(++cfgTimeOut >= 30)													//超时时间--30s
						{
							cfgTimeOut = 0;
							Led2_Set(LED_OFF);Led3_Set(LED_OFF);Led4_Set(LED_OFF);Led5_Set(LED_OFF);
							break;
						}
					}
					
					DelayXms(500);
					Led2_Set(LED_OFF);Led3_Set(LED_OFF);Led4_Set(LED_OFF);Led5_Set(LED_OFF);
					DelayXms(500);
				}
				
				NET_DEVICE_ClrData();
#else
				UsartPrintf(USART_DEBUG, "STA Tips:	Link Wifi\r\n");
				
				while(NET_DEVICE_SendCmd("AT+CWJAP=\"ONENET\",\"IOT@Chinamobile123\"\r\n", "GOT IP", 1))
				{
					Led7_Set(LED_ON);
					DelayXms(500);
					
					Led7_Set(LED_OFF);
					DelayXms(500);
				}
				
				status = 0;
				
				break;
#endif
			}
			else
				status = 1;
		}
	}
	
	return status;

}

//==========================================================
//	函数名称：	NET_DEVICE_Init
//
//	函数功能：	网络设备初始化
//
//	入口参数：	无
//
//	返回参数：	返回初始化结果
//
//	说明：		0-成功		1-失败
//==========================================================
_Bool NET_DEVICE_Init(char *ip, char *port)
{
	
	unsigned char errCount = 0;
	_Bool status = 1;
	char cfgBuffer[32];
	
	netDeviceInfo.netWork = 0;

	switch(netDeviceInfo.initStep)
	{
		case 0:
			
#if(NET_TIME_EN == 1)
			while(!netDeviceInfo.net_time)
			{
				netDeviceInfo.net_time = NET_DEVICE_GetTime();
			}
#endif
			
			memset(cfgBuffer, 0, sizeof(cfgBuffer));
			
			strcpy(cfgBuffer, "AT+CIPSTART=\"TCP\",\"");
			strcat(cfgBuffer, ip);
			strcat(cfgBuffer, "\",");
			strcat(cfgBuffer, port);
			strcat(cfgBuffer, "\r\n");
			UsartPrintf(USART_DEBUG, "STA Tips:	%s", cfgBuffer);
		
			while(NET_DEVICE_SendCmd(cfgBuffer, "CONNECT", 1))				//连接平台，检索“CONNECT”，如果失败会进入循环体
			{
				Led5_Set(LED_ON);
				DelayXms(500);
				
				Led5_Set(LED_OFF);
				DelayXms(500);
				
				if(++errCount >= 10)
				{
					UsartPrintf(USART_DEBUG, "PT info Error,Use APP -> 8266\r\n");
					status = 1;
					break;
				}
			}
			
			if(errCount != 10)
				netDeviceInfo.initStep++;
		
		break;
			
#if(NET_DEVICE_TRANS == 1)
			
		case 2:

			ESP8266_EnterTrans();											//进入透传模式
			UsartPrintf(USART_DEBUG, "Tips:	EnterTrans\r\n");

			netDeviceInfo.initStep++;

		break;
			
#endif
		
		default:
			
			status = 0;
			netDeviceInfo.netWork = 1;
			UsartPrintf(USART_DEBUG, "Tips:	ESP8266 STA_Mode OK\r\n");
		
		break;
	}
	
	return status;

}

//==========================================================
//	函数名称：	NET_DEVICE_Reset
//
//	函数功能：	网络设备复位
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void NET_DEVICE_Reset(void)
{
	
#if(NET_DEVICE_TRANS == 1)
	ESP8266_QuitTrans();	//退出透传模式
	UsartPrintf(USART_DEBUG, "Tips:	QuitTrans\r\n");
#endif

	UsartPrintf(USART_DEBUG, "Tips:	ESP8266_Reset\r\n");
	
	NET_DEVICE_RST_ON;		//复位
	DelayXms(250);
	
	NET_DEVICE_RST_OFF;		//结束复位
	DelayXms(1000);

}

//==========================================================
//	函数名称：	NET_DEVICE_ReLink
//
//	函数功能：	重连平台
//
//	入口参数：	无
//
//	返回参数：	返回连接结果
//
//	说明：		0-成功		1-失败
//==========================================================
_Bool NET_DEVICE_ReLink(char *ip, char *port)
{
	
	_Bool status = 0;
	char cfgBuffer[32];
	
#if(NET_DEVICE_TRANS == 1)
	ESP8266_QuitTrans();
	UsartPrintf(USART_DEBUG, "Tips:	QuitTrans\r\n");
#endif
	
	NET_DEVICE_SendCmd("AT+CIPCLOSE\r\n", "OK", 1);												//连接前先关闭一次
	UsartPrintf(USART_DEBUG, "Tips:	CIPCLOSE\r\n");
	DelayXms(500);																				//等待

	memset(cfgBuffer, 0, sizeof(cfgBuffer));
			
	strcpy(cfgBuffer, "AT+CIPSTART=\"TCP\",\"");
	strcat(cfgBuffer, ip);
	strcat(cfgBuffer, "\",");
	strcat(cfgBuffer, port);
	strcat(cfgBuffer, "\r\n");
	UsartPrintf(USART_DEBUG, "STA Tips:	%s", cfgBuffer);

	status = NET_DEVICE_SendCmd(cfgBuffer, "CONNECT", 1);										//重新连接
	
#if(NET_DEVICE_TRANS == 1)
		ESP8266_EnterTrans();
		UsartPrintf(USART_DEBUG, "Tips:	EnterTrans\r\n");
#endif
	
	return status;

}

//==========================================================
//	函数名称：	NET_DEVICE_SendCmd
//
//	函数功能：	向网络设备发送一条命令，并等待正确的响应
//
//	入口参数：	cmd：需要发送的命令
//				res：需要检索的响应
//				mode：1-清除接收		0-不清除(能获取返回信息)
//
//	返回参数：	返回连接结果
//
//	说明：		0-成功		1-失败
//==========================================================
_Bool NET_DEVICE_SendCmd(char *cmd, char *res, _Bool mode)
{
	
	unsigned char timeOut = 200;
	
	NET_IO_Send((unsigned char *)cmd, strlen((const char *)cmd));	//写命令到网络设备
	
	netDeviceInfo.dataType = 0;
	
	while(timeOut--)												//等待
	{
		if(NET_IO_WaitRecive() == REV_OK)							//如果收到数据
		{
			if(strstr((const char *)netIOInfo.buf, res) != NULL)	//如果检索到关键词
			{
				if(mode)
					NET_IO_ClearRecive();							//清空缓存
				
				netDeviceInfo.dataType = 1;
				
				return 0;
			}
		}
		
		DelayXms(10);												//挂起等待
	}
	
	netDeviceInfo.dataType = 1;
	
	return 1;

}

//==========================================================
//	函数名称：	NET_DEVICE_SendData
//
//	函数功能：	使网络设备发送数据到平台
//
//	入口参数：	data：需要发送的数据
//				len：数据长度
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void NET_DEVICE_SendData(unsigned char *data, unsigned short len)
{
	
#if(NET_DEVICE_TRANS == 1)
	NET_IO_Send(data, len);  						//发送设备连接请求数据
#else
	char cmdBuf[32];
	
	DelayXms(50);									//等待一下
	
	NET_IO_ClearRecive();							//清空接收缓存
	sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len);		//发送命令
	if(!NET_DEVICE_SendCmd(cmdBuf, ">", 1))			//收到‘>’时可以发送数据
	{
		NET_IO_Send(data, len);  					//发送设备连接请求数据
	}
	
	DelayXms(50);									//等待一下
#endif

}

//==========================================================
//	函数名称：	NET_DEVICE_CheckListHead
//
//	函数功能：	检查发送链表头是否为空
//
//	入口参数：	无
//
//	返回参数：	0-空	1-不为空
//
//	说明：		
//==========================================================
_Bool NET_DEVICE_CheckListHead(void)
{

	if(netIOInfo.head == NULL)
		return 0;
	else
		return 1;

}

//==========================================================
//	函数名称：	NET_DEVICE_GetListHeadBuf
//
//	函数功能：	获取链表里需要发送的数据指针
//
//	入口参数：	无
//
//	返回参数：	获取链表里需要发送的数据指针
//
//	说明：		
//==========================================================
unsigned char *NET_DEVICE_GetListHeadBuf(void)
{

	return netIOInfo.head->buf;

}

//==========================================================
//	函数名称：	NET_DEVICE_GetListHeadLen
//
//	函数功能：	获取链表里需要发送的数据长度
//
//	入口参数：	无
//
//	返回参数：	获取链表里需要发送的数据长度
//
//	说明：		
//==========================================================
unsigned short NET_DEVICE_GetListHeadLen(void)
{

	return netIOInfo.head->dataLen;

}

//==========================================================
//	函数名称：	NET_DEVICE_AddDataSendList
//
//	函数功能：	在发送链表尾新增一个发送链表
//
//	入口参数：	buf：需要发送的数据
//				dataLen：数据长度
//
//	返回参数：	0-成功	其他-失败
//
//	说明：		异步发送方式
//==========================================================
unsigned char NET_DEVICE_AddDataSendList(unsigned char *buf ,unsigned short dataLen)
{
	
	struct NET_SEND_LIST *current = (struct NET_SEND_LIST *)NET_MallocBuffer(sizeof(struct NET_SEND_LIST));
																//分配内存
	
	if(current == NULL)
		return 1;
	
	current->buf = (unsigned char *)NET_MallocBuffer(dataLen);	//分配内存
	if(current->buf == NULL)
	{
		NET_FreeBuffer(current);								//失败则释放
		return 2;
	}
	
	if(netIOInfo.head == NULL)									//如果head为NULL
		netIOInfo.head = current;								//head指向当前分配的内存区
	else														//如果head不为NULL
		netIOInfo.end->next = current;							//则end指向当前分配的内存区
	
	memcpy(current->buf, buf, dataLen);							//复制数据
	current->dataLen = dataLen;
	current->next = NULL;										//下一段为NULL
	
	netIOInfo.end = current;									//end指向当前分配的内存区
	
	return 0;

}

//==========================================================
//	函数名称：	NET_DEVICE_DeleteDataSendList
//
//	函数功能：	从链表头删除一个链表
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
_Bool NET_DEVICE_DeleteDataSendList(void)
{
	
	struct NET_SEND_LIST *next = netIOInfo.head->next;	//保存链表头的下一段数据地址
	
	netIOInfo.head->dataLen = 0;
	netIOInfo.head->next = NULL;
	NET_FreeBuffer(netIOInfo.head->buf);				//释放内存
	NET_FreeBuffer(netIOInfo.head);						//释放内存
	
	netIOInfo.head = next;								//链表头指向下一段数据
	
	return 0;

}

//==========================================================
//	函数名称：	NET_DEVICE_GetIPD
//
//	函数功能：	获取平台返回的数据
//
//	入口参数：	等待的时间(乘以10ms)
//
//	返回参数：	平台返回的原始数据
//
//	说明：		不同网络设备返回的格式不同，需要去调试
//				如ESP8266的返回格式为	"+IPD,x:yyy"	x代表数据长度，yyy是数据内容
//==========================================================
unsigned char *NET_DEVICE_GetIPD(unsigned short timeOut)
{

#if(NET_DEVICE_TRANS == 0)
	char *ptrIPD = NULL;
#endif
	
	do
	{
		if(NET_IO_WaitRecive() == REV_OK)								//如果接收完成
		{
#if(NET_DEVICE_TRANS == 0)
			ptrIPD = strstr((char *)netIOInfo.buf, "IPD,");				//搜索“IPD”头
			if(ptrIPD == NULL)											//如果没找到，可能是IPD头的延迟，还是需要等待一会，但不会超过设定的时间
			{
				//UsartPrintf(USART_DEBUG, "\"IPD\" not found\r\n");
			}
			else
			{
				ptrIPD = strchr(ptrIPD, ':');							//找到':'
				if(ptrIPD != NULL)
				{
					ptrIPD++;
					return (unsigned char *)(ptrIPD);
				}
				else
					return NULL;
				
			}
#else
			return netIOInfo.buf;
#endif
		}
		
		DelayXms(5);													//延时等待
	} while(timeOut--);
	
	return NULL;														//超时还未找到，返回空指针

}

//==========================================================
//	函数名称：	NET_DEVICE_ClrData
//
//	函数功能：	清空网络设备数据接收缓存
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void NET_DEVICE_ClrData(void)
{

	NET_IO_ClearRecive();	//清空缓存

}

//==========================================================
//	函数名称：	NET_DEVICE_Check
//
//	函数功能：	检查网络设备连接状态
//
//	入口参数：	无
//
//	返回参数：	返回状态
//
//	说明：		
//==========================================================
unsigned char NET_DEVICE_Check(void)
{
	
	unsigned char status = 0;
	unsigned char timeOut = 200;
	
#if(NET_DEVICE_TRANS == 1)
	ESP8266_QuitTrans();
	UsartPrintf(USART_DEBUG, "Tips:	QuitTrans\r\n");
#endif

	NET_IO_ClearRecive();												//清空缓存
	NET_IO_Send((unsigned char *)"AT+CIPSTATUS\r\n",  14);				//发送状态监测
	
	while(--timeOut)
	{
		if(NET_IO_WaitRecive() == REV_OK)
		{
			if(strstr((const char *)netIOInfo.buf, "STATUS:2"))			//获得IP
			{
				status = 2;
				UsartPrintf(USART_DEBUG, "ESP8266 Got IP\r\n");
			}
			else if(strstr((const char *)netIOInfo.buf, "STATUS:3"))	//建立连接
			{
				status = 0;
				UsartPrintf(USART_DEBUG, "ESP8266 Connect OK\r\n");
			}
			else if(strstr((const char *)netIOInfo.buf, "STATUS:4"))	//失去连接
			{
				status = 1;
				UsartPrintf(USART_DEBUG, "ESP8266 Lost Connect\r\n");
			}
			else if(strstr((const char *)netIOInfo.buf, "STATUS:5"))	//物理掉线
			{
				status = 3;
				UsartPrintf(USART_DEBUG, "ESP8266 Lost\r\n");			//设备丢失
			}
			
			break;
		}
		
		DelayXms(10);
	}
	
	if(timeOut == 0)
	{
		status = 3;
		UsartPrintf(USART_DEBUG, "ESP8266 TimeOut\r\n");
	}
	
	return status;

}

//==========================================================
//	函数名称：	NET_DEVICE_ReConfig
//
//	函数功能：	设备网络设备初始化的步骤
//
//	入口参数：	步骤值
//
//	返回参数：	无
//
//	说明：		该函数设置的参数在网络设备初始化里边用到
//==========================================================
void NET_DEVICE_ReConfig(unsigned char step)
{

	netDeviceInfo.initStep = step;

}
