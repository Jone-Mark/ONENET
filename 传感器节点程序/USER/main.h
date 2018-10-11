#ifndef __MAIN_H
#define __MAIN_H	

#include "stm32f10x.h"
#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"	 
#include "dht11.h" 	
#include "usart2.h"
#include "bh1750.h"
#include "timer.h"
#include "misc.h"
#include "adc.h"

u16 adcx;
float temp;
u16 led0pwmval=0;  
u8 dir=1;	
uint8_t i=0;
u8 temperature;  	    
u8 humidity;
u16 value_co2;
u8 Read_CO2[9]= {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
u8 string[]="";  

#define USART1_Send printf
#define USART2_Send u2_printf

void get_temp_humi_data(void);
void get_co2_data(void);
void get_light_data(void);
void get_soilhumi_data(void);
void pwm_on(void);

#endif

