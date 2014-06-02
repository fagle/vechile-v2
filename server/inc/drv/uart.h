////////////////////////////////////////////////////////////////////////////////////////////
//
//    
//    File: uart.h
//
//    Copyright (C): 2007-2009 Searen Network Software Ltd.
//
//    [ This source code is the sole property of Searen Network Software Ltd.  ]
//    [ All rights reserved.  No part of this source code may be reproduced in ]
//    [ any form or by any electronic or mechanical means, including informa-  ]
//    [ tion storage and retrieval system, without the prior written permission]
//    [ of Searen Network Software Ltd.                                        ]
//    [                                                                        ]
//    [   For use by authorized Searen Network Software Ltd. employees only.   ]
//
//    Description:   This programe will can control, handle and maintain Nand.
//                   The Nand Flash functions include read, write and erase flash.
//                   It also supports other functions such as read flash ID, GPIO 
//                   ports input and output setup, flash block management and flash 
//                   read or write ready signal up and dowm. 
//
//
//  AUTHOR: Cao Peng.
//  DATE: Oct. 08, 2009
//  MODIFIED: 
//  DATE: 
//
/////////////////////////////////////////////////////////////////////////////////////////////
//
#ifndef __USER_UART_H__
#define __USER_UART_H__

#include "stm32f10x_lib.h"

#ifdef  __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////
#define UARTBUFLEN   0x10

///////////////////////////////////////////////////////////////////////
// the definitaion of structure and union
//
typedef struct
{
    char   RingBufCtr;		    // Number of characters in the ring buffer           
    char * RingBufInPtr;	    // Pointer to where next character will be inserted   
    char * RingBufOutPtr;	    // Pointer from where next character will be extracted
    char   Buffer[UARTBUFLEN];  // Storage for key scan code
} uart_info_t, *puart_info_t;

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void UART_Configuration ( void )
//* 功能        : 打印十六进制数据 
//* 输入参数    : char ch2, ulong l
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void UART_Configuration ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void UART3_GPIO_Configuration ( void )
//* 功能        : 打印十六进制数据 
//* 输入参数    : char ch2, ulong l
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void UART3_GPIO_Configuration ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u8   Uart1_PutChar(u8 ch)
//* 功能        : 打印十六进制数据 
//* 输入参数    : char ch2, ulong l
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u8 Uart3_PutChar ( u8 ch );
u8 Uart3_GetChar ( );
u8 Uart3_GetCharOnce ( );
u16 Uart3_GetPage ( );
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void Uart1_PutString ( u8 * buf )
//* 功能        : 打印十六进制数据 
//* 输入参数    : char ch2, ulong l
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void Uart3_PutString ( u8 * buf );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void print0nx ( char ch2, s32 l )
//* 功能        : 打印十六进制数据 
//* 输入参数    : char ch2, ulong l
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void print0nx ( char ch2, s32 l );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void printd ( s16 l )
//* 功能        : 打印十进制数据 
//* 输入参数    : ulong l
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void printd ( s16 l );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void printd32 ( int32 l )
//* 功能        : 打印十进制数据 
//* 输入参数    : ulong l
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void printd32 ( s32 l );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_printf ( const char * format, ... )
//* 功能        : 格式化输出数据
//* 输入参数    : const char * format, ...
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_printf ( const char * format, ... );

////////////////////////////////////////////////////////////////////////
#ifdef  __cplusplus
}
#endif

////////////////////////////////////////////////////////////////////////
#endif // __USER_UART_H__ 
