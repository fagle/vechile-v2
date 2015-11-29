// *******************************************************************
//  serial.c
// *******************************************************************
#include <stdarg.h>
#include "config.h"
#include "ember-types.h"
#include "error.h"
#include "hal.h"
#include "serial.h"
#include "frame.h"
#include "network.h"

////////////////////////////////////////////////////////////////////////
// constants and globals
extern uart_t  serial_info; 

////////////////////////////////////////////////////////////////////////
// public functions
/*******************************************************************************
* Function Name  : u8 sea_uartgetbyte ( puart_t uart )
* Description    : get ch from uart_t ring.
* Input          : None
* Output         : key value
* Return         : None
*******************************************************************************/
static u8 sea_uartgetbyte ( puart_t uart )  
{
    u8  ch;
   
    if (uart == NULL)
        return 0x00;
    
    if(uart->out != uart->in)    
    {
        ch = uart->buffer[uart->out ++];
        if (uart->out >= UARTBUFSIZE)
            uart->out = 0x00;
        return ch;
    }
    return 0x00;
}

/*******************************************************************************
* Function Name  : bool sea_uartisempty ( puart_t uart ) 
* Description    : is empty of ring.
* Input          : puart_t uart
* Output         : true/flase
* Return         : None
*******************************************************************************/
static u8  sea_uartisempty ( puart_t uart )  
{
    if (uart == NULL)
        return 0x01;
    //return (uart->cnt == 0x00);
    return (uart->out == uart->in);
}

/*******************************************************************************
* Function Name  : void sea_uartputbyte ( puart_t uart, u8 ch )
* Description    : Inserts ch in ring.
* Input          : puart_t uart, u8 ch
* Output         : None
* Return         : None
*******************************************************************************/
static void sea_uartputbyte ( puart_t uart, u8 ch )
{
    if (uart == NULL)
        return;
        
    uart->buffer[uart->in ++] = ch;
    if (uart->in >= UARTBUFSIZE)
        uart->in = 0x00;
}

/*******************************************************************************
* Function Name  : void sea_uartputbyte ( puart_t uart )
* Description    : Inserts ch in ring.
* Input          : puart_t uart
* Output         : None
* Return         : None
*******************************************************************************/
static void sea_uartclear ( puart_t uart )
{
    if (uart == NULL)
        return;
    
    uart->out = uart->in = 0x00;
}

#if 0
/*******************************************************************************
* Function Name  : u8 sea_readbyte ( void )
* Description    : get ch from serial_info ring.
* Input          : None
* Output         : key value
* Return         : None
*******************************************************************************/
u8 sea_readbyte ( void )  
{
    return serial_info.get(&serial_info);
}

/*******************************************************************************
* Function Name  : bool sea_isempty ( void ) 
* Description    : is empty of ring.
* Input          : None
* Output         : boolean
* Return         : None
*******************************************************************************/
bool sea_isempty ( void )  
{
    return (bool)serial_info.ept(&serial_info);
}
#endif

////////////////////////////////////////////////////////////////////////////////
//轮询模式
/*******************************************************************************
* Function Name  : void sea_uartsendbyte ( u8 ch )
* Description    : send a byte to uart 
* Input          : u8 ch 
* Output         : None
* Return         : None
*******************************************************************************/
u8 sea_uartsendbyte ( u8 ch )
{
    vu32 timeout = UARTTIMEOUT;
    
    while ((SC1_UARTSTAT & SC_UARTTXFREE) != SC_UARTTXFREE && timeout --)  ;   
    if (timeout)
    {
        SC1_DATA = ch;
        return 0x01;
    }
    return 0x00;
}

/*******************************************************************************
* Function Name  : void sea_uartsendstring ( const u8 * str )
* Description    : print a string to the uart
* Input          : str为发送数据的地址
* Output         : None
* Return         : None
*******************************************************************************/
void sea_uartsendstring ( const u8 * str )
{
    u8 * ptr = (u8 *)str;
    
    while (*ptr)
        sea_uartsendbyte(*ptr ++);
}

/*******************************************************************************
* Function Name  : void UartSendByte(u8 ch )
* Description    : send a byte to uart 
* Input          : USART_TypeDef * uart, u8 ch 
* Output         : None
* Return         : None
*******************************************************************************/
void UartSendByte ( u8 ch )
{
    sea_uartsendbyte(ch);
}

