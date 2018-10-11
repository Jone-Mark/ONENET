#include "stm32f10x.h"
#include"12864.h"
#include"delay.h"	
/************************************************************************************************
 //FILE:液晶12864驱动程序
 //VERS:1.0
 //AUTHOR:福建师范大学林木泉
 //DATE:2012/07/11
************************************************************************************************/
/************************************************************************************************
@f_name: void LCD12864_InitPort(void)
@brief:	 初始化硬件端口配置
@param:	 None
@return: None
************************************************************************************************/
void LCD12864_InitPort(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;	//定义结构体		
	
	RCC_APB2PeriphClockCmd(GPIOCLK|RCC_APB2Periph_AFIO, ENABLE);  //使能功能复用IO时钟，不开启复用时钟不能显示
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable,ENABLE);    //把调试设置普通IO口

	GPIO_InitStructure.GPIO_Pin  = LCD_GPIO_DAT;		//数据口配置成开漏输出模式，此模式下读输入寄存器的值得到IO口状态
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;   //开漏输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_Init(LCD12864_GPIO , &GPIO_InitStructure);    //IO口初始化函数（使能上述配置）

	GPIO_InitStructure.GPIO_Pin  = LCD_GPIO_CMD;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //推挽输出   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LCD12864_GPIO , &GPIO_InitStructure); 

	GPIO_Init(LCD12864_GPIO , &GPIO_InitStructure);	  //初始化IO口配置
	GPIO_Write(LCD12864_GPIO ,0xffff);	  
}
/************************************************************************************************
@f_name: void NOP(void)
@brief:	 延时函数
@param:	 None
@return: None
************************************************************************************************/
void NOP(void)
{ u8 i;	for(i=0; i<100; i++); } 
/************************************************************************************************
@f_name: u8 LCD12864_busy(void)
@brief:	 检测忙状态
@param:	 None
@return: None
************************************************************************************************/
u8 LCD12864_busy(void)
{
	u8 x;
	LCD_RS(0); 
	LCD_RW(1);  
	LCD_EN(1); 
	NOP();
	NOP(); 
	x=Text_Busy;
	LCD_EN(0); 
	return x;
}
/************************************************************************************************
@f_name: void LCD12684_Wcmd(u8 dat)
@brief:	 写指令
@param:	 u8 dat  输入指令
@return: None
************************************************************************************************/
void LCD12684_Wcmd(u8 dat)
{
	while(LCD12864_busy()); //忙检测
	LCD_RS(0);      
	LCD_RW(0);
	LCD_EN(0);
	NOP();
	NOP();
	LCD_WriteData(dat);	  
	NOP();
	NOP();
	LCD_EN(1);
	NOP();
	NOP();
	LCD_EN(0);
}
/************************************************************************************************
@f_name: void LCD12684_Wcmd(u8 dat)
@brief:	 写数据
@param:	 u8 dat 输入数据
@return: None
************************************************************************************************/
void LCD12684_Wdat(u8 dat)
{
	while(LCD12864_busy()); //忙检测
	LCD_RS(1);      
	LCD_RW(0);
	LCD_EN(0);
	NOP();
	NOP();
	LCD_WriteData(dat);	 
	NOP();
	NOP();
	LCD_EN(1);
	NOP();
	NOP();
	LCD_EN(0);
}
/************************************************************************************************
@f_name: void LCD12864_Init(void)
@brief:	 液晶初始化
@param:	 None
@return: None
************************************************************************************************/
void LCD12864_Init(void)
{	
		LCD_PSB(1);   //并口方式    
		LCD_RST(0);   //液晶复位	低电平有效
    delay_ms(3);                  
    LCD_RST(1);  	//置高电平等待复位
    delay_ms(3);

		LCD12684_Wcmd(0x34);    //扩充指令操作
    delay_ms(5);
    LCD12684_Wcmd(0x30);    //基本指令操作
    delay_ms(5);
    LCD12684_Wcmd(0x0c);   //显示开，关光标
    delay_ms(5);
    LCD12684_Wcmd(0x01);   //清除LCD的显示内容
    delay_ms(5); 
}
/************************************************************************************************
@f_name: void LCD12864_Clr(void)
@brief:	 清屏
@param:	 None
@return: None
************************************************************************************************/
void LCD12864_Clr(void)
{
	LCD12684_Wcmd(0x34);   //扩充指令操作	“绘图”
    delay_ms(5);
    LCD12684_Wcmd(0x30);    //基本指令操作
    delay_ms(5);
	LCD12684_Wcmd(0x01);   //清屏
    delay_ms(5);
} 
/************************************************************************************************
@f_name: void LCD12864_Pos(u8 x,u8 y)
@brief:	 设置显示位置
@param:	 u8 x：X轴    u8 y:Y轴  
@return: None
************************************************************************************************/
void LCD12864_Pos(u8 x,u8 y)
{
	u8  pos;
	if (x==1)  		 {x=0x80;} 	 
	else if (x==2)	 {x=0x90;}	
	else if (x==3)	 {x=0x88;}	 
	else if (x==4)   {x=0x98;}	 
	else x=0x80;
	pos = x+y ; 	
	LCD12684_Wcmd(pos);  //显示地址  
}
/************************************************************************************************
@f_name: void LCD12864_PhotoDis(u8 *bmp)
@brief:	 显示图形
@param:	 u8 *bmp 图形数组
@return: None
************************************************************************************************/
void LCD12864_PhotoDis(u8 *bmp)
{
	u8 i,j;
	LCD12684_Wcmd(0x34); //关闭图形显示
	
	for(i=0;i<32;i++)
	{
		LCD12684_Wcmd(0x80+i);  //先写入水平坐标值
		LCD12684_Wcmd(0x80);      //写入垂直坐标值
		for(j=0;j<16;j++)   		  //再写入两个8位元的数据    
		LCD12684_Wdat(*bmp++);     
		delay_ms(1);
	}
	
	for(i=0;i<32;i++)
	{ 
		LCD12684_Wcmd(0x80+i);
		LCD12684_Wcmd(0x88);
		for(j=0;j<16;j++)         
		LCD12684_Wdat(*bmp++);    
		delay_ms(1);
	}
	LCD12684_Wcmd(0x36);       //写完数据,开图形显示 	
}
/************************************************************************************************
@f_name: void LCD_ShowString(u8 x,u8 y,const u8 *p)
@brief:	 显示字符串
@param:	 u8 *bmp 图形数组
@return: None
************************************************************************************************/
void LCD_ShowString(u8 x,u8 y,const u8 *p)
{
	u8 temp;
	if(x>9) {x=1;}
	if(y>9) {y=0;}
	LCD12864_Pos(x,y);
	temp=*p;
	while(temp!='\0')
	{
		LCD12684_Wdat(temp);
		temp=*(++p);
	} 
}
