#ifndef _NET_DEVICE_H_
#define _NET_DEVICE_H_


//=============================配置==============================
//===========可以提供RTOS的内存管理方案，也可以使用C库的=========
#include <stdlib.h>
#include "sys.h"
#define NET_MallocBuffer	malloc

#define NET_FreeBuffer		free
//==========================================================




typedef struct
{
	
	unsigned int net_time;
	
	unsigned short err : 2; 		//错误类型
	unsigned short initStep : 4;	//初始化步骤
	unsigned short dataType : 4;	//设定数据返回类型--16种
	unsigned short reboot : 1;		//死机重启标志
	unsigned short netWork : 1;		//网络访问OK
	unsigned short reverse : 4;		//预留

} NET_DEVICE_INFO;

extern NET_DEVICE_INFO netDeviceInfo;

#define NET_RST_Pin GPIO_Pin_4
#define NET_RST_Port GPIOA

#define NET_DEVICE_RST_ON		GPIO_ResetBits(NET_RST_Port, NET_RST_Pin)
#define NET_DEVICE_RST_OFF	GPIO_SetBits(NET_RST_Port, NET_RST_Pin)

#define NET_DEVICE_TRANS		0 //1-使能透传模式		0-失能透传模式

#define PHONE_AP_MODE			1	//1-使用手机热点模式。当周围无路由时，AirKiss无法使用，则使用固定方式配置
									//0-使用AirKiss的方式配网

#define NET_TIME_EN				0	//1-获取网络时间		0-不获取




void NET_DEVICE_IO_Init(void);

_Bool NET_DEVICE_Exist(void);

_Bool NET_DEVICE_Init(char *ip, char *port);

void NET_DEVICE_Reset(void);

_Bool NET_DEVICE_ReLink(char *ip, char *port);

_Bool NET_DEVICE_SendCmd(char *cmd, char *res, _Bool mode);

void NET_DEVICE_SendData(unsigned char *data, unsigned short len);

_Bool NET_DEVICE_CheckListHead(void);

unsigned char *NET_DEVICE_GetListHeadBuf(void);

unsigned short NET_DEVICE_GetListHeadLen(void);

unsigned char NET_DEVICE_AddDataSendList(unsigned char *buf ,unsigned short dataLen);

_Bool NET_DEVICE_DeleteDataSendList(void);

unsigned char *NET_DEVICE_GetIPD(unsigned short timeOut);

void NET_DEVICE_ClrData(void);

unsigned char NET_DEVICE_Check(void);

void NET_DEVICE_ReConfig(unsigned char step);

#endif
