#ifndef _NET_DEVICE_H_
#define _NET_DEVICE_H_


//=============================����==============================
//===========�����ṩRTOS���ڴ��������Ҳ����ʹ��C���=========
#include <stdlib.h>
#include "sys.h"
#define NET_MallocBuffer	malloc

#define NET_FreeBuffer		free
//==========================================================




typedef struct
{
	
	unsigned int net_time;
	
	unsigned short err : 2; 		//��������
	unsigned short initStep : 4;	//��ʼ������
	unsigned short dataType : 4;	//�趨���ݷ�������--16��
	unsigned short reboot : 1;		//����������־
	unsigned short netWork : 1;		//�������OK
	unsigned short reverse : 4;		//Ԥ��

} NET_DEVICE_INFO;

extern NET_DEVICE_INFO netDeviceInfo;

#define NET_RST_Pin GPIO_Pin_4
#define NET_RST_Port GPIOA

#define NET_DEVICE_RST_ON		GPIO_ResetBits(NET_RST_Port, NET_RST_Pin)
#define NET_DEVICE_RST_OFF	GPIO_SetBits(NET_RST_Port, NET_RST_Pin)

#define NET_DEVICE_TRANS		0 //1-ʹ��͸��ģʽ		0-ʧ��͸��ģʽ

#define PHONE_AP_MODE			1	//1-ʹ���ֻ��ȵ�ģʽ������Χ��·��ʱ��AirKiss�޷�ʹ�ã���ʹ�ù̶���ʽ����
									//0-ʹ��AirKiss�ķ�ʽ����

#define NET_TIME_EN				0	//1-��ȡ����ʱ��		0-����ȡ




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
