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
//* ������      : void UART_Configuration ( void )
//* ����        : ��ӡʮ���������� 
//* �������    : char ch2, ulong l
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void UART_Configuration ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void UART3_GPIO_Configuration ( void )
//* ����        : ��ӡʮ���������� 
//* �������    : char ch2, ulong l
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void UART3_GPIO_Configuration ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : u8   Uart1_PutChar(u8 ch)
//* ����        : ��ӡʮ���������� 
//* �������    : char ch2, ulong l
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
u8 Uart3_PutChar ( u8 ch );
u8 Uart3_GetChar ( );
u8 Uart3_GetCharOnce ( );
u16 Uart3_GetPage ( );
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void Uart1_PutString ( u8 * buf )
//* ����        : ��ӡʮ���������� 
//* �������    : char ch2, ulong l
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void Uart3_PutString ( u8 * buf );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void print0nx ( char ch2, s32 l )
//* ����        : ��ӡʮ���������� 
//* �������    : char ch2, ulong l
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void print0nx ( char ch2, s32 l );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void printd ( s16 l )
//* ����        : ��ӡʮ�������� 
//* �������    : ulong l
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void printd ( s16 l );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void printd32 ( int32 l )
//* ����        : ��ӡʮ�������� 
//* �������    : ulong l
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void printd32 ( s32 l );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_printf ( const char * format, ... )
//* ����        : ��ʽ���������
//* �������    : const char * format, ...
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_printf ( const char * format, ... );

////////////////////////////////////////////////////////////////////////
#ifdef  __cplusplus
}
#endif

////////////////////////////////////////////////////////////////////////
#endif // __USER_UART_H__ 
