#ifndef __IIC_H__
#define __IIC_H__

#include "stm32f10x.h" 
#include "systicker.h"
#include "config.h"

#define RTC_SCL_PIN   GPIO_Pin_6           // GPIO_Pin_6
#define RTC_SDA_PIN   GPIO_Pin_7           // GPIO_Pin_7

#define SCL_H         GPIOB->BSRR = RTC_SCL_PIN 
#define SCL_L         GPIOB->BRR  = RTC_SCL_PIN  
    
#define SDA_H         GPIOB->BSRR = RTC_SDA_PIN 
#define SDA_L         GPIOB->BRR  = RTC_SDA_PIN 

#define SCL_read      GPIOB->IDR  & RTC_SCL_PIN 
#define SDA_read      GPIOB->IDR  & RTC_SDA_PIN 

#define PCF8365T_Address_Second  0x02
#define PCF8365T_Address_Minute  0x03
#define PCF8365T_Address_Hour    0x04
#define PCF8365T_Address_Day     0x05
#define PCF8365T_Address_Weekday 0x06
#define PCF8365T_Address_Month   0x07
#define PCF8365T_Address_Year    0x08

/************************************************
* 函数名      : void I2C_Configuration ( void )
* 功能        : 配置iic引脚
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
void I2C_Configuration ( void );

/************************************************
* 函数名      : void PCF8365T_SetDate ( u8 y , u8 mon , u8 d , u8 h , u8 min , u8 s ) 
* 功能        : set pcf8365 date and time
* 输入参数    : u8 y , u8 mon , u8 d , u8 h , u8 min , u8 s
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
void PCF8365T_SetDate ( u8 y , u8 mon , u8 d , u8 h , u8 min , u8 s ); 

/************************************************
* 函数名      : void PCF8365T_ShowDate ( void ) 
* 功能        : show pcf8365 date and time
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
void PCF8365T_ShowDate ( void );

/************************************************
* 函数名      : void Write_PCF8365T(u8 Address, u8 Value) 
* 功能        : ReceiveByte
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
void Write_PCF8365T ( u8 address, u8 value );

/************************************************
* 函数名      : void Read_PCF8365T(u8 Address, u8 Value) 
* 功能        : ReceiveByte
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
u8  Read_PCF8365T ( u8 AddRess );

/************************************************
* 函数名      : void PCF8365T_ReadTime ( data_time * time )
* 功能        : data_time * time
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
void PCF8365T_ReadDate ( systime_t * time );

/************************************************
* 函数名      : void PCF8365T_WriteDate ( data_time time )
* 功能        : write date time
* 输入参数    : data_time time
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
void PCF8365T_WriteDate ( systime_t time );

/*********************************************************
* 函数名      : void PCF8365T_SyschTime ( u8 * time )
* 功能        : 检测时间
* 输入参数    : 时间
* 输出参数    : 0x01:时间到  0x00:时间未到
* 修改记录    : 无
* 备注        : 无
*********************************************************/
void PCF8365T_SyschTime ( u8 * time );

#endif   // __IIC_H__
