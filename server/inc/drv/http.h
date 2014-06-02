#ifndef __HTTP_H__
#define __HTTP_H__

//////////////////////////////////////////////////////////////////////////////////////////
//
#include "stm32f10x.h"
#include "config.h"

/************************************************
* 函数名      : void sea_sendgprsmessage ( const u8 * msg, const u8 * phone )
* 功能        : 发送短信 from EM310
* 输入参数    : const u8 * msg, const u8 * phone
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
void sea_sendgprsmessage ( const u8 * msg, const u8 * phone );

/************************************************
* 函数名      : void sea_sendopenmessage ( const u8 * phone )
* 功能        : 发送短信 from EM310
* 输入参数    : const u8 * phone
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
void sea_sendopenmessage ( const u8 * phone );

/************************************************
* 函数名      : void sea_sendclosemessage ( const u8 * phone )
* 功能        : 发送短信 from EM310
* 输入参数    : const u8 * phone
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
void sea_sendclosemessage ( const u8 * phone );

/************************************************
* 函数名      : u8 HTTP_IpsendGet ( void )
* 功能        : send get to web
* 输入参数    : 无
* 输出参数    : ture / flase
* 修改记录    : 无
* 备注        : 无
*************************************************/
u8 HTTP_IpsendGet ( void );

/************************************************
* 函数名      : u8 HTTP_IpsendPostSize ( void )
* 功能        : send post to web
* 输入参数    : 无
* 输出参数    : ture / flase
* 修改记录    : 无
* 备注        : 无
*************************************************/
void HTTP_IpsendPost ( void );

/************************************************
* 函数名      : u8 HTTP_IpsendPost ( u16 len )
* 功能        : send post to web
* 输入参数    : u16 len
* 输出参数    : ture / flase
* 修改记录    : 无
* 备注        : 无
*************************************************/
u8 HTTP_IpsendPostSize ( u16 len );

/************************************************
* 函数名      : u8 HTTP_ReportFrame ( u8 id, u8 size, u16 road_id, u8 * body )
* 功能        : 向gprs发送帧
* 输入参数    : u8 id, u8 size, u16 road_id, u8 * body
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/
u8 HTTP_ReportFrame ( u8 id, u8 size, u16 road, u8 * body );

//////////////////////////////////////////////////////////////////////////////////////////
//
#endif
