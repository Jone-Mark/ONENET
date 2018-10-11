/**************************************************************
	*	ÎÄ¼şÃû£º 	onenet.c
	*	×÷Õß£º 		ÕÅ¼ÌÈğ
	*	ÈÕÆÚ£º 		2017-05-27
	*	°æ±¾£º 		V1.0
	*	ËµÃ÷£º 		OneNETÆ½Ì¨Ó¦ÓÃÊ¾Àı
	*	ĞŞ¸Ä¼ÇÂ¼£º	
*************************************************************/

//µ¥Æ¬»úÍ·ÎÄ¼ş
#include "stm32f10x.h"
//Í¼Æ¬Êı¾İÎÄ¼ş
#include "image_2k.h"
#include "net_device.h"
//Ğ­ÒéÎÄ¼ş
#include "onenet.h"
#include "fault.h"
#include "edpkit.h"
//Ó²¼şÇı¶¯
#include "usart.h"
#include "delay.h"
#include "gpio.h"
#include "lcd.h"
//C¿â
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

ONETNET_INFO oneNetInfo = {"5109249", "BowZ4WDVhgPluXL1mrIoYZLacFw=",
							"183.230.40.39", "876",
							0, 0, 0, 1, 0};
/* Í¨ĞÅĞ­Òé£
Ğ­µ÷Æ÷<---->ÖÕ¶Ë
            ³¤¶È       ËõĞ´          ÃèÊö
°üÍ·       ¶ş×Ö½Ú      SD          ÒÔ¡°$@¡±¿ªÍ·£¬Ê®Áù ½ø ÖÆ Îª0x24,0x40
ÖÕ¶ËµØÖ·   ¶ş×Ö½Ú      ADDR        ÖÕ¶ËµÄµØÖ·
¹¦ÄÜÂë     Ò»×Ö½Ú      FC          ÃüÁîÂë
Êı¾İ³¤¶È   Ò»×Ö½Ú      LEN         ºóÃæ´øµÄÊı¾İ×Ö½ÚÊı£¬Ã»ÓĞÊı¾İ³¤¶ÈÎª 0¡£
Êı¾İÄÚÈİ    N ×Ö½Ú     DATA         0Îª¹Ø£» 1Îª¿ª
¼ìÑéÂë     Ò»×Ö½Ú      XOR         ´ÓÖÕ¶ËµØÖ·¿ªÊ¼£¬µ½Êı¾İÄÚÈİµÄÒì»òºÍ¡£
°üÎ²       ¶ş×Ö½Ú      ED          ÒÔ¡®\r\n¡¯½áÎ²£¬Ê® Áù ½ø ÖÆ Îª
*/
char SW_CMD[10]={0x24,0x40,0x00,0xff,0x02,01,0xff,0xff,0x0D,0x0A};//·¢ËÍÖ¸Áî

