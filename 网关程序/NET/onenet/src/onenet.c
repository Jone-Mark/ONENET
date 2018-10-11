/**************************************************************
	*	�ļ����� 	onenet.c
	*	���ߣ� 		�ż���
	*	���ڣ� 		2017-05-27
	*	�汾�� 		V1.0
	*	˵���� 		OneNETƽ̨Ӧ��ʾ��
	*	�޸ļ�¼��	
*************************************************************/

//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"
//ͼƬ�����ļ�
#include "image_2k.h"
#include "net_device.h"
//Э���ļ�
#include "onenet.h"
#include "fault.h"
#include "edpkit.h"
//Ӳ������
#include "usart.h"
#include "delay.h"
#include "gpio.h"
#include "lcd.h"
//C��
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

ONETNET_INFO oneNetInfo = {"5109249", "BowZ4WDVhgPluXL1mrIoYZLacFw=",
							"183.230.40.39", "876",
							0, 0, 0, 1, 0};
/* ͨ��Э��
Э����<---->�ն�
            ����       ��д          ����
��ͷ       ���ֽ�      SD          �ԡ�$@����ͷ��ʮ�� �� �� Ϊ0x24,0x40
�ն˵�ַ   ���ֽ�      ADDR        �ն˵ĵ�ַ
������     һ�ֽ�      FC          ������
���ݳ���   һ�ֽ�      LEN         ������������ֽ�����û�����ݳ���Ϊ 0��
��������    N �ֽ�     DATA         0Ϊ�أ� 1Ϊ��
������     һ�ֽ�      XOR         ���ն˵�ַ��ʼ�����������ݵ����͡�
��β       ���ֽ�      ED          �ԡ�\r\n����β��ʮ �� �� �� Ϊ
*/
char SW_CMD[10]={0x24,0x40,0x00,0xff,0x02,01,0xff,0xff,0x0D,0x0A};//����ָ��

//==========================================================
//	�������ƣ�	OneNet_DevLink
//	�������ܣ�	��onenet��������
//	��ڲ�����	devid�������豸��devid���ƷID
//				auth_key�������豸��masterKey��apiKey���豸��Ȩ��Ϣ
//	���ز�����	��
//	˵����		��onenetƽ̨�������ӣ��ɹ������oneNetInfo.netWork����״̬��־
//==========================================================
void OneNet_DevLink(const char* devid, const char* auth_key)
{
	
	EDP_PACKET_STRUCTURE edpPacket = {NULL, 0, 0, 0};				//Э���

	unsigned char *dataPtr;
	
	UsartPrintf(USART_DEBUG, "OneNet_DevLink\r\n"
                        "DEVID: %s,     APIKEY: %s\r\n"
                        , devid, auth_key);
	

#if 1
	if(EDP_PacketConnect1(devid, auth_key, 256, &edpPacket) == 0)	//����devid �� apikey��װЭ���
#else
	if(EDP_PacketConnect2(proid, auth_key, 256, &edpPacket) == 0)	//���ݲ�Ʒid �� ��Ȩ��Ϣ��װЭ���
#endif
	
	{
		NET_DEVICE_SendData(edpPacket._data, edpPacket._len);		//�ϴ�ƽ̨
		
		dataPtr = NET_DEVICE_GetIPD(250);							//�ȴ�ƽ̨��Ӧ
		if(dataPtr != NULL)
		{
			if(EDP_UnPacketRecv(dataPtr) == CONNRESP)
			{
				switch(EDP_UnPacketConnectRsp(dataPtr))
				{
					case 0:UsartPrintf(USART_DEBUG, "Tips:	���ӳɹ�\r\n");oneNetInfo.netWork = 1;break;
					case 1:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ�Э�����\r\n");break;
					case 2:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ��豸ID��Ȩʧ��\r\n");break;
					case 3:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ�������ʧ��\r\n");break;
					case 4:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ��û�ID��Ȩʧ��\r\n");break;
					case 5:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ�δ��Ȩ\r\n");break;
					case 6:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ���Ȩ����Ч\r\n");break;
					case 7:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ�������δ����\r\n");break;
					case 8:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ����豸�ѱ�����\r\n");break;
					case 9:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ��ظ��������������\r\n");break;
					default:UsartPrintf(USART_DEBUG, "ERR:	����ʧ�ܣ�δ֪����\r\n");break;
				}
			}
		}
		
		EDP_DeleteBuffer(&edpPacket);								//ɾ��
	}
	else
		UsartPrintf(USART_DEBUG, "WARN:	EDP_PacketConnect Failed\r\n");
	
	if(oneNetInfo.netWork)											//�������ɹ�
	{
		oneNetInfo.errCount = 0;
	}
	else
	{
		if(++oneNetInfo.errCount >= 5)								//��������趨�����󣬻�δ����ƽ̨
		{
			oneNetInfo.netWork = 0;
			faultType = faultTypeReport = FAULT_NODEVICE;			//���ΪӲ������
		}
	}
	
}

