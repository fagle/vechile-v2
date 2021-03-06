
#ifndef __IOMODE_H__
#define __IOMODE_H__

#include "stm32f10x.h"

/************************************************
* 函数名      : u8 Char_To_Num( u8 ch )
* 功能        : 将字符转换成对应的数字
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
u8 Char_To_Num( u8 ch );

/************************************************
* 函数名      : u8 Num_To_Char( u8 ch )
* 功能        : 将字符转换成对应的数字
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
u8 Num_To_Char( u8 ch );

/***********************************************
* 函数名      : void Hex_To_TwoChar( u8 high , u8 low)
* 功能        : 采用iomode命令对十六进制和字符进行转换后，协调器通过EM310向服务
                器上报数据时必须将十六进制数据用字符表示，例如，发送指令“68 42 
                04 00 46 43”，在指令中at%ipsend="684204004643"，每个数字都是一
                个字符，要发送0x68，需要用‘6’和‘8’两个字符表示
* 输入参数    : 高四位和低四位
* 输出参数    : 十六进制数据
* 修改记录    : 无
* 备注        : 无
************************************************/
void Hex_To_TwoChar( u8 ch , u8 * high , u8 * low);

/************************************************
* 函数名      : void Num_To_String( u16 size, u8 * str )
* 功能        : 将数字转换成一串字符
* 输入参数    : u16 size, u8 * str
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
void Num_To_String ( u16 size, u8 * str );

/************************************************
* 函数名      : u8 GPRS_IpsendByte( u8 ch )
* 功能        : 使用ipsend发送一字节数据
* 输入参数    : 字节
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
u8 GPRS_IpsendByte( u8 ch );

/************************************************
* 函数名      : u8 u8 GPRS_Ipsend2Byte ( u8 ch )
* 功能        : 使用ipsend发送2字节数据
* 输入参数    : 字节
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
u8 GPRS_Ipsend2Byte ( u8 ch );

/************************************************
* 函数名      : u8 GPRS_IpsendNumber ( u16 size )
* 功能        : 使用ipsend发送一字节数据
* 输入参数    : u16 size
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
u8 GPRS_IpsendNumber ( u16 size );

/************************************************
* 函数名      : u8 GPRS_IpsendString ( u8 * pointer )
* 功能        : 使用ipsend发送数据
* 输入参数    : u8 * pointer
* 输出参数    : ture / flase
* 修改记录    : 无
* 备注        : 无
*************************************************/
u8 GPRS_IpsendString ( u8 * pointer );

/************************************************
* 函数名      : u8 GPRS_Ipsend( u8 * pointer )
* 功能        : 发送一组数据
* 输入参数    : 字符串
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
u8 GPRS_Ipsend( u8 * pointer );

/************************************************
* 函数名      : u8 GPRS_IpwaitAnswer ( void )
* 功能        : waiting for response
* 输入参数    : 无
* 输出参数    : true / flase
* 修改记录    : 无
* 备注        : 无
*************************************************/
u8 GPRS_IpwaitAnswer ( void );

/************************************************
* 函数名      : u8 GPRS_IpsendOpen ( void )
* 功能        : send open to web
* 输入参数    : 无
* 输出参数    : ture / flase
* 修改记录    : 无
* 备注        : 无
*************************************************/
u8 GPRS_IpsendOpen ( void );

#endif   // __IOMODE_H__