//==========================================================
//	º¯ÊıÃû³Æ£º	OneNet_DevLink
//	º¯Êı¹¦ÄÜ£º	Óëonenet´´½¨Á¬½Ó
//	Èë¿Ú²ÎÊı£º	devid£º´´½¨Éè±¸µÄdevid»ò²úÆ·ID
//				auth_key£º´´½¨Éè±¸µÄmasterKey»òapiKey»òÉè±¸¼øÈ¨ĞÅÏ¢
//	·µ»Ø²ÎÊı£º	ÎŞ
//	ËµÃ÷£º		ÓëonenetÆ½Ì¨½¨Á¢Á¬½Ó£¬³É¹¦»ò»á±ê¼ÇoneNetInfo.netWorkÍøÂç×´Ì¬±êÖ¾
//==========================================================
void OneNet_DevLink(const char* devid, const char* auth_key)
{
	
	EDP_PACKET_STRUCTURE edpPacket = {NULL, 0, 0, 0};				//Ğ­Òé°ü

	unsigned char *dataPtr;
	
	UsartPrintf(USART_DEBUG, "OneNet_DevLink\r\n"
                        "DEVID: %s,     APIKEY: %s\r\n"
                        , devid, auth_key);
	

#if 1
	if(EDP_PacketConnect1(devid, auth_key, 256, &edpPacket) == 0)	//¸ù¾İdevid ºÍ apikey·â×°Ğ­Òé°ü
#else
	if(EDP_PacketConnect2(proid, auth_key, 256, &edpPacket) == 0)	//¸ù¾İ²úÆ·id ºÍ ¼øÈ¨ĞÅÏ¢·â×°Ğ­Òé°ü
#endif
	
	{
		NET_DEVICE_SendData(edpPacket._data, edpPacket._len);		//ÉÏ´«Æ½Ì¨
		
		dataPtr = NET_DEVICE_GetIPD(250);							//µÈ´ıÆ½Ì¨ÏìÓ¦
		if(dataPtr != NULL)
		{
			if(EDP_UnPacketRecv(dataPtr) == CONNRESP)
			{
				switch(EDP_UnPacketConnectRsp(dataPtr))
				{
					case 0:UsartPrintf(USART_DEBUG, "Tips:	Á¬½Ó³É¹¦\r\n");oneNetInfo.netWork = 1;break;
					case 1:UsartPrintf(USART_DEBUG, "WARN:	Á¬½ÓÊ§°Ü£ºĞ­Òé´íÎó\r\n");break;
					case 2:UsartPrintf(USART_DEBUG, "WARN:	Á¬½ÓÊ§°Ü£ºÉè±¸ID¼øÈ¨Ê§°Ü\r\n");break;
					case 3:UsartPrintf(USART_DEBUG, "WARN:	Á¬½ÓÊ§°Ü£º·şÎñÆ÷Ê§°Ü\r\n");break;
					case 4:UsartPrintf(USART_DEBUG, "WARN:	Á¬½ÓÊ§°Ü£ºÓÃ»§ID¼øÈ¨Ê§°Ü\r\n");break;
					case 5:UsartPrintf(USART_DEBUG, "WARN:	Á¬½ÓÊ§°Ü£ºÎ´ÊÚÈ¨\r\n");break;
					case 6:UsartPrintf(USART_DEBUG, "WARN:	Á¬½ÓÊ§°Ü£ºÊÚÈ¨ÂëÎŞĞ§\r\n");break;
					case 7:UsartPrintf(USART_DEBUG, "WARN:	Á¬½ÓÊ§°Ü£º¼¤»îÂëÎ´·ÖÅä\r\n");break;
					case 8:UsartPrintf(USART_DEBUG, "WARN:	Á¬½ÓÊ§°Ü£º¸ÃÉè±¸ÒÑ±»¼¤»î\r\n");break;
					case 9:UsartPrintf(USART_DEBUG, "WARN:	Á¬½ÓÊ§°Ü£ºÖØ¸´·¢ËÍÁ¬½ÓÇëÇó°ü\r\n");break;
					default:UsartPrintf(USART_DEBUG, "ERR:	Á¬½ÓÊ§°Ü£ºÎ´Öª´íÎó\r\n");break;
				}
			}
		}
		
		EDP_DeleteBuffer(&edpPacket);								//É¾°ü
	}
	else
		UsartPrintf(USART_DEBUG, "WARN:	EDP_PacketConnect Failed\r\n");
	
	if(oneNetInfo.netWork)											//Èç¹û½ÓÈë³É¹¦
	{
		oneNetInfo.errCount = 0;
	}
	else
	{
		if(++oneNetInfo.errCount >= 5)								//Èç¹û³¬¹ıÉè¶¨´ÎÊıºó£¬»¹Î´½ÓÈëÆ½Ì¨
		{
			oneNetInfo.netWork = 0;
			faultType = faultTypeReport = FAULT_NODEVICE;			//±ê¼ÇÎªÓ²¼ş´íÎó
		}
	}
	
}

