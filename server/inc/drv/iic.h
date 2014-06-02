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
* ������      : void I2C_Configuration ( void )
* ����        : ����iic����
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void I2C_Configuration ( void );

/************************************************
* ������      : void PCF8365T_SetDate ( u8 y , u8 mon , u8 d , u8 h , u8 min , u8 s ) 
* ����        : set pcf8365 date and time
* �������    : u8 y , u8 mon , u8 d , u8 h , u8 min , u8 s
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void PCF8365T_SetDate ( u8 y , u8 mon , u8 d , u8 h , u8 min , u8 s ); 

/************************************************
* ������      : void PCF8365T_ShowDate ( void ) 
* ����        : show pcf8365 date and time
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void PCF8365T_ShowDate ( void );

/************************************************
* ������      : void Write_PCF8365T(u8 Address, u8 Value) 
* ����        : ReceiveByte
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void Write_PCF8365T ( u8 address, u8 value );

/************************************************
* ������      : void Read_PCF8365T(u8 Address, u8 Value) 
* ����        : ReceiveByte
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
u8  Read_PCF8365T ( u8 AddRess );

/************************************************
* ������      : void PCF8365T_ReadTime ( data_time * time )
* ����        : data_time * time
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void PCF8365T_ReadDate ( systime_t * time );

/************************************************
* ������      : void PCF8365T_WriteDate ( data_time time )
* ����        : write date time
* �������    : data_time time
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void PCF8365T_WriteDate ( systime_t time );

/*********************************************************
* ������      : void PCF8365T_SyschTime ( u8 * time )
* ����        : ���ʱ��
* �������    : ʱ��
* �������    : 0x01:ʱ�䵽  0x00:ʱ��δ��
* �޸ļ�¼    : ��
* ��ע        : ��
*********************************************************/
void PCF8365T_SyschTime ( u8 * time );

#endif   // __IIC_H__
