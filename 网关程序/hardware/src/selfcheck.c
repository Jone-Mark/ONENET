/**************************************************************
*	�ļ����� 	selfcheck.c
*	���ߣ� 		�ż���
*	���ڣ� 		2016-11-23
*	�汾�� 		V1.0
*	˵���� 		LED��ʼ��������LED
*	�޸ļ�¼��	
************************************************************/

//Ӳ������
#include "selfcheck.h"
#include "usart.h"
#include "delay.h"

CHECK_INFO checkInfo = {DEV_ERR, DEV_ERR, DEV_ERR};

/*
************************************************************
*	�������ƣ�	Check_PowerOn
*	�������ܣ�	����豸���
*	��ڲ�����	��
*	���ز�����	��
*	˵����		IIC�豸���Զ�ȡ�Ĵ������鿴��Ӧ���
*				��Ҫ���sht20��adxl345��gy30��eeprom
************************************************************
*/
void Check_PowerOn(void)
{

//	unsigned char value = 0;
//	
//	//���SH20
//	I2C_ReadByte(0X40, 0XE7, &value);					//��ȡ�û��Ĵ���
//	if(value)
//	{
//		UsartPrintf(USART_DEBUG, "SHT20 :Ok\r\n");
//		checkInfo.SHT20_OK = DEV_OK;
//	}
//	else
//		UsartPrintf(USART_DEBUG, "SHT20 :Error\r\n");
//	DelayXms(1);
//	
//	//���EEPROM
//	if(!I2C_ReadByte(0x50, 255, &value))
//	{
//		UsartPrintf(USART_DEBUG, "EEPROM :Ok\r\n");
//		checkInfo.EEPROM_OK = DEV_OK;
//	}
//	else
//		UsartPrintf(USART_DEBUG, "EEPROM :Error\r\n");
//	DelayXms(1);

}