//==========================================================
//	º¯ÊıÃû³Æ£º	OneNet_PushData
//	º¯Êı¹¦ÄÜ£º	PUSHDATA
//	Èë¿Ú²ÎÊı£º	dst_devid£º½ÓÊÕÉè±¸µÄdevid
//				data£ºÊı¾İÄÚÈİ
//				data_len£ºÊı¾İ³¤¶È
//	·µ»Ø²ÎÊı£º	0-·¢ËÍ³É¹¦	1-Ê§°Ü
//	ËµÃ÷£º		Éè±¸ÓëÉè±¸Ö®¼äµÄÍ¨ĞÅ
//==========================================================
_Bool OneNet_PushData(const char* dst_devid, const char* data, unsigned int data_len)
{
	
	EDP_PACKET_STRUCTURE edpPacket = {NULL, 0, 0, 0};							//Ğ­Òé°ü
	
	if(!oneNetInfo.netWork)														//Èç¹ûÍøÂçÎ´Á¬½Ó »ò ²»ÎªÊı¾İÊÕ·¢Ä£Ê½
		return 1;
	
	if(EDP_PacketPushData(dst_devid, data, data_len, &edpPacket) == 0)
	{
		//NET_DEVICE_SendData(edpPacket._data, edpPacket._len);					//ÉÏ´«Æ½Ì¨
		NET_DEVICE_AddDataSendList(edpPacket._data, edpPacket._len);			//¼ÓÈëÁ´±í
		
		EDP_DeleteBuffer(&edpPacket);											//É¾°ü
	}
	else
		UsartPrintf(USART_DEBUG, "WARN:	OneNet_PushData Failed\r\n");
	
	return 0;

}