/*******************************************************************************
* Function Name  : void UartSendString ( u8 * str )
* Description    : print a string to the uart
* Input          : str为发送数据的地址
* Output         : None
* Return         : None
*******************************************************************************/
void UartSendString (const u8 * str )
{
    sea_uartsendstring(str);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void print_0nx ( char ch2, s32 l )
//* 功能        : 打印十六进制数据 8
//* 输入参数    : char ch2, s32 l
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void print0nx ( char ch2, s32 l )
{
    char ch;
    int i;

    ch2 = ch2 - 0x30;
    for (i = ch2 - 0x01; i >= 0x00; i --) 
    {
	ch = (l >> (i * 4)) & 0x0f;
	if (ch < 10)
	    UartSendByte(ch + 0x30);
	else
	    UartSendByte(ch - 10 + 'a');
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void printd32 ( int32 l )
//* 功能        : 打印十进制数据 
//* 输入参数    : int32 l
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void printd32 ( s32 l )
{
    s32 prod, t;
    int    flag;
    char   ch;
    int      i, j;

    if (l == 0x00)
    {
	UartSendByte('0');
	return;
    }
    else if (l < 0x00)
    {
	UartSendByte('-');
	l = -l;
    }

    for (flag = 0x00, i = 15; i >= 0x00; i --) 
    {
	prod = 0x01;
	t = l;
	for (j = 0x00; j < i; j ++) 
	{
	    prod = prod * 10;
	    t = t / 10;
	}
	ch = t;
	l -= prod * t;
	if (ch == 0x00 && flag == 0x00)
	    continue;

	if (ch < 10)
	    UartSendByte(ch + 0x30);
	else
	    UartSendByte('?');

	flag = 0x01;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void print_d ( int16 l )
//* 功能        : 打印十进制数据 
//* 输入参数    : int16 l
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void printd ( s16 l )
{
    u16 prod, t;
    int    flag;
    char   ch;
    int    i, j;

    if (l == 0x00)
    {
        UartSendByte('0');
	return;
    }
    else if (l < 0x00)
    {
	UartSendByte( '-');
	l *= -1;
    }

    for (flag = 0x00, i = 0x08; i >= 0x00; i --) 
    {
	prod = 0x01;
	t = l;
	for (j = 0x00; j < i; j ++) 
	{
	    prod = prod * 10;
	    t = t / 10;
	}
	ch = t;
	l -= prod * t;

	if (ch == 0x00 && flag == 0x00)
	    continue;

	if (ch < 10)
	    UartSendByte(ch + 0x30);
	else
	    UartSendByte('?');

	flag = 0x01;
    }
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_printf ( const char * format, ... )
//* 功能        : 格式化输出数据
//* 输入参数    : const char * format, ...
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_printf ( const char * format, ... )
{ 
#ifdef FRPRINTBUF
    va_list v_list;
    u8   *  ptr;
    
    if (!serial_info.enable)
        return;
    
    memset(uartfrm.buf, 0x00, FRPRINTBUF);
    va_start(v_list, format);     
    vsprintf((char *)uartfrm.buf, format, v_list); 
    va_end(v_list);
    
    ptr = uartfrm.buf;
    while (*ptr)
    {
        if (*ptr == '\n')
            sea_uartsendbyte('\r');
        sea_uartsendbyte(*ptr);
        ptr ++;
    }
#else  
    va_list ap;
    u32     arg;
    char *  str;
    char    ch1, ch2;

    if (!serial_info.enable)
        return;
    
    va_start(ap, format);
    while (*format) 
    {
	if (*format == '%') 
	{
	    format ++;
	    ch1 = *format ++;//ch1 = x表示16进制
	    switch (ch1)
	    {
		case 's': 
		    str = va_arg(ap, char *);
		    UartSendString((u8 *)str);
		    break; 
	        case 'c': 
		    ch2 = va_arg(ap, char);
		    UartSendByte(ch2);
		    break; 
		case 'd': 
		    arg = va_arg(ap, s16);
		    printd(arg);
		    break;
		case 'l':
		    arg = va_arg(ap, s32);
		    printd32(arg);
	            break;
                default: 
		    if (ch1 == 'x')
		        ch2 = 0x34;          // print 4 char for data 
		    else 
		    {
		        ch2 = *format ++;
		        if (ch1 == 0x30)
		            format ++;
		        else
		            ch2 = ch1;
	            }
		    ch2 = ch2 > 0x38 ? 0x38 : ch2;
		    arg = va_arg(ap, s32); 
		    print0nx(ch2, arg);
		    break;
	    }
	} 
	else
	{
	    if (*format == '\n')
	        UartSendByte( '\r');
	    UartSendByte(*format ++);
	}
    }
    va_end(ap);   
#endif    
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u8 SerialInit ( int8u port, u32 rate, u8 parity, u8 stopBits )
//* 功能        : serial初始化
//* 输入参数    : int8u port, u32 rate, u8 parity, u8 stopBits
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : port无必要！
//*------------------------------------------------*/
u8 SerialInit ( int8u port, u32 rate, u8 parity, u8 stopBits )
{  
    if (port > MAXUSART)
        return 0x00;
    
    sea_memset(&serial_info, 0x00, sizeof(uart_t));
    serial_info.ept    = sea_uartisempty;
    serial_info.get    = sea_uartgetbyte;
    serial_info.put    = sea_uartputbyte;
    serial_info.clr    = sea_uartclear;
    
    return halInternalUartInit(port, rate, parity, stopBits);
}
//////////////////////////////////////////////////////////////////////////////////////////