//==========================================================
//	�������ƣ�	OneNet_PushData
//	�������ܣ�	PUSHDATA
//	��ڲ�����	dst_devid�������豸��devid
//				data����������
//				data_len�����ݳ���
//	���ز�����	0-���ͳɹ�	1-ʧ��
//	˵����		�豸���豸֮���ͨ��
//==========================================================
_Bool OneNet_PushData(const char* dst_devid, const char* data, unsigned int data_len)
{
	
	EDP_PACKET_STRUCTURE edpPacket = {NULL, 0, 0, 0};							//Э���
	
	if(!oneNetInfo.netWork)														//�������δ���� �� ��Ϊ�����շ�ģʽ
		return 1;
	
	if(EDP_PacketPushData(dst_devid, data, data_len, &edpPacket) == 0)
	{
		//NET_DEVICE_SendData(edpPacket._data, edpPacket._len);					//�ϴ�ƽ̨
		NET_DEVICE_AddDataSendList(edpPacket._data, edpPacket._len);			//��������
		
		EDP_DeleteBuffer(&edpPacket);											//ɾ��
	}
	else
		UsartPrintf(USART_DEBUG, "WARN:	OneNet_PushData Failed\r\n");
	
	return 0;

}

//==========================================================
//	�������ƣ�	OneNet_SendData
//	�������ܣ�	�ϴ����ݵ�ƽ̨
//	��ڲ�����	type���������ݵĸ�ʽ
//	���ز�����	SEND_TYPE_OK-���ͳɹ�	SEND_TYPE_DATA-��Ҫ����
//	˵����		
//==========================================================
unsigned char OneNet_SendData(FORMAT_TYPE type, char *devid, char *apikey, DATA_STREAM *streamArray, unsigned short streamArrayCnt)
{
	
	EDP_PACKET_STRUCTURE edpPacket = {NULL, 0, 0, 0};											//Э���
	
	_Bool status = SEND_TYPE_OK;
	short body_len = 0;
	
	if(!oneNetInfo.netWork)
		return SEND_TYPE_DATA;
	
	if(type < 1 && type > 5)
		return SEND_TYPE_DATA;
	
	//UsartPrintf(USART_DEBUG, "Tips:	OneNet_SendData-EDP_TYPE%d\r\n", type);
	
	if(type != kTypeBin)																		//�������ļ���ȫ���������ã�����Ҫִ����Щ
	{
		body_len = DSTREAM_GetDataStream_Body_Measure(type, streamArray, streamArrayCnt, 0);	//��ȡ��ǰ��Ҫ���͵����������ܳ���
		
		if(body_len)
		{
			if(EDP_PacketSaveData(devid, body_len, NULL, (SaveDataType)type, &edpPacket) == 0)	//���
			{
				body_len = DSTREAM_GetDataStream_Body(type, streamArray, streamArrayCnt, edpPacket._data, edpPacket._size, edpPacket._len);
				
				if(body_len)
				{
					edpPacket._len += body_len;
					//NET_DEVICE_SendData(edpPacket._data, edpPacket._len);						//�ϴ����ݵ�ƽ̨
					NET_DEVICE_AddDataSendList(edpPacket._data, edpPacket._len);				//��������
					//UsartPrintf(USART_DEBUG, "Send %d Bytes\r\n", edpPacket._len);
				}
				else
					UsartPrintf(USART_DEBUG, "WARN:	DSTREAM_GetDataStream_Body Failed\r\n");
				
				EDP_DeleteBuffer(&edpPacket);													//ɾ��
			}
			else
				UsartPrintf(USART_DEBUG, "WARN:	EDP_NewBuffer Failed\r\n");
		}
		else
			status = SEND_TYPE_DATA;
	}
	else
	{
		OneNet_SendData_Picture(devid, Array, sizeof(Array));
	}
	
	faultTypeReport = FAULT_NONE;																			//����֮��������
	
	return status;
	
}