//==========================================================
//	º¯ÊıÃû³Æ£º	OneNet_SendData
//	º¯Êı¹¦ÄÜ£º	ÉÏ´«Êı¾İµ½Æ½Ì¨
//	Èë¿Ú²ÎÊı£º	type£º·¢ËÍÊı¾İµÄ¸ñÊ½
//	·µ»Ø²ÎÊı£º	SEND_TYPE_OK-·¢ËÍ³É¹¦	SEND_TYPE_DATA-ĞèÒªÖØËÍ
//	ËµÃ÷£º		
//==========================================================
unsigned char OneNet_SendData(FORMAT_TYPE type, char *devid, char *apikey, DATA_STREAM *streamArray, unsigned short streamArrayCnt)
{
	
	EDP_PACKET_STRUCTURE edpPacket = {NULL, 0, 0, 0};											//Ğ­Òé°ü
	
	_Bool status = SEND_TYPE_OK;
	short body_len = 0;
	
	if(!oneNetInfo.netWork)
		return SEND_TYPE_DATA;
	
	if(type < 1 && type > 5)
		return SEND_TYPE_DATA;
	
	//UsartPrintf(USART_DEBUG, "Tips:	OneNet_SendData-EDP_TYPE%d\r\n", type);
	
	if(type != kTypeBin)																		//¶ş½øÖÆÎÄ¼ş°ÉÈ«²¿¹¤×÷×öºÃ£¬²»ĞèÒªÖ´ĞĞÕâĞ©
	{
		body_len = DSTREAM_GetDataStream_Body_Measure(type, streamArray, streamArrayCnt, 0);	//»ñÈ¡µ±Ç°ĞèÒª·¢ËÍµÄÊı¾İÁ÷µÄ×Ü³¤¶È
		
		if(body_len)
		{
			if(EDP_PacketSaveData(devid, body_len, NULL, (SaveDataType)type, &edpPacket) == 0)	//·â°ü
			{
				body_len = DSTREAM_GetDataStream_Body(type, streamArray, streamArrayCnt, edpPacket._data, edpPacket._size, edpPacket._len);
				
				if(body_len)
				{
					edpPacket._len += body_len;
					//NET_DEVICE_SendData(edpPacket._data, edpPacket._len);						//ÉÏ´«Êı¾İµ½Æ½Ì¨
					NET_DEVICE_AddDataSendList(edpPacket._data, edpPacket._len);				//¼ÓÈëÁ´±í
					//UsartPrintf(USART_DEBUG, "Send %d Bytes\r\n", edpPacket._len);
				}
				else
					UsartPrintf(USART_DEBUG, "WARN:	DSTREAM_GetDataStream_Body Failed\r\n");
				
				EDP_DeleteBuffer(&edpPacket);													//É¾°ü
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
	
	faultTypeReport = FAULT_NONE;																			//·¢ËÍÖ®ºóÇå³ı±ê¼Ç
	
	return status;
	
}

//==========================================================
//	º¯ÊıÃû³Æ£º	OneNet_SendData_EDPType2
//
//	º¯Êı¹¦ÄÜ£º	ÉÏ´«¶ş½øÖÆÊı¾İµ½Æ½Ì¨
//
//	Èë¿Ú²ÎÊı£º	devid£ºÉè±¸ID(ÍÆ¼öÎªNULL)
//				picture£ºÍ¼Æ¬Êı¾İ
//				pic_len£ºÍ¼Æ¬Êı¾İ³¤¶È
//
//	·µ»Ø²ÎÊı£º	ÎŞ
//
//	ËµÃ÷£º		ÈôÊÇµÍËÙÉè±¸£¬Êı¾İÁ¿´óÊ±£¬½¨ÒéÊ¹ÓÃÍøÂçÉè±¸µÄÍ¸´«Ä£Ê½
//				ÉÏ´«Í¼Æ¬ÊÇ£¬Ç¿ÁÒ½¨Òédevid×Ö¶ÎÎª¿Õ£¬·ñÔòÆ½Ì¨»á½«Í¼Æ¬Êı¾İÏÂ·¢µ½Éè±¸
//==========================================================
#define PKT_SIZE 1024
void OneNet_SendData_Picture(char *devid, const char* picture, unsigned int pic_len)
{
	
	EDP_PACKET_STRUCTURE edpPacket = {NULL, 0, 0, 0};					//Ğ­Òé°ü

	char type_bin_head[] = "{\"ds_id\":\"pic\"}";						//Í¼Æ¬Êı¾İÍ·
	unsigned char *pImage = (unsigned char *)picture;
	
	if(EDP_PacketSaveData(devid, pic_len, type_bin_head, kTypeBin, &edpPacket) == 0)
	{	
		NET_DEVICE_ClrData();
		
		UsartPrintf(USART_DEBUG, "Send %d Bytes\r\n", edpPacket._len);
		NET_DEVICE_SendData(edpPacket._data, edpPacket._len);			//ÉÏ´«Êı¾İµ½Æ½Ì¨
		
		EDP_DeleteBuffer(&edpPacket);									//É¾°ü
		
		UsartPrintf(USART_DEBUG, "image len = %d\r\n", pic_len);
		
		while(pic_len > 0)
		{
			DelayXms(100);												//´«Í¼Ê±£¬Ê±¼ä¼ä¸ô»á´óÒ»µã£¬ÕâÀï¶îÍâÔö¼ÓÒ»¸öÑÓÊ±
			
			if(pic_len >= PKT_SIZE)
			{
				NET_DEVICE_SendData(pImage, PKT_SIZE);					//´®¿Ú·¢ËÍ·ÖÆ¬
				
				pImage += PKT_SIZE;
				pic_len -= PKT_SIZE;
			}
			else
			{
				NET_DEVICE_SendData(pImage, (unsigned short)pic_len);	//´®¿Ú·¢ËÍ×îºóÒ»¸ö·ÖÆ¬
				pic_len = 0;
			}
		}
		
		UsartPrintf(USART_DEBUG, "image send ok\r\n");
	}
	else
		UsartPrintf(USART_DEBUG, "EDP_PacketSaveData Failed\r\n");
}

//==========================================================
//	º¯ÊıÃû³Æ£º	OneNet_HeartBeat
//	º¯Êı¹¦ÄÜ£º	·¢ËÍĞÄÌøÇëÇó
//	Èë¿Ú²ÎÊı£º	ÎŞ
//	·µ»Ø²ÎÊı£º	SEND_TYPE_OK-·¢ËÍ³É¹¦	SEND_TYPE_HEART-ĞèÒªÖØËÍ
//	ËµÃ÷£º		
//==========================================================
unsigned char OneNet_SendData_Heart(void)
{
	
	EDP_PACKET_STRUCTURE edpPacket = {NULL, 0, 0, 0};		//Ğ­Òé°ü
	
	if(!oneNetInfo.netWork)									//Èç¹ûÍøÂçÎªÁ¬½Ó »ò ²»ÎªÊı¾İÊÕ·¢Ä£Ê½
		return SEND_TYPE_HEART;
	
	if(EDP_PacketPing(&edpPacket))
		return SEND_TYPE_HEART;
	
	oneNetInfo.heartBeat = 0;
	
	//NET_DEVICE_SendData(edpPacket._data, edpPacket._len);	//ÏòÆ½Ì¨ÉÏ´«ĞÄÌøÇëÇó
	NET_DEVICE_AddDataSendList(edpPacket._data, edpPacket._len);//¼ÓÈëÁ´±í
	
	EDP_DeleteBuffer(&edpPacket);							//É¾°ü
	
	return SEND_TYPE_OK;
	
}

//==========================================================
//	º¯ÊıÃû³Æ£º	OneNet_HeartBeat_Check
//
//	º¯Êı¹¦ÄÜ£º	·¢ËÍĞÄÌøºóµÄĞÄÌø¼ì²â
//
//	Èë¿Ú²ÎÊı£º	ÎŞ
//
//	·µ»Ø²ÎÊı£º	0-³É¹¦	1-µÈ´ı
//
//	ËµÃ÷£º		»ùÓÚµ÷ÓÃÊ±»ù£¬runCountÃ¿¸ô´Ëº¯Êıµ÷ÓÃÒ»´ÎµÄÊ±¼ä×ÔÔö
//				´ïµ½Éè¶¨ÉÏÏŞ¼ì²âĞÄÌø±êÖ¾Î»ÊÇ·ñ¾ÍĞ÷
//				ÉÏÏŞÊ±¼ä¿ÉÒÔ²»ÓÃÌ«¾«È·
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
		oneNetInfo.sendData = SEND_TYPE_HEART;		//·¢ËÍĞÄÌøÇëÇó
		
		if(++oneNetInfo.errCount >= 3)
		{
			unsigned char errType = 0;
			
			oneNetInfo.errCount = 0;
			
			errType = NET_DEVICE_Check();											//ÍøÂçÉè±¸×´Ì¬¼ì²é
			if(errType == CHECK_CONNECTED || errType == CHECK_CLOSED || errType == CHECK_GOT_IP)
				faultTypeReport = faultType = FAULT_PRO;								//±ê¼ÇÎªĞ­Òé´íÎó
			else if(errType == CHECK_NO_DEVICE)
				faultTypeReport = faultType = FAULT_NODEVICE;							//±ê¼ÇÎªÉè±¸´íÎó
			else
				faultTypeReport = faultType = FAULT_NONE;								//ÎŞ´íÎó
		}
	}
	
	return 1;

}

//==========================================================
//	º¯ÊıÃû³Æ£º	OneNet_RevPro
//	º¯Êı¹¦ÄÜ£º	Æ½Ì¨·µ»ØÊı¾İ¼ì²â
//	Èë¿Ú²ÎÊı£º	dataPtr£ºÆ½Ì¨·µ»ØµÄÊı¾İ
//	·µ»Ø²ÎÊı£º	ÎŞ
//	ËµÃ÷£º		
//==========================================================
void OneNet_RevPro(unsigned char *cmd)
{
	
	EDP_PACKET_STRUCTURE edpPacket = {NULL, 0, 0, 0};	//Ğ­Òé°ü
	
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
	switch(type)										//ÅĞ¶ÏÊÇpushdata»¹ÊÇÃüÁîÏÂ·¢
	{
		case PINGRESP:
		
			UsartPrintf(USART_DEBUG, "Tips:	HeartBeat OK\r\n");
			oneNetInfo.heartBeat = 1;
		
		break;
		
		case PUSHDATA:									//½âpushdata°ü
			
			result = EDP_UnPacketPushData(cmd, &cmdid_devid, &req, &req_len);
		
			if(result == 0)
				//UsartPrintf(USART_DEBUG, "src_devid: %s, req: %s, req_len: %d\r\n", cmdid_devid, req, req_len);
			
		break;
		
		case CMDREQ:									//½âÃüÁî°ü
			
			result = EDP_UnPacketCmd(cmd, &cmdid_devid, &cmdid_len, &req, &req_len);
			
			if(result == 0)								//½â°ü³É¹¦£¬Ôò½øĞĞÃüÁî»Ø¸´µÄ×é°ü
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
	
	NET_DEVICE_ClrData();								//Çå¿Õ»º´æ
	
	if(result == -1)
		return;
	
	dataPtr = strchr(req, ':');							//ËÑË÷':'

	if(dataPtr != NULL && result != -1)					//Èç¹ûÕÒµ½ÁË
	{
		dataPtr++;
		
		while(*dataPtr >= '0' && *dataPtr <= '9')		//ÅĞ¶ÏÊÇ·ñÊÇÏÂ·¢µÄÃüÁî¿ØÖÆÊı¾İ
		{
			numBuf[num++] = *dataPtr++;
		}
		
		num = atoi((const char *)numBuf);				//×ªÎªÊıÖµĞÎÊ½
		
		if(strstr((char *)req, "light"))				//ËÑË÷"redled"
		{
			if(num == 1)								//¿ØÖÆÊı¾İÈç¹ûÎª1£¬´ú±í¿ª
			{
				state1=1;
				SW_CMD[3]=0x01;//ÖÕ¶ËµØÖ·Îª1
				SW_CMD[6]=0x01;//on
				SW_CMD[7]=0x03;//XOR Ôİ²»¼ÆËã
				Usart_SendString(USART_DEBUG,(unsigned char*)SW_CMD,10); //·¢ËÍ¿ØÖÆÏÂĞĞÃüÁî
				LED1_ON;    //LED0¿ª
			}
			else if(num == 0)							//¿ØÖÆÊı¾İÈç¹ûÎª0£¬´ú±í¹Ø
			{
				state1=0;
				SW_CMD[3]=0x01;//ÖÕ¶ËµØÖ·Îª1
				SW_CMD[6]=0x00;//off
				SW_CMD[7]=0x02;//XOR Ôİ²»¼ÆËã
				Usart_SendString(USART_DEBUG,(unsigned char*)SW_CMD,10); //·¢ËÍ¿ØÖÆÏÂĞĞÃüÁî
				LED1_OFF;   //LED0¹Ø
			}
			LCD_ShowxNum(240,525,state1,1,24,0X80);
		}
														//ÏÂÍ¬
		else if(strstr((char *)req, "irrig"))
		{
			if(num == 1)
			{
				state2=1;
				SW_CMD[3]=0x02;//ÖÕ¶ËµØÖ·Îª2
				SW_CMD[6]=0x01;//on
				SW_CMD[7]=0x00;//XOR Ôİ²»¼ÆËã
				Usart_SendString(USART_DEBUG,(unsigned char*)SW_CMD,10); //·¢ËÍ¿ØÖÆÏÂĞĞÃüÁî
				LED1_ON;    //LED0¿ª
			}
			else if(num == 0)
			{
				state2=0;
				SW_CMD[3]=0x02;//ÖÕ¶ËµØÖ·Îª2
				SW_CMD[6]=0x00;//off
				SW_CMD[7]=0x01;//XOR Ôİ²»¼ÆËã
				Usart_SendString(USART_DEBUG,(unsigned char*)SW_CMD,10); //·¢ËÍ¿ØÖÆÏÂĞĞÃüÁî
				LED1_OFF;   //LED0¹Ø
			}
			LCD_ShowxNum(240,550,state2,1,24,0X80);
		}
		else if(strstr((char *)req, "venti"))
		{
			if(num == 1)
			{
				state3=1;
				SW_CMD[3]=0x03;//ÖÕ¶ËµØÖ·Îª3
				SW_CMD[6]=0x01;//on
				SW_CMD[7]=0x01;//XOR Ôİ²»¼ÆËã
				Usart_SendString(USART_DEBUG,(unsigned char*)SW_CMD,10); //·¢ËÍ¿ØÖÆÏÂĞĞÃüÁî
				LED1_ON;    //LED1¿ª
			}
			else if(num == 0)
			{
				state3=0;
				SW_CMD[3]=0x03;//ÖÕ¶ËµØÖ·Îª3
				SW_CMD[6]=0x00;//off
				SW_CMD[7]=0x00;//XOR Ôİ²»¼ÆËã
				Usart_SendString(USART_DEBUG,(unsigned char*)SW_CMD,10); //·¢ËÍ¿ØÖÆÏÂĞĞÃüÁî
				LED1_OFF;   //LED1¹Ø
			}
			LCD_ShowxNum(240,575,state3,1,24,0X80);
		}
		else if(strstr((char *)req, "spray"))
		{
			if(num == 1)
			{ 
				state4=1;
				SW_CMD[3]=0x04;//ÖÕ¶ËµØÖ·Îª4
				SW_CMD[6]=0x01;//on
				SW_CMD[7]=0x06;//XOR Ôİ²»¼ÆËã
				Usart_SendString(USART_DEBUG,(unsigned char*)SW_CMD,10); //·¢ËÍ¿ØÖÆÏÂĞĞÃüÁî
				LED1_ON;    //LED1¿ª
			}
			else if(num == 0)
			{
				state4=0;
				SW_CMD[3]=0x04;//ÖÕ¶ËµØÖ·Îª4
				SW_CMD[6]=0x00;//off
				SW_CMD[7]=0x07;//XOR Ôİ²»¼ÆËã
				Usart_SendString(USART_DEBUG,(unsigned char*)SW_CMD,10); //·¢ËÍ¿ØÖÆÏÂĞĞÃüÁî
				LED1_OFF;   //LED1¹Ø
			}
			LCD_ShowxNum(240,600,state4,1,24,0X80);
		}
	}
	
	if(type == PUSHDATA && result == 0)					//Èç¹ûÊÇpushdata ÇÒ ½â°ü³É¹¦
	{
		EDP_FreeBuffer(cmdid_devid);					//ÊÍ·ÅÄÚ´æ
		EDP_FreeBuffer(req);
	}
	else if(type == CMDREQ && result == 0)				//Èç¹ûÊÇÃüÁî°ü ÇÒ ½â°ü³É¹¦
	{
		EDP_FreeBuffer(cmdid_devid);					//ÊÍ·ÅÄÚ´æ
		EDP_FreeBuffer(req);
														//»Ø¸´ÃüÁî
		//NET_DEVICE_SendData(edpPacket._data, edpPacket._len);//ÉÏ´«Æ½Ì¨
		NET_DEVICE_AddDataSendList(edpPacket._data, edpPacket._len);//¼ÓÈëÁ´±í
		EDP_DeleteBuffer(&edpPacket);					//É¾°ü
		oneNetInfo.sendData = SEND_TYPE_DATA;			//±ê¼ÇÊı¾İ·´À¡
	}

}
