#ifndef __12864_H_
#define __12864_H_	
#include"sys.h"	 

//管脚定义，移植修改区
//**************************************************************************************************************************
#define  LCD12864_GPIO    GPIOB
#define  GPIOCLK          RCC_APB2Periph_GPIOB

#define  RS		     GPIO_Pin_0
#define  RW		     GPIO_Pin_1
#define  EN		     GPIO_Pin_3
#define  PSB	     GPIO_Pin_4
#define  RST	     GPIO_Pin_5
#define  Busy        GPIO_Pin_15
#define  LCD_GPIO_DAT   GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10 |GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15
#define  LCD_GPIO_CMD	GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5



#define    LCD_RS(x)   x ? GPIO_SetBits(LCD12864_GPIO,RS):  GPIO_ResetBits(LCD12864_GPIO,RS)
#define    LCD_RW(x)   x ? GPIO_SetBits(LCD12864_GPIO,RW):  GPIO_ResetBits(LCD12864_GPIO,RW)
#define    LCD_EN(x)   x ? GPIO_SetBits(LCD12864_GPIO,EN):  GPIO_ResetBits(LCD12864_GPIO,EN)
#define    LCD_PSB(x)  x ? GPIO_SetBits(LCD12864_GPIO,PSB): GPIO_ResetBits(LCD12864_GPIO,PSB)
#define    LCD_RST(x)  x ? GPIO_SetBits(LCD12864_GPIO,RST): GPIO_ResetBits(LCD12864_GPIO,RST)


#define  Text_Busy	         GPIO_ReadInputDataBit(LCD12864_GPIO  ,Busy)  //PBin(7)	  
#define  LCD_WriteData(x)     {LCD12864_GPIO->BSRR = x<<8 & 0xff00;LCD12864_GPIO->BRR = ((~x)<<8) & 0xff00;}  //高8位的数据
//#define  LCD_WriteData(x)   {LCD12864_GPIO->BSRR = x    & 0x00ff;LCD12864_GPIO->BRR = ((~x)<<8) & 0x00ff;}  //低8位的数据

void LCD12864_InitPort(void);	  //硬件端口初始化
void NOP(void);					 //延时函数
u8 LCD12864_busy(void);			 //检测忙状态
void LCD12684_Wcmd(u8 dat);		 //写指令
void LCD12684_Wdat(u8 dat);		 //写数据
void LCD12864_Init(void);		 //液晶初始化
void LCD12864_Clr(void);		 //	清屏
void LCD12864_Pos(u8 x,u8 y);	 //设置显示位置
void LCD12864_PhotoDis(u8 *bmp); //	显示图片
void LCD_ShowString(u8 x,u8 y,const u8 *p);	 //	显示字符串
//**********************************************************************************************************************
#endif	