//==========================================================
//	�������ƣ�	OneNet_SendData_EDPType2
//
//	�������ܣ�	�ϴ����������ݵ�ƽ̨
//
//	��ڲ�����	devid���豸ID(�Ƽ�ΪNULL)
//				picture��ͼƬ����
//				pic_len��ͼƬ���ݳ���
//
//	���ز�����	��
//
//	˵����		���ǵ����豸����������ʱ������ʹ�������豸��͸��ģʽ
//				�ϴ�ͼƬ�ǣ�ǿ�ҽ���devid�ֶ�Ϊ�գ�����ƽ̨�ὫͼƬ�����·����豸
//==========================================================
#define PKT_SIZE 1024
void OneNet_SendData_Picture(char *devid, const char* picture, unsigned int pic_len)
{
	
	EDP_PACKET_STRUCTURE edpPacket = {NULL, 0, 0, 0};					//Э���

	char type_bin_head[] = "{\"ds_id\":\"pic\"}";						//ͼƬ����ͷ
	unsigned char *pImage = (unsigned char *)picture;
	
	if(EDP_PacketSaveData(devid, pic_len, type_bin_head, kTypeBin, &edpPacket) == 0)
	{	
		NET_DEVICE_ClrData();
		
		UsartPrintf(USART_DEBUG, "Send %d Bytes\r\n", edpPacket._len);
		NET_DEVICE_SendData(edpPacket._data, edpPacket._len);			//�ϴ����ݵ�ƽ̨
		
		EDP_DeleteBuffer(&edpPacket);									//ɾ��
		
		UsartPrintf(USART_DEBUG, "image len = %d\r\n", pic_len);
		
		while(pic_len > 0)
		{
			DelayXms(100);												//��ͼʱ��ʱ�������һ�㣬�����������һ����ʱ
			
			if(pic_len >= PKT_SIZE)
			{
				NET_DEVICE_SendData(pImage, PKT_SIZE);					//���ڷ��ͷ�Ƭ
				
				pImage += PKT_SIZE;
				pic_len -= PKT_SIZE;
			}
			else
			{
				NET_DEVICE_SendData(pImage, (unsigned short)pic_len);	//���ڷ������һ����Ƭ
				pic_len = 0;
			}
		}
		
		UsartPrintf(USART_DEBUG, "image send ok\r\n");
	}
	else
		UsartPrintf(USART_DEBUG, "EDP_PacketSaveData Failed\r\n");
}

//==========================================================
//	�������ƣ�	OneNet_HeartBeat
//	�������ܣ�	������������
//	��ڲ�����	��
//	���ز�����	SEND_TYPE_OK-���ͳɹ�	SEND_TYPE_HEART-��Ҫ����
//	˵����		
//==========================================================
unsigned char OneNet_SendData_Heart(void)
{
	
	EDP_PACKET_STRUCTURE edpPacket = {NULL, 0, 0, 0};		//Э���
	
	if(!oneNetInfo.netWork)									//�������Ϊ���� �� ��Ϊ�����շ�ģʽ
		return SEND_TYPE_HEART;
	
	if(EDP_PacketPing(&edpPacket))
		return SEND_TYPE_HEART;
	
	oneNetInfo.heartBeat = 0;
	
	//NET_DEVICE_SendData(edpPacket._data, edpPacket._len);	//��ƽ̨�ϴ���������
	NET_DEVICE_AddDataSendList(edpPacket._data, edpPacket._len);//��������
	
	EDP_DeleteBuffer(&edpPacket);							//ɾ��
	
	return SEND_TYPE_OK;
	
}

