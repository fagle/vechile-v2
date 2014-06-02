#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "config.h"
#include "micro.h"
#include "ember-types.h"
#include "halSerial.h"
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include <stdarg.h>
#endif // DOXYGEN_SHOULD_SKIP_THIS

///////////////////////////////////////////////////////////////////////////////////
//
#define UARTBUFSIZE       (0x40)
#define UARTTIMEOUT       (0x30000)

#define MAXUSART          (0x01)

#define RX_COM            (0x01)          //port1\USART1 (USART2) RX_COM1 
#define TX_COM            (0x02)          //port2\USART1
#define RX_GPRS           (0x03)
#define TX_GPRS           (0x04)
#define RX_COM_INX        (0x00)
#define TX_COM_INX        (0x01)
#define RX_GPRS_INX       (0x02)
#define TX_GPRS_INX       (0x03)

#ifdef DEBUG //all debug message
    #define Debug sea_printf
#else
    #define Debug(...) \
        do{} while(0)
#endif

#ifndef USE_RS485
#define SHOW_ESSENTIAL_MSG
#endif

#ifdef SHOW_ESSENTIAL_MSG
    #define DBG(...)    if (sys_info.ctrl.release) while(0); else\
        uartfrm.print(&uartfrm,__VA_ARGS__) 
#else
    #define DBG(...) \
        do{} while(0)
#endif

////////////////////////////////////////////////////////////////////////////////
//
typedef struct _uart_t
{
    u8   enable;
    u8   in, out;
    u8   buffer[UARTBUFSIZE];
    void (*put) ( struct _uart_t * uart, u8 ch );
    u8   (*get) ( struct _uart_t * uart );
    u8   (*ept) ( struct _uart_t * uart );
    void (*clr) ( struct _uart_t * uart );
} uart_t, *puart_t;

/////////////////////////////////////////////////////////////////////////////////////////////
//
/*******************************************************************************
* Function Name  : u8 sea_readbyte ( void )
* Description    : get ch from serial_info ring.
* Input          : None
* Output         : key value
* Return         : None
*******************************************************************************/
u8 sea_readbyte ( void );

/*******************************************************************************
* Function Name  : bool sea_isempty ( void ) 
* Description    : is empty of ring.
* Input          : None
* Output         : boolean
* Return         : None
*******************************************************************************/
bool sea_isempty ( void );

/*******************************************************************************
* Function Name  : void sea_putbyte ( u8  ch )
* Description    : Inserts ch in ring.
* Input          : u8 ch
* Output         : None
* Return         : None
*******************************************************************************/
void sea_putbyte ( u8  ch );

/*******************************************************************************
* Function Name  : void reset_serialinfo ( void  )
* Description    : reset serial_info body 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void reset_serialinfo ( void  );

/*******************************************************************************
* Function Name  : void sea_uartsendbyte ( u8 ch )
* Description    : send a byte to uart 
* Input          : u8 ch 
* Output         : None
* Return         : None
*******************************************************************************/
u8 sea_uartsendbyte ( u8 ch );

/*******************************************************************************
* Function Name  : void sea_uartsendstring ( const u8 * str )
* Description    : print a string to the uart
* Input          : strΪ�������ݵĵ�ַ
* Output         : None
* Return         : None
*******************************************************************************/
void sea_uartsendstring ( const u8 * str );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void print_0nx ( char ch2, s32 l )
//* ����        : ��ӡʮ���������� 8
//* �������    : char ch2, s32 l
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void print0nx ( char ch2, s32 l );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void printd32 ( int32 l )
//* ����        : ��ӡʮ�������� 
//* �������    : int32 l
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void printd32 ( s32 l );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void print_d ( int16 l )
//* ����        : ��ӡʮ�������� 
//* �������    : int16 l
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void printd ( s16 l );

////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_printf ( const char * format, ... )
//* ����        : ��ʽ���������
//* �������    : const char * format, ...
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_printf ( const char * format, ... );

////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : u8 SerialInit ( int8u port, u32 rate, u8 parity, int8u stopBits )
//* ����        : initailize serial
//* �������    : int8u port, u32 rate, u8 parity, int8u stopBits
//* �������    : status
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
u8 SerialInit ( int8u port, u32 rate, u8 parity, int8u stopBits );

////////////////////////////////////////////////////////////////////////////////////////////
//
extern uart_t  serial_info;

////////////////////////////////////////////////////////////////////////////////////////////
#endif // __SERIAL_H__

