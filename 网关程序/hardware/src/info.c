/**************************************************************
*	文件名： 	info.c
*	作者： 		张继瑞
*	日期： 		2017-02-23
*	版本： 		V1.1
*	说明： 		V1.0：SSID、PSWD、DEVID、APIKEY保存及读取。
*				    V1.1：取消了SSID和PSWD的保存和读写，替换为了智能配网，wifi类型的网络设备可以自动保存。

*	重要：   只有当外部存储器存在时，才从中读取信息
*					 若不存在，会读取固化在代码里的信息
*	修改记录：	
************************************************************/

//硬件驱动
#include "info.h"
#include "delay.h"
#include "usart.h"

//协议
#include "onenet.h"

//C库
#include <string.h>
#include <stdlib.h>

/*
************************************************************
*	函数名称：	Info_Check
*	函数功能：	检查信息是否存在
*	入口参数：	无
*	返回参数：	检查结果
*	说明：		  判断wifi的ssid和pswd是否存在
*				0-ok	1-无ssid	2-无pswd
*				3-无devid	4-无apikey
************************************************************
*/
unsigned char Info_Check(void)
{
//	unsigned char rData = 0;
//	
//	AT24C02_ReadByte(DEVID_ADDRESS, &rData);	//读取长度值
//	if(rData == 0 || rData >= 10)				//如果为0或超出
//		return 1;
//	
//	AT24C02_ReadByte(AKEY_ADDRESS, &rData);		//读取长度值
//	if(rData == 0 || rData >= 30)				//如果为0或超出
//		return 2;
//        
	return 0;

}

/*
************************************************************
*	函数名称：	Info_WifiLen
*	函数功能：	获取信息长度
*	入口参数：	sp：需要检查的信息-见说明
*	返回参数：	检查结果
*	说明：		获取0-ssid长度	1-pswd长度	
*				2-devid长度		3-apikey长度
************************************************************/
unsigned char Info_WifiLen(unsigned char sp)
{
//	unsigned char len = 0; 
//  switch(sp)
//    {
//        case 0:AT24C02_ReadByte(DEVID_ADDRESS, &len);		//读取长度值
//			         if(len == 0 || len >= 10)					//如果为0或超出
//				       return 1;
//        break;
//        
//        case 1:
//              AT24C02_ReadByte(AKEY_ADDRESS, &len);		//读取长度值
//			        if(len == 0 || len >= 30)					//如果为0或超出
//				      return 1;
//        break;
//    }
//	
	return 0;
}

/*
************************************************************
*	函数名称：	Info_CountLen
*	函数功能：	计算字段长度
*	入口参数：	info：需要检查的字段
*	返回参数：	字段长度
*	说明：		  计算串1发过来的字段长度   以"\r\n"结尾
************************************************************
*/
unsigned char Info_CountLen(char *info)
{
	unsigned char len = 0;
	char *buf = strstr(info, ":");		//找到':'
	buf++;								//偏移到下一个字节，代表字段信息开始
	while(1)
	{
		if(*buf == '\r')				//直到'\r'为止
			return len;
		
		buf++;
		len++;
	}

}