//==========================================================
//	�������ƣ�	OneNet_HeartBeat_Check
//
//	�������ܣ�	������������������
//
//	��ڲ�����	��
//
//	���ز�����	0-�ɹ�	1-�ȴ�
//
//	˵����		���ڵ���ʱ����runCountÿ���˺�������һ�ε�ʱ������
//				�ﵽ�趨���޼��������־λ�Ƿ����
//				����ʱ����Բ���̫��ȷ
//==========================================================
_Bool OneNet_Check_Heart(void)
{
	
	static unsigned char runCount = 0;
	
	if(!oneNetInfo.netWork)
		return 1;

	if(oneNetInfo.heartBeat == 1)
	{
		runCount = 0;
		oneNetInfo.errCount = 0;
		
		return 0;
	}
	
	if(++runCount >= 40)
	{
		runCount = 0;
		
		UsartPrintf(USART_DEBUG, "HeartBeat TimeOut: %d\r\n", oneNetInfo.errCount);
		oneNetInfo.sendData = SEND_TYPE_HEART;		//������������
		
		if(++oneNetInfo.errCount >= 3)
		{
			unsigned char errType = 0;
			
			oneNetInfo.errCount = 0;
			
			errType = NET_DEVICE_Check();											//�����豸״̬���
			if(errType == CHECK_CONNECTED || errType == CHECK_CLOSED || errType == CHECK_GOT_IP)
				faultTypeReport = faultType = FAULT_PRO;								//���ΪЭ�����
			else if(errType == CHECK_NO_DEVICE)
				faultTypeReport = faultType = FAULT_NODEVICE;							//���Ϊ�豸����
			else
				faultTypeReport = faultType = FAULT_NONE;								//�޴���
		}
	}
	
	return 1;

}

