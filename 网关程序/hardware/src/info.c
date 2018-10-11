/**************************************************************
*	�ļ����� 	info.c
*	���ߣ� 		�ż���
*	���ڣ� 		2017-02-23
*	�汾�� 		V1.1
*	˵���� 		V1.0��SSID��PSWD��DEVID��APIKEY���漰��ȡ��
*				    V1.1��ȡ����SSID��PSWD�ı���Ͷ�д���滻Ϊ������������wifi���͵������豸�����Զ����档

*	��Ҫ��   ֻ�е��ⲿ�洢������ʱ���Ŵ��ж�ȡ��Ϣ
*					 �������ڣ����ȡ�̻��ڴ��������Ϣ
*	�޸ļ�¼��	
************************************************************/

//Ӳ������
#include "info.h"
#include "delay.h"
#include "usart.h"

//Э��
#include "onenet.h"

//C��
#include <string.h>
#include <stdlib.h>

/*
************************************************************
*	�������ƣ�	Info_Check
*	�������ܣ�	�����Ϣ�Ƿ����
*	��ڲ�����	��
*	���ز�����	�����
*	˵����		  �ж�wifi��ssid��pswd�Ƿ����
*				0-ok	1-��ssid	2-��pswd
*				3-��devid	4-��apikey
************************************************************
*/
unsigned char Info_Check(void)
{
//	unsigned char rData = 0;
//	
//	AT24C02_ReadByte(DEVID_ADDRESS, &rData);	//��ȡ����ֵ
//	if(rData == 0 || rData >= 10)				//���Ϊ0�򳬳�
//		return 1;
//	
//	AT24C02_ReadByte(AKEY_ADDRESS, &rData);		//��ȡ����ֵ
//	if(rData == 0 || rData >= 30)				//���Ϊ0�򳬳�
//		return 2;
//        
	return 0;

}

/*
************************************************************
*	�������ƣ�	Info_WifiLen
*	�������ܣ�	��ȡ��Ϣ����
*	��ڲ�����	sp����Ҫ������Ϣ-��˵��
*	���ز�����	�����
*	˵����		��ȡ0-ssid����	1-pswd����	
*				2-devid����		3-apikey����
************************************************************/
unsigned char Info_WifiLen(unsigned char sp)
{
//	unsigned char len = 0; 
//  switch(sp)
//    {
//        case 0:AT24C02_ReadByte(DEVID_ADDRESS, &len);		//��ȡ����ֵ
//			         if(len == 0 || len >= 10)					//���Ϊ0�򳬳�
//				       return 1;
//        break;
//        
//        case 1:
//              AT24C02_ReadByte(AKEY_ADDRESS, &len);		//��ȡ����ֵ
//			        if(len == 0 || len >= 30)					//���Ϊ0�򳬳�
//				      return 1;
//        break;
//    }
//	
	return 0;
}

/*
************************************************************
*	�������ƣ�	Info_CountLen
*	�������ܣ�	�����ֶγ���
*	��ڲ�����	info����Ҫ�����ֶ�
*	���ز�����	�ֶγ���
*	˵����		  ���㴮1���������ֶγ���   ��"\r\n"��β
************************************************************
*/
unsigned char Info_CountLen(char *info)
{
	unsigned char len = 0;
	char *buf = strstr(info, ":");		//�ҵ�':'
	buf++;								//ƫ�Ƶ���һ���ֽڣ������ֶ���Ϣ��ʼ
	while(1)
	{
		if(*buf == '\r')				//ֱ��'\r'Ϊֹ
			return len;
		
		buf++;
		len++;
	}

}



