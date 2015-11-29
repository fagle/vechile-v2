#include <stdarg.h>
#include "config.h"
#include "ember-types.h"
#include "error.h"
#include "hal.h"
#include "serial.h"
#include "frame.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static pframe sea_getframe ( struct frm_t frm, pframe_t ptr )
//* 功能        : receiving frame 
//* 输入参数    : struct frm_t frm, pframe_t ptr
//* 输出参数    : pointer of frame_t
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static pframe_t sea_getframe ( struct frm_t * frm, pframe_t ptr )
{
    if (frm->out != frm->in)
    {
        memcpy(ptr, &frm->list[frm->out], sizeof(frame_t));   
        frm->out++;
        if(frm->out>=FRAMEBUFLEN)
            frm->out = 0x00;
        return ptr;
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void sea_printframe ( struct frm_t * frm, const char * str, ... )
//* 功能        : print frame infmation
//* 输入参数    : struct frm_t * frm, const char * str, ...
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void sea_printframe ( struct frm_t * frm, const char * str, ... )
{
    va_list v_list;
    u8   *  ptr;
    
    memset(frm->buf, 0x00, FRPRINTBUF);
    va_start(v_list, str);     
    vsprintf((char *)frm->buf, str, v_list); 
    va_end(v_list);
    
    ptr = frm->buf;
    while (*ptr)
    {
       frm->sendbyte(*ptr);
       ptr ++;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static bool sea_putstring ( struct frm_t frm, u8 cmd, u16 road, u8 * str, u8 len )
//* 功能        : data output
//* 输入参数    : struct frm_t frm, u8 cmd, u16 road, u8 * str, u8 len
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static u8 sea_putstring ( struct frm_t * frm, u8 cmd, u16 road, u8 * str, u8 len )
{
    u8   ch, sum;

    if (!len)   return 0x01;

    frm->sendbyte(PREFIX);    // PRT_DATA);
    frm->sendbyte(cmd);
    frm->sendbyte(len);      
    frm->sendbyte((road >> 0x08) & 0xff);
    frm->sendbyte(road & 0xff);
    sum = cmd + len + (road & 0xff) + ((road >> 0x08) & 0xff);  // len + cmd + (road >> 0x08) & 0xff + road & 0xff;
    for (ch = 0x00; ch < len; ch ++)
    {
	frm->sendbyte(str[ch]);
        sum += (str[ch] & 0xff);
    }
    frm->sendbyte(sum);
    return 0x01;
}


/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : s16 sea_putframe ( struct frm_t * frm, u8 cmd, u16 road, u8 len, u8 * str )
//* 功能        : string data output
//* 输入参数    : struct frm_t * frm, u8 cmd, u16 road, u8 len, u8 * str
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
s16 sea_putframe ( struct frm_t * frm, u8 cmd, u16 road, u8 len, u8 * str )
{
    u8 * ptr = (u8 *)str;
    u8   size;

    while (len)
    {
        size = len > FRDATASIZE ? FRDATASIZE : len;
        sea_putstring(frm, cmd, road, ptr, size);
        ptr += size;
        len -= size;
    }
    return PRT_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static s16 sea_putbyte ( struct frm_t frm, u8 cmd, u16 road, u8 value )
//* 功能        : send byte data output
//* 输入参数    : struct frm_t frm, u8 cmd, u16 road, u8 value
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static s16 sea_putframebyte ( struct frm_t * frm, u8 cmd, u16 road, u8 value )
{
    return sea_putframe(frm, cmd, road, 0x01, &value);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static s16 sea_putshort ( struct frm_t frm, u8 cmd, u16 road, u16 value )
//* 功能        : send short data output
//* 输入参数    : struct frm_t frm, u8 cmd, u16 road, u16 value
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static s16 sea_putframeshort ( struct frm_t * frm, u8 cmd, u16 road, u16 value )
{
    u8 str[0x02];

    str[0x00] = (value >> 0x08) & 0xff;
    str[0x01] = value & 0xff;

    return sea_putframe(frm, cmd, road, 0x02, str);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static s16 sea_putlong ( struct frm_t frm, u8 cmd, u16 road, u32 value )
//* 功能        : send long data output
//* 输入参数    : struct frm_t frm, u8 cmd, u16 road, u32 value
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static s16 sea_putframelong ( struct frm_t * frm, u8 cmd, u16 road, u32 value )
{
    u8 str[0x04];

    str[0x00] = (value >> 0x18) & 0xff;
    str[0x01] = (value >> 0x10) & 0xff;
    str[0x02] = (value >> 0x08) & 0xff;
    str[0x03] = value & 0xff;
    
    return sea_putframe(frm, cmd, road, 0x04, str);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void sea_recvbyte ( struct frm_t * frm, u8 ch )
//* 功能        : receive byte from uart in sepcial protocol
//* 输入参数    : struct frm_t frm, u8 ch
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void sea_recvbyte ( struct frm_t * frm, u8 ch )
{
    static u8 sum;

//    UartSendByte(CONSOLE, ch);
    if (ch == PREFIX && frm->state != PRT_DATA)
    {
	frm->idx   = 0x00;
        frm->state = PRT_DATA;
    }
    else if (frm->state == PRT_DATA)
    {
        if (frm->idx == 0x00)
        {
            frm->list[frm->in].cmd = ch; 
            frm->idx ++;
            sum = ch;
        }
        else 
        {
            if (frm->idx == 0x01)
            {
                if (ch > MAXFRMLEN)
                {
                    frm->state = PRT_IDLE;
                    return;
                }
                frm->list[frm->in].len = ch;
            }
            else if (frm->idx == 0x02)
                frm->list[frm->in].road = ch << 0x08; 
            else if (frm->idx == 0x03)
                frm->list[frm->in].road |= ch; 
            else if (frm->idx <= frm->list[frm->in].len + FEXTRASIZE)
            {
                if (frm->idx == frm->list[frm->in].len + FEXTRASIZE)
                {
                    if ((sum & 0xff) == ch) 
                    {
                        if (frm->in >= FRAMEBUFLEN - 0x01) 
                            frm->in = 0x00;
                        else
                            frm->in ++;   
                    }
                    frm->state = PRT_SUM;
                }
                else
	            frm->list[frm->in].body[frm->idx - FEXTRASIZE] = ch;
            }
            else
                frm->state = PRT_IDLE;
            sum += ch;
            frm->idx ++;
        }
    }
    else
        frm->state = PRT_IDLE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_initframe ( pframe_info_t ptr,  USART_TypeDef * uart )
//* 功能        : initialize frame information structure body
//* 输入参数    : pframe_info_t ptr,  USART_TypeDef * uart
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_initframe ( pframe_info_t ptr )
{   
    memset((void *)ptr, 0x00, sizeof(frame_info_t));

    ptr->get      = sea_getframe;
    ptr->put      = sea_putframe;
    ptr->recv     = sea_recvbyte;
    ptr->putbyte  = sea_putframebyte;
    ptr->putshort = sea_putframeshort;
    ptr->putlong  = sea_putframelong;
    ptr->sendbyte = sea_uartsendbyte;
    ptr->print    = sea_printframe;
}

////////////////////////////////////////////////////////////////////////////////