//==========================================================
//	�������ƣ�	OneNet_RevPro
//	�������ܣ�	ƽ̨�������ݼ��
//	��ڲ�����	dataPtr��ƽ̨���ص�����
//	���ز�����	��
//	˵����		
//==========================================================
void OneNet_RevPro(unsigned char *cmd)
{
	
	EDP_PACKET_STRUCTURE edpPacket = {NULL, 0, 0, 0};	//Э���
	
	char *cmdid_devid = NULL;
	unsigned short cmdid_len = 0;
	char *req = NULL;
	unsigned int req_len = 0;
	unsigned char type = 0;
	
	short result = 0;

	char *dataPtr = NULL;
	char numBuf[10];
	int num = 0;
	
	int state1,state2,state3,state4;
	
	type = EDP_UnPacketRecv(cmd);
	switch(type)										//�ж���pushdata���������·�
	{
		case PINGRESP:
		
			UsartPrintf(USART_DEBUG, "Tips:	HeartBeat OK\r\n");
			oneNetInfo.heartBeat = 1;
		
		break;
		
		case PUSHDATA:									//��pushdata��
			
			result = EDP_UnPacketPushData(cmd, &cmdid_devid, &req, &req_len);
		
			if(result == 0)
				//UsartPrintf(USART_DEBUG, "src_devid: %s, req: %s, req_len: %d\r\n", cmdid_devid, req, req_len);
			
		break;
		
		case CMDREQ:									//�������
			
			result = EDP_UnPacketCmd(cmd, &cmdid_devid, &cmdid_len, &req, &req_len);
			
			if(result == 0)								//����ɹ������������ظ������
			{
				EDP_PacketCmdResp(cmdid_devid, cmdid_len, req, req_len, &edpPacket);
				//UsartPrintf(USART_DEBUG, "cmdid: %s, req: %s, req_len: %d\r\n", cmdid_devid, req, req_len);
			}
			
		break;
			
		case SAVEACK:
			
			if(cmd[3] == MSG_ID_HIGH && cmd[4] == MSG_ID_LOW)
			{
				//UsartPrintf(USART_DEBUG, "Tips:	Send %s\r\n", cmd[5] ? "Err" : "Ok");
			}
			else
				UsartPrintf(USART_DEBUG, "Tips:	Message ID Err\r\n");
			
		break;
			
		default:
			result = -1;
		break;
	}
	
	NET_DEVICE_ClrData();								//��ջ���
	
	if(result == -1)
		return;
	
	dataPtr = strchr(req, ':');							//����':'

	if(dataPtr != NULL && result != -1)					//����ҵ���
	{
		dataPtr++;
		
		while(*dataPtr >= '0' && *dataPtr <= '9')		//�ж��Ƿ����·��������������
		{
			numBuf[num++] = *dataPtr++;
		}
		
		num = atoi((const char *)numBuf);				//תΪ��ֵ��ʽ
		
		if(strstr((char *)req, "light"))				//����"redled"
		{
			if(num == 1)								//�����������Ϊ1������
			{
				state1=1;
				SW_CMD[3]=0x01;//�ն˵�ַΪ1
				SW_CMD[6]=0x01;//on
				SW_CMD[7]=0x03;//XOR �ݲ�����
				Usart_SendString(USART_DEBUG,(unsigned char*)SW_CMD,10); //���Ϳ�����������
				LED1_ON;    //LED0��
			}
			else if(num == 0)							//�����������Ϊ0�������
			{
				state1=0;
				SW_CMD[3]=0x01;//�ն˵�ַΪ1
				SW_CMD[6]=0x00;//off
				SW_CMD[7]=0x02;//XOR �ݲ�����
				Usart_SendString(USART_DEBUG,(unsigned char*)SW_CMD,10); //���Ϳ�����������
				LED1_OFF;   //LED0��
			}
			LCD_ShowxNum(240,525,state1,1,24,0X80);
		}
														//��ͬ
		else if(strstr((char *)req, "irrig"))
		{
			if(num == 1)
			{
				state2=1;
				SW_CMD[3]=0x02;//�ն˵�ַΪ2
				SW_CMD[6]=0x01;//on
				SW_CMD[7]=0x00;//XOR �ݲ�����
				Usart_SendString(USART_DEBUG,(unsigned char*)SW_CMD,10); //���Ϳ�����������
				LED1_ON;    //LED0��
			}
			else if(num == 0)
			{
				state2=0;
				SW_CMD[3]=0x02;//�ն˵�ַΪ2
				SW_CMD[6]=0x00;//off
				SW_CMD[7]=0x01;//XOR �ݲ�����
				Usart_SendString(USART_DEBUG,(unsigned char*)SW_CMD,10); //���Ϳ�����������
				LED1_OFF;   //LED0��
			}
			LCD_ShowxNum(240,550,state2,1,24,0X80);
		}
		else if(strstr((char *)req, "venti"))
		{
			if(num == 1)
			{
				state3=1;
				SW_CMD[3]=0x03;//�ն˵�ַΪ3
				SW_CMD[6]=0x01;//on
				SW_CMD[7]=0x01;//XOR �ݲ�����
				Usart_SendString(USART_DEBUG,(unsigned char*)SW_CMD,10); //���Ϳ�����������
				LED1_ON;    //LED1��
			}
			else if(num == 0)
			{
				state3=0;
				SW_CMD[3]=0x03;//�ն˵�ַΪ3
				SW_CMD[6]=0x00;//off
				SW_CMD[7]=0x00;//XOR �ݲ�����
				Usart_SendString(USART_DEBUG,(unsigned char*)SW_CMD,10); //���Ϳ�����������
				LED1_OFF;   //LED1��
			}
			LCD_ShowxNum(240,575,state3,1,24,0X80);
		}
		else if(strstr((char *)req, "spray"))
		{
			if(num == 1)
			{ 
				state4=1;
				SW_CMD[3]=0x04;//�ն˵�ַΪ4
				SW_CMD[6]=0x01;//on
				SW_CMD[7]=0x06;//XOR �ݲ�����
				Usart_SendString(USART_DEBUG,(unsigned char*)SW_CMD,10); //���Ϳ�����������
				LED1_ON;    //LED1��
			}
			else if(num == 0)
			{
				state4=0;
				SW_CMD[3]=0x04;//�ն˵�ַΪ4
				SW_CMD[6]=0x00;//off
				SW_CMD[7]=0x07;//XOR �ݲ�����
				Usart_SendString(USART_DEBUG,(unsigned char*)SW_CMD,10); //���Ϳ�����������
				LED1_OFF;   //LED1��
			}
			LCD_ShowxNum(240,600,state4,1,24,0X80);
		}
	}
	
	if(type == PUSHDATA && result == 0)					//�����pushdata �� ����ɹ�
	{
		EDP_FreeBuffer(cmdid_devid);					//�ͷ��ڴ�
		EDP_FreeBuffer(req);
	}
	else if(type == CMDREQ && result == 0)				//���������� �� ����ɹ�
	{
		EDP_FreeBuffer(cmdid_devid);					//�ͷ��ڴ�
		EDP_FreeBuffer(req);
														//�ظ�����
		//NET_DEVICE_SendData(edpPacket._data, edpPacket._len);//�ϴ�ƽ̨
		NET_DEVICE_AddDataSendList(edpPacket._data, edpPacket._len);//��������
		EDP_DeleteBuffer(&edpPacket);					//ɾ��
		oneNetInfo.sendData = SEND_TYPE_DATA;			//������ݷ���
	}

}
